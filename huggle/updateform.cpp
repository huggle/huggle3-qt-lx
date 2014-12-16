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
#include <QTemporaryDir>
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

using namespace Huggle;

UpdateForm::UpdateForm(QWidget *parent) : QDialog(parent), ui(new Ui::UpdateForm)
{
    this->ui->setupUi(this);
    this->qData = NULL;
    this->timer = new QTimer(this);

    this->setWindowTitle(_l("updater-title"));
    this->ui->pushButton->setText(_l("updater-update"));
    this->ui->pushButton_2->setText(_l("updater-close"));
    this->ui->checkBox->setText(_l("updater-disable-notify"));
    this->ui->checkBox_2->setChecked(Configuration::HuggleConfiguration->SystemConfig_NotifyBeta);
}

UpdateForm::~UpdateForm()
{
    while (this->Instructions.count())
    {
        delete this->Instructions.at(0);
        this->Instructions.removeAt(0);
    }
    delete this->ui;
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
#ifdef HUGGLE_WIN
    return true;
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
        this->ui->pushButton->setEnabled(false);
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
        // copy folder that contains binary data to temporary folder
        QDir binary_folder(QCoreApplication::applicationDirPath());
        QStringList values;
        QStringList dirs;
        recurseAddDir(binary_folder, values, "", dirs);
        foreach (QString directory, dirs)
        {
            if (!QDir().mkpath(temp.path() + QDir::separator() + directory))
            {
                this->Fail("Failed to create " + temp.path() + QDir::separator() + directory);
                return;
            }
        }
        foreach(QString file, values)
        {
            if (!QFile::copy(binary_folder.path() + QDir::separator() + file,
                             temp.path() + QDir::separator() + file))
            {
                this->Fail("Unable to copy " + binary_folder.path() + QDir::separator() + file +
                                    " to " + temp.path() + QDir::separator() + file);
                return;
            }
        }
        QString manifest_path = temp.path() + QDir::separator() + "updater.xml";
        QFile manifest(manifest_path);
        if (!manifest.open(QIODevice::ReadWrite))
        {
            this->Fail("Unable to open the manifest file");
            return;
        }
        manifest.write(this->Manifest.toUtf8());
        // now we can launch the copy of huggle with parameters for update
#ifdef HUGGLE_WIN
        QString program = temp.path() + QDir::separator() + "huggle.exe";
        QString arguments = "--update" + manifest_path;
        /*
        QProcess *ClientProcess = new QProcess( this );
        // exit calling application on called application start
        connect( ClientProcess, SIGNAL( started() ), this, SLOT( Exit() ) );
        // receive errors
        connect(ClientProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(ProcessError(QProcess::ProcessError)));

        ClientProcess->startDetached( program, arguments );*/
        SHELLEXECUTEINFO shExInfo = {0};
        shExInfo.cbSize = sizeof(shExInfo);
        shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        shExInfo.hwnd = 0;
        shExInfo.lpVerb = reinterpret_cast<LPCTSTR>("runas");
        shExInfo.lpFile = reinterpret_cast<LPCTSTR>(program.utf16());
        shExInfo.lpParameters = reinterpret_cast<LPCTSTR>(arguments.utf16());
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
    }
    this->close();

}
void Huggle::UpdateForm::on_pushButton_2_clicked()
{
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
        if (!line->attributes().contains("from"))
        {
            Syslog::HuggleLogs->WarningLog("Invalid updater instruction: download is missing from, ignoring the update");
            return false;
        }

        this->Instructions.append(new Instruction(Instruction_Download, line->attribute("from"), line->attribute("to")));
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
    if (this->ui->checkBox->isChecked())
    {
        Configuration::HuggleConfiguration->SystemConfig_UpdatesEnabled = false;
        Configuration::HuggleConfiguration->SaveSystemConfig();
    }
    Configuration::HuggleConfiguration->SystemConfig_NotifyBeta = this->ui->checkBox_2->isChecked();
    QDialog::reject();
}

void UpdateForm::Fail(QString reason)
{
    Generic::MessageBox("Fatal", reason, MessageBoxStyleError);
    this->Error = reason;
    this->close();
}

Instruction::Instruction(InstructionType type, QString from, QString to, bool is_elevated, bool is_recursive, bool merge, bool is_overwriting)
{
    this->Elevated = is_elevated;
    this->Overwrite = is_overwriting;
    this->Destination = to;
    this->Source = from;
    this->Is_Merging = merge;
    this->Is_Recursive = is_recursive;
    this->Type = type;
}
