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
#include <QNetworkReply>
#include "configuration.hpp"
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
}

UpdateForm::~UpdateForm()
{
    delete this->ui;
    delete this->timer;
    delete this->manualDownloadpage;
}

void UpdateForm::Check()
{
    this->qData = new WebserverQuery();
    QString version = Configuration::HuggleConfiguration->HuggleVersion;
    if (version.contains(" "))
    {
        // we don't need to send the irrelevant stuff
        version = version.mid(0, version.indexOf(" "));
    }
    this->qData->URL = "http://tools.wmflabs.org/huggle/updater/?version=" + QUrl::toPercentEncoding(version)
            + "&os=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->Platform);
    if (Configuration::HuggleConfiguration->SystemConfig_NotifyBeta)
    {
       this->qData->URL += "&notifybeta";
    }
    Syslog::HuggleLogs->DebugLog("checking for update at "+this->qData->URL);
    this->qData->Process();
    connect(this->timer, SIGNAL(timeout()), this, SLOT(OnTick()));
    this->qData->IncRef();
    this->timer->start(60);
}

void Huggle::UpdateForm::on_pushButton_clicked()
{
    if (this->manualDownloadpage != nullptr)
    {
        QDesktopServices::openUrl(*(this->manualDownloadpage));
    }
    this->close();

}
void Huggle::UpdateForm::on_pushButton_2_clicked()
{
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
    {
        return;
    }

    QDomDocument r;
    r.setContent(this->qData->Result->Data);
    QDomNodeList l = r.elementsByTagName("obsolete");
    if (l.count() == 0)
    {
        // there is no new version of huggle
        this->qData->DecRef();
        this->timer->stop();
        return;
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
            info = info.replace("$LATESTHUGGLE", version);
            this->ui->label->setText(info);
            l = r.elementsByTagName("manualDownloadpage");
            if(l.count() > 0)
            {
                this->manualDownloadpage = new QUrl(l.at(0).toElement().text());
                this->ui->pushButton->setText(_l("updater-open-manualdownloadpage"));
                this->ui->pushButton->setEnabled(true);
            }
            this->show();
            this->qData->DecRef();
            this->timer->stop();
            return;
        } else
        {
            // get the instructions
            l = r.childNodes();
            int id = 0;
            while (id < l.count())
            {
                QDomElement element = l.at(0).toElement();
                id++;
                if (element.tagName() == "download")
                {
                    if (!element.attributes().contains("target"))
                    {
                        Syslog::HuggleLogs->Log("WARNING: Invalid updater instruction: download is missing target, ignoring the update");
                        this->qData->DecRef();
                        this->timer->stop();
                        return;
                    }
                    this->Instructions.append("download " + element.text() + " " + element.attribute("target"));
                }
                if (element.tagName() == "exec")
                {
                    bool root = false;
                    if (element.attributes().contains("root"))
                    {
                        if (element.attribute("root") == "true")
                        {
                            root = true;
                        }
                    }
                    if (root)
                    {
                        this->Instructions.append("roexec " + element.text());
                    } else
                    {
                        this->Instructions.append("exec " + element.text());
                    }
                }
            }
        }
        this->ui->label->setText(info);
        this->show();
    }

    this->qData->UnregisterConsumer("updater");
    this->timer->stop();
}

void Huggle::UpdateForm::on_label_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}
