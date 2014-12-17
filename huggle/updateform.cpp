//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "updateform.hpp"
#include <QDesktopServices>
#include <QtXml>
#if QT_VERSION >= 0x050000
#include <QTemporaryDir>
#endif
#include <QNetworkReply>
#ifdef HUGGLE_WIN
#include <windows.h>
#include <Shellapi.h>
#include <tchar.h>
#include <string>
#endif
#include "configuration.hpp"
#include "core.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "localization.hpp"
#include "syslog.hpp"
#include "ui_updateform.h"
#include "webserverquery.hpp"

#define LOG Syslog::HuggleLogs->Log

using namespace Huggle;

UpdateForm::UpdateForm(QWidget *parent) : QDialog(parent), ui(new Ui::UpdateForm)
{
    this->ui->setupUi(this);
    this->qData = NULL;
    this->timer = new QTimer(this);
    this->ui->checkBox_2->setChecked(Configuration::HuggleConfiguration->SystemConfig_NotifyBeta);
    if (hcfg->UpdaterMode)
    {
        this->ui->checkBox->setEnabled(false);
        this->ui->checkBox_2->setEnabled(false);
        this->ui->label->setText("I am now updating huggle, please wait...");
        this->ui->pushButton->setEnabled(false);
        this->ui->pushButton_2->setEnabled(false);
        this->Update();
    }
    else
    {
        this->setWindowTitle(_l("updater-title"));
        this->ui->pushButton->setText(_l("updater-update"));
        this->ui->pushButton_2->setText(_l("updater-close"));
        this->ui->checkBox->setText(_l("updater-disable-notify"));
    }
}

UpdateForm::~UpdateForm()
{
    while (this->Instructions.count())
    {
        delete this->Instructions.at(0);
        this->Instructions.removeAt(0);
    }
    delete this->ui;
    delete this->manager;
    delete this->file;
    delete this->reply;
    delete this->timer;
    delete this->manualDownloadpage;
}

void UpdateForm::Check()
{
    this->qData = new WebserverQuery();
    this->qData->URL = "http://tools.wmflabs.org/huggle/updater/?version=" + QUrl::toPercentEncoding(HUGGLE_VERSION)
            + "&os=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->Platform)
            + "&language=" + Localizations::HuggleLocalizations->PreferredLanguage + "&test";

    if (Configuration::HuggleConfiguration->SystemConfig_NotifyBeta)
       this->qData->URL += "&notifybeta";

    HUGGLE_DEBUG1("checking for update at " + this->qData->URL);
    this->qData->Process();
    connect(this->timer, SIGNAL(timeout()), this, SLOT(OnTick()));
    this->qData->IncRef();
    this->timer->start(HUGGLE_TIMER);
}

// Checks if OS is supported by updater
static bool IsSupported()
{
#if QT_VERSION >= 0x050000
#ifdef HUGGLE_WIN
    return true;
#else
    return false;
#endif
#else
    return false;
#endif
}

static void recurseAddDir(QDir d, QStringList &list, QString path, QStringList &dirs)
{
    QStringList qsl = d.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    foreach(QString file, qsl)
    {
        QFileInfo finfo(QString("%1/%2").arg(d.path()).arg(file));
        if (finfo.isSymLink())
            return;
        if (finfo.isDir())
        {
            QString dirname = finfo.fileName();
            QDir sd(finfo.filePath());
            dirs << QDir::toNativeSeparators(path + dirname);
            recurseAddDir(sd, list, path + dirname + QDir::separator(), dirs);
        }
        else
        {
            list << QDir::toNativeSeparators(path + finfo.fileName());
        }
    }
}

void Huggle::UpdateForm::on_pushButton_clicked()
{
    Configuration::HuggleConfiguration->SystemConfig_NotifyBeta = this->ui->checkBox_2->isChecked();
    if (this->ui->checkBox->isChecked())
    {
        Configuration::HuggleConfiguration->SystemConfig_UpdatesEnabled = false;
        Configuration::HuggleConfiguration->SaveSystemConfig();
    }
    if (this->manualDownloadpage != nullptr)
    {
        QDesktopServices::openUrl(*(this->manualDownloadpage));
    }
    else
    {
#if QT_VERSION >= 0x050000
        this->ui->pushButton->setEnabled(false);
        this->ui->label->setText("Preparing files...");
        this->ui->pushButton_2->setEnabled(false);
        if (this->Instructions.count() < 1)
        {
            this->Fail("Manifest contains no data");
            return;
        }
        if (!IsSupported())
        {
            // this is unlike error as there should be no manifest provided for unknown OS
            this->Fail("This operating system is not supported by updater");
            return;
        }
        // first we need to get a temporary folder to which we backup manifest and whole huggle binary folder
        QTemporaryDir temp;
        if (!temp.isValid())
        {
            this->Fail("Unable to get a temporary folder");
            return;
        }
        temp.setAutoRemove(false);
        Syslog::HuggleLogs->Log("Backing up huggle to " + temp.path());
        this->TempPath = temp.path();
        // copy folder that contains binary data to temporary folder
        QDir binary_folder(QCoreApplication::applicationDirPath());
        recurseAddDir(binary_folder, values, "", dirs);
        foreach (QString directory, dirs)
        {
            if (!QDir().mkpath(temp.path() + QDir::separator() + directory))
            {
                this->Fail("Failed to create " + temp.path() + QDir::separator() + directory);
                return;
            }
        }
        this->RootPath = binary_folder.path();
        this->MovingFiles = true;
        this->ui->progressBar->setMaximum(values.count());
        this->timer->start(20);
    }
}
void Huggle::UpdateForm::on_pushButton_2_clicked()
{
    if (hcfg->UpdaterMode)
    {
        QApplication::exit();
        return;
    }
    Configuration::HuggleConfiguration->SystemConfig_NotifyBeta = this->ui->checkBox_2->isChecked();
    if (this->ui->checkBox->isChecked())
    {
        Configuration::HuggleConfiguration->SystemConfig_UpdatesEnabled = false;
        Configuration::HuggleConfiguration->SaveSystemConfig();
    }
    this->close();
}

void UpdateForm::OnTick()
{
    if (this->MovingFiles)
    {
        if (this->CurrentFile >= this->values.count())
        {
            this->timer->stop();
            this->PreparationFinish();
            return;
        }
        QString file_ = this->values.at(this->CurrentFile);
        if (!QFile::copy(this->RootPath + QDir::separator() + file_,
                         this->TempPath + QDir::separator() + file_))
        {
            this->Fail("Unable to copy " + this->RootPath + QDir::separator() + file_ +
                                " to " + this->TempPath + QDir::separator() + file_);
        }
        this->CurrentFile++;
        this->ui->progressBar->setValue(this->CurrentFile);
        return;
    }
    if (hcfg->UpdaterMode)
    {
        this->NextInstruction();
        return;
    }
    if (!this->qData->IsProcessed())
        return;
    QDomDocument r;
    this->Manifest = this->qData->Result->Data;
    r.setContent(this->qData->Result->Data);
    QDomNodeList l = r.elementsByTagName("obsolete");
    if (l.count() == 0)
    {
        // there is no new version of huggle
        goto finish;
    } else
    {
        QString version = l.at(0).toElement().text();
        /// \todo LOCALIZE ME
        QString info = "A newer version of Huggle is available: version " + version;
        l = r.elementsByTagName("info");
        if (l.count() > 0)
        {
            // we don't know how to update o.O
            info = l.at(0).toElement().text();
            this->ui->pushButton->setEnabled(false);
            info = info.replace("$LATESTHUGGLE", version).replace("$VERSION", hcfg->HuggleVersion);
            this->ui->label->setText(info);
            l = r.elementsByTagName("manualDownloadpage");
            if(l.count() > 0)
            {
                this->manualDownloadpage = new QUrl(l.at(0).toElement().text());
                this->ui->pushButton->setText(_l("updater-open-manualdownloadpage"));
                this->ui->pushButton->setEnabled(true);
            }
            this->show();
            goto finish;
        } else
        {
            // get the instructions
            l = r.elementsByTagName("update");
            if (l.count())
            {
                l = l.at(0).childNodes();
                int id = 0;
                while (id < l.count())
                {
                    QDomElement element = l.at(id).toElement();
                    if (this->parse_xml(&element))
                        id++;
                    else
                        goto invalid;
                }
            }
            // we are ready for update now
            this->ui->pushButton->setEnabled(true);
            info = "Huggle can be updated from " + hcfg->HuggleVersion + " to " + version + " please click Update button to proceed.";
            this->ui->label->setText(info);
            this->show();
            goto finish;
        }
    }

    invalid:
        this->ui->pushButton->setEnabled(false);
        this->ui->label->setText("FAILED: the update manifest is invalid. I can't update. The error is: " + this->Error + "\n\nPlease send this error to developers");

    finish:
        this->qData->DecRef();
        this->timer->stop();
}

void UpdateForm::Exit()
{
    Core::HuggleCore->Shutdown();
}

void Huggle::UpdateForm::on_label_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}

void UpdateForm::httpReadyRead()
{
    if (this->file)
        this->file->write(this->reply->readAll());
}

void UpdateForm::httpDownloadFinished()
{
    this->file->flush();
    this->file->close();
    // there is a bug in Qt, we need to reopen the file for hash to be correct
    delete this->file;
    this->file = new QFile(this->TempPath + "temp/" +this->inst->Destination);
    if (!file->open(QIODevice::ReadOnly))
    {
        this->Fail("Unable to open " + this->TempPath + "temp/" + this->inst->Destination);
    }
    QByteArray data = file->readAll();
    file->close();
    QString hash = QString(QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex());
    if (hash != this->inst->Hash_MD5)
    {
        LOG("Hash " + hash + " not matches " + this->inst->Hash_MD5);
        this->Fail("Hash for " + this->inst->Source + " doesn't match");
    } else
    {
        LOG("Hash " + hash + " matches " + this->inst->Hash_MD5);
    }
    delete this->file;
    this->file = nullptr;
    this->reply->deleteLater();
    this->reply = nullptr;
    delete this->inst;
    this->inst = nullptr;
}

void UpdateForm::updateDownloadProgress(qint64, qint64)
{

}

bool Huggle::UpdateForm::parse_xml(QDomElement *line)
{
    if (line->tagName() == "download")
    {
        if (!line->attributes().contains("to"))
        {
            Syslog::HuggleLogs->WarningLog("Invalid updater instruction: download is missing to, ignoring the update");
            return false;
        }
        if (!line->attributes().contains("md5"))
        {
            Syslog::HuggleLogs->WarningLog("Invalid updater instruction: download is missing md5, ignoring the update");
            return false;
        }

        this->Instructions.append(new Instruction(Instruction_Download, line->text(), line->attribute("to"),
                                                  false, false, false, false, line->attribute("md5")));
    }
    if (line->tagName() == "move")
    {
        if (!line->attributes().contains("to"))
        {
            Syslog::HuggleLogs->WarningLog("Invalid updater instruction: move is missing to, ignoring the update");
            return false;
        }
        if (!line->attributes().contains("from"))
        {
            Syslog::HuggleLogs->WarningLog("Invalid updater instruction: move is missing from, ignoring the update");
            return false;
        }
        bool elevated_ = false;
        bool overwrite_ = false;
        bool merging_ = false;
        bool recursive_ = false;
        if (line->attributes().contains("elevated") && line->attribute("elevated") == "true")
            elevated_ = true;
        if (line->attributes().contains("overwrite") && line->attribute("overwrite") == "true")
            overwrite_ = true;
        this->Instructions.append(new Instruction(Instruction_Move, line->attribute("from"), line->attribute("to"), elevated_, recursive_, merging_, overwrite_));
    }
    if (line->tagName() == "copydir")
    {
        if (!line->attributes().contains("to"))
        {
            Syslog::HuggleLogs->WarningLog("Invalid updater instruction: copydir is missing to, ignoring the update");
            return false;
        }
        if (!line->attributes().contains("from"))
        {
            Syslog::HuggleLogs->WarningLog("Invalid updater instruction: copydir is missing from, ignoring the update");
            return false;
        }
        bool elevated_ = false;
        bool overwrite_ = false;
        bool merging_ = false;
        bool recursive_ = false;
        if (line->attributes().contains("elevated") && line->attribute("elevated") == "true")
            elevated_ = true;
        if (line->attributes().contains("overwrite") && line->attribute("overwrite") == "true")
            overwrite_ = true;
        this->Instructions.append(new Instruction(Instruction_Copy, line->attribute("from"), line->attribute("to"), elevated_, recursive_, merging_, overwrite_));
    }
    if (line->tagName() == "copy")
    {
        if (!line->attributes().contains("to"))
        {
            Syslog::HuggleLogs->WarningLog("Invalid updater instruction: copy is missing to, ignoring the update");
            return false;
        }
        if (!line->attributes().contains("from"))
        {
            Syslog::HuggleLogs->WarningLog("Invalid updater instruction: copy is missing from, ignoring the update");
            return false;
        }
        bool elevated_ = false;
        bool overwrite_ = false;
        bool merging_ = false;
        bool recursive_ = false;
        if (line->attributes().contains("elevated") && line->attribute("elevated") == "true")
            elevated_ = true;
        if (line->attributes().contains("overwrite") && line->attribute("overwrite") == "true")
            overwrite_ = true;
        this->Instructions.append(new Instruction(Instruction_Copy, line->attribute("from"), line->attribute("to"), elevated_, recursive_, merging_, overwrite_));
    }
    if (line->tagName() == "remove")
    {
        bool elevated_ = false;
        bool overwrite_ = false;
        bool merging_ = false;
        bool recursive_ = false;
        if (line->attributes().contains("elevated") && line->attribute("elevated") == "true")
            elevated_ = true;
        if (line->attributes().contains("overwrite") && line->attribute("overwrite") == "true")
            overwrite_ = true;
        this->Instructions.append(new Instruction(Instruction_Delete, line->text(), "", elevated_, recursive_));
    }
    if (line->tagName() == "exec")
    {
        bool root = false;
        if (line->attributes().contains("elevated") && line->attribute("elevated") == "true")
            root = true;
        this->Instructions.append(new Instruction(Instruction_Execute, line->text(), "", root));
    }
    return true;
}

void UpdateForm::reject()
{
    //if (hcfg->UpdaterMode)
    //    return;
    if (this->ui->checkBox->isChecked())
    {
        Configuration::HuggleConfiguration->SystemConfig_UpdatesEnabled = false;
        Configuration::HuggleConfiguration->SaveSystemConfig();
    }
    Configuration::HuggleConfiguration->SystemConfig_NotifyBeta = this->ui->checkBox_2->isChecked();
    QDialog::reject();
}

void UpdateForm::PreparationFinish()
{
    QString manifest_path = this->TempPath + QDir::separator() + "updater.xml";
    QFile manifest(manifest_path);
    if (!manifest.open(QIODevice::ReadWrite))
    {
        this->Fail("Unable to open the manifest file");
        return;
    }
    manifest.write(this->Manifest.toUtf8());
    manifest.close();
    // now we can launch the copy of huggle with parameters for update
#ifdef HUGGLE_WIN
    QString program = this->TempPath + QDir::separator() + "huggle.exe";
    QString arguments = "--huggleinternal-update -v --syslog " + this->TempPath + QDir::separator() + "update.log";
    /*
    QProcess *ClientProcess = new QProcess( this );
    // exit calling application on called application start
    connect( ClientProcess, SIGNAL( started() ), this, SLOT( Exit() ) );
    // receive errors
    connect(ClientProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(ProcessError(QProcess::ProcessError)));

    ClientProcess->startDetached( program, arguments );*/
    SHELLEXECUTEINFO shExInfo {};
    shExInfo.cbSize = sizeof(shExInfo);
    shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExInfo.hwnd = 0;
#ifdef __GNUC__
#ifdef UNICODE
    shExInfo.lpVerb = QString("runas").toStdWString().c_str();
    shExInfo.lpFile = program.toStdWString().c_str();
    shExInfo.lpParameters = arguments.toStdWString().c_str();
#else
    shExInfo.lpVerb = QString("runas").toStdString().c_str();
    shExInfo.lpFile = program.toStdString().c_str();
    shExInfo.lpParameters = arguments.toStdString().c_str();
#endif
#else
    //! \todo This is broken on Visual Studio 2013
    shExInfo.lpVerb = _T("runas");
    shExInfo.lpFile = _T(program.toLocal8Bit().toStdString().c_str());
    shExInfo.lpParameters = _T(arguments.toStdString().c_str());
#endif
    shExInfo.lpDirectory = 0;
    shExInfo.nShow = SW_SHOW;
    shExInfo.hInstApp = 0;

    if (ShellExecuteEx(&shExInfo))
    {
        Exit();
    } else
    {
        this->Fail("Unable to launch the updater");
    }
#endif
    this->close();
}

void UpdateForm::Fail(QString reason)
{
    Generic::MessageBox("Fatal", reason, MessageBoxStyleError, true);
    this->Error = reason;
    if (this->timer)
        this->timer->stop();
    this->close();
    if (hcfg->UpdaterMode)
        QApplication::exit();
}

bool UpdateForm::ProcessManifest(QString data)
{
    QDomDocument r;
    r.setContent(data);
    QDomNodeList l;
    l = r.elementsByTagName("update");
    if (l.count())
    {
        l = l.at(0).childNodes();
        int id = 0;
        while (id < l.count())
        {
            QDomElement element = l.at(id).toElement();
            if (this->parse_xml(&element))
                id++;
            else
                return false;
        }
        return true;
    }
    return false;
}

void UpdateForm::ProcessDownload()
{
    if (this->file)
    {
        this->file->close();
        delete this->file;
        this->file = nullptr;
    }
    this->Write("Downloading " + this->inst->Source + "...");
    delete this->manager;
    this->manager = new QNetworkAccessManager(this);
    QString download_target = this->TempPath + "temp/";
    if (!QDir().exists(download_target) && !QDir().mkpath(download_target))
    {
        this->Fail("Unable to create a temporary folder");
        return;
    }
    this->file = new QFile(download_target + this->inst->Destination);
    if (this->file->exists())
    {
        this->Fail(download_target + this->inst->Destination + " already exists!");
        return;
    }
    QFileInfo info(download_target + this->inst->Destination);
    if (!QDir().exists(info.absoluteDir().path()) && !QDir().mkpath(info.absoluteDir().path()))
    {
        this->Fail("Unable to create folder " + info.absoluteDir().path());
        return;
    }
    if (!this->file->open(QIODevice::ReadWrite))
    {
        this->Fail("Unable to open " + info.absolutePath() + " for writing");
        return;
    }
    this->url = QUrl(this->inst->Source);
    this->reply = this->manager->get(QNetworkRequest(this->url));
    connect(this->reply, SIGNAL(finished()), this, SLOT(httpDownloadFinished()));
    connect(this->reply, SIGNAL(readyRead()), this, SLOT(httpReadyRead()));
    connect(this->reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateDownloadProgress(qint64,qint64)));
}

void UpdateForm::Write(QString message)
{
    LOG(message);
    this->ui->label->setText(message);
}

void UpdateForm::NextInstruction()
{
    if (this->inst)
        return;
    if (this->Instructions.count() == 0)
    {
        this->ui->pushButton_2->setEnabled(true);
        this->Write("Update was successfuly finished, you can start huggle now");
        this->timer->stop();
        return;
    }
    // we read the instruction and process it
    this->inst = this->Instructions.at(0);
    this->Instructions.removeAt(0);
    switch (this->inst->Type)
    {
        case Instruction_Download:
            ProcessDownload();
            return;
        case Instruction_Folder:
            if (!QDir().mkpath(this->Path(this->inst->Destination)))
            {
                this->Fail("Unable to create folder " + this->Path(this->inst->Destination));
            }
            break;
        case Instruction_Delete:
        {
            QString path = this->Path(this->inst->Destination);
            LOG("Deleting: " + path);
            if (this->inst->Is_Recursive)
            {
                if (!QDir(path).removeRecursively())
                {
                    this->Fail("Unable to delete: " + path);
                    break;
                }
            } else
            {
                if (!QFile(path).remove())
                {
                    this->Fail("Unable to delete: " + path);
                    break;
                }
            }
        }
            break;
    }
    delete this->inst;
    this->inst = nullptr;
}

void UpdateForm::Update()
{
    this->TempPath = QApplication::applicationDirPath() + QDir::separator();
    connect(this->timer, SIGNAL(timeout()), this, SLOT(OnTick()));
    LOG("Reading manifest");
    QFile file(QApplication::applicationDirPath() + "/updater.xml");
    if (!file.open(QIODevice::ReadOnly))
    {
        this->Fail("Unable to open the manifest file @ " + QApplication::applicationDirPath() + "/updater.xml");
        return;
    }
    this->Manifest = QString(file.readAll());
    if (!this->ProcessManifest(this->Manifest))
    {
        this->Fail("Manifest can't be read");
        return;
    }
    file.close();
    LOG("Processing update");
    this->timer->start(HUGGLE_TIMER);
}

QString UpdateForm::Path(QString text)
{
    QString path = text;
    path = path.replace("$root", this->RootPath)
            .replace("$temp", this->TempPath + "/temp")
            .replace("$root_bck", this->TempPath);
    return path;
}

Instruction::Instruction(InstructionType type, QString from, QString to, bool is_elevated, bool is_recursive, bool merge, bool is_overwriting, QString md5)
{
    this->Elevated = is_elevated;
    this->Overwrite = is_overwriting;
    this->Destination = to;
    this->Source = from;
    this->Hash_MD5 = md5;
    this->Is_Merging = merge;
    this->Is_Recursive = is_recursive;
    this->Type = type;
}
