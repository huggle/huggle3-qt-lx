//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "reportuser.hpp"
#include "ui_reportuser.h"
using namespace Huggle;

ReportUser::ReportUser(QWidget *parent) : QDialog(parent), ui(new Ui::ReportUser)
{
    this->ui->setupUi(this);
    this->user = NULL;
    this->qHistory = NULL;
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->pushButton->setEnabled(false);
    this->ui->pushButton->setText(Localizations::HuggleLocalizations->Localize("report-history"));
    QStringList header;
    this->ui->tableWidget->setColumnCount(5);
    this->diff = new QTimer(this);
    connect(this->diff, SIGNAL(timeout()), this, SLOT(On_DiffTick()));
    header << Localizations::HuggleLocalizations->Localize("page") <<
              Localizations::HuggleLocalizations->Localize("time") <<
              Localizations::HuggleLocalizations->Localize("link") <<
              Localizations::HuggleLocalizations->Localize("diffid") <<
              Localizations::HuggleLocalizations->Localize("report-include");
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->tq = NULL;
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->Messaging = false;
    this->BlockForm = NULL;
    this->qd = NULL;
    this->ReportText = "";
    this->Loading = false;
#if QT_VERSION >= 0x050000
// Qt5 code
    this->ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    this->ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    this->ui->tableWidget->setShowGrid(false);

    QStringList header_bl;
    this->ui->tableWidget_2->setColumnCount(8);
    header_bl << Localizations::HuggleLocalizations->Localize("id") <<
                 Localizations::HuggleLocalizations->Localize("time") <<
                 Localizations::HuggleLocalizations->Localize("block-type") <<
                 Localizations::HuggleLocalizations->Localize("block-admin") <<
                 Localizations::HuggleLocalizations->Localize("reason") <<
                 Localizations::HuggleLocalizations->Localize("duration") <<
                 Localizations::HuggleLocalizations->Localize("expiry-time") <<
                 Localizations::HuggleLocalizations->Localize("flags");
    this->ui->tableWidget_2->setHorizontalHeaderLabels(header_bl);
    this->ui->tableWidget_2->verticalHeader()->setVisible(false);
    this->ui->tableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
#if QT_VERSION >= 0x050000
// Qt5 code
    this->ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    this->ui->tableWidget_2->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    this->ui->tableWidget_2->setShowGrid(false);
    this->t2 = NULL;
    this->qBlockHistory = NULL;
    this->timer = NULL;
    /// \todo LOCALIZE ME
    this->ui->webView->setHtml("Please select a diff in list in order to open preview");
}

ReportUser::~ReportUser()
{
    if (this->qHistory != NULL)
    {
        this->qHistory->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
        this->qHistory->SafeDelete();
    }
    if (this->qBlockHistory != NULL)
    {
        this->qBlockHistory->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
    }
    if (this->qd != NULL)
    {
        this->qd->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
    }
    delete this->diff;
    if (this->tq != NULL)
    {
        this->tq->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
    }
    delete this->BlockForm;
    delete this->ui;
}

bool ReportUser::SetUser(WikiUser *u)
{
    if (this->qHistory != NULL)
    {
        this->qHistory->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
    }
    this->user = u;
    this->ui->label->setText(u->Username);
    this->qHistory = new ApiQuery();
    this->qHistory->RegisterConsumer(HUGGLECONSUMER_REPORTFORM);
    this->qHistory->Parameters = "list=recentchanges&rcuser=" + QUrl::toPercentEncoding(u->Username) +
            "&rcprop=user%7Ccomment%7Ctimestamp%7Ctitle%7Cids%7Csizes&rclimit=20&rctype=edit%7Cnew";
    this->qHistory->SetAction(ActionQuery);
    this->qHistory->Process();
    if (this->qBlockHistory != NULL)
    {
        this->qBlockHistory->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
    }
    if (!Configuration::HuggleConfiguration->Rights.contains("block"))
    {
        this->ui->pushButton_4->setEnabled(false);
    }
    this->qBlockHistory = new ApiQuery();
    this->qBlockHistory->RegisterConsumer(HUGGLECONSUMER_REPORTFORM);
    this->qBlockHistory->Parameters = "list=logevents&leprop=ids%7Ctitle%7Ctype%7Cuser%7Ctimestamp%7Ccomment%7Cdetails%7Ctags&letype"\
                                      "=block&ledir=newer&letitle=User:" + QUrl::toPercentEncoding(this->user->Username);
    this->qBlockHistory->SetAction(ActionQuery);
    this->qBlockHistory->Process();
    this->timer = new QTimer(this);
    connect(this->timer, SIGNAL(timeout()), this, SLOT(Tick()));
    this->timer->start(200);
    return true;
}

void ReportUser::Tick()
{
    if (this->qBlockHistory != NULL)
    {
        if (this->qBlockHistory->IsProcessed())
        {
            QDomDocument d;
            d.setContent(this->qBlockHistory->Result->Data);
            QDomNodeList results = d.elementsByTagName("item");
            int CurrentId = 0;
            while (CurrentId < results.count())
            {
                QDomNode node = results.at(CurrentId);
                QDomElement _e = results.at(CurrentId).toElement();
                CurrentId++;
                if (!_e.attributes().contains("logid"))
                {
                    continue;
                }
                if (!_e.attributes().contains("type"))
                {
                    continue;
                } else if (_e.attribute("type") != "block")
                {
                    continue;
                }
                if (!_e.attributes().contains("action"))
                {
                    continue;
                }
                if (!_e.attributes().contains("user"))
                {
                    continue;
                }
                if (!_e.attributes().contains("timestamp"))
                {
                    continue;
                }
                if (!_e.attributes().contains("comment"))
                {
                    continue;
                }
                QString id = _e.attribute("logid");
                QString action = _e.attribute("action");
                QString user = _e.attribute("user");
                QString timestamp = _e.attribute("timestamp");
                QString comment = _e.attribute("comment");
                QString duration = "indefinite";
                QString expiration = "will not expire";
                QString flags = "";
                if (action == "unblock")
                {
                    duration = "unblock";
                    expiration = "unblock";
                    flags = "unblock";
                }
                QDomNodeList qlflags = node.childNodes();
                int flag_n = 0;
                while (flag_n < qlflags.count())
                {
                    QDomElement fe = qlflags.at(flag_n).toElement();
                    flag_n++;
                    if (fe.nodeName() != "block")
                    {
                        continue;
                    }
                    if (fe.attributes().contains("duration"))
                    {
                        duration = fe.attribute("duration");
                    }
                    if (fe.attributes().contains("expiry"))
                    {
                        expiration = fe.attribute("expiry");
                    }
                    if (fe.attributes().contains("flags"))
                    {
                        flags = fe.attribute("flags");
                    }
                }
                this->ui->tableWidget_2->insertRow(0);
                this->ui->tableWidget_2->setItem(0, 0, new QTableWidgetItem(id));
                this->ui->tableWidget_2->setItem(0, 1, new QTableWidgetItem(timestamp));
                this->ui->tableWidget_2->setItem(0, 2, new QTableWidgetItem(action));
                this->ui->tableWidget_2->setItem(0, 3, new QTableWidgetItem(user));
                this->ui->tableWidget_2->setItem(0, 4, new QTableWidgetItem(comment));
                this->ui->tableWidget_2->setItem(0, 5, new QTableWidgetItem(duration));
                this->ui->tableWidget_2->setItem(0, 6, new QTableWidgetItem(expiration));
                this->ui->tableWidget_2->setItem(0, 7, new QTableWidgetItem(flags));
            }
            this->qBlockHistory->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
            this->ui->tableWidget_2->resizeRowsToContents();
            this->qBlockHistory = NULL;
        }
    }

    if (this->qHistory == NULL)
    {
        return;
    }

    if (this->Loading)
    {
        if (this->qHistory->IsProcessed())
        {
            QDomDocument d;
            d.setContent(this->qHistory->Result->Data);
            QDomNodeList results = d.elementsByTagName("rev");
            if (results.count() == 0)
            {
                /// \todo LOCALIZE ME
                this->ui->pushButton->setText("Error unable to retrieve report page at " + Configuration::HuggleConfiguration->LocalConfig_ReportPath);
                this->qHistory->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
                this->qHistory = NULL;
                return;
            }
            QDomElement e = results.at(0).toElement();
            this->_p = e.text();
            if (!this->CheckUser())
            {
                /// \todo LOCALIZE ME
                this->ui->pushButton->setText("This user is already reported");
                this->qHistory->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
                this->qHistory = NULL;
                return;
            }
            this->InsertUser();
            Core::HuggleCore->EditPage(Configuration::HuggleConfiguration->AIVP, this->_p, "Reporting " + user->Username);
            this->timer->stop();
            this->user->IsReported = true;
            WikiUser::UpdateUser(this->user);
            /// \todo LOCALIZE ME
            this->ui->pushButton->setText("Reported");
            this->qHistory->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
            this->qHistory = NULL;
            return;
        }
        return;
    }

    if (this->qHistory->IsProcessed())
    {
        Huggle::Syslog::HuggleLogs->DebugLog(this->qHistory->Result->Data, 2);
        QDomDocument d;
        d.setContent(this->qHistory->Result->Data);
        QDomNodeList results = d.elementsByTagName("rc");
        int xx = 0;
        if (results.count() > 0)
        {
            while (results.count() > xx)
            {
                QDomElement edit = results.at(xx).toElement();
                if (!edit.attributes().contains("type"))
                {
                    continue;
                }
                QString page = "unknown page";
                if (edit.attributes().contains("title"))
                {
                    page = edit.attribute("title");
                }
                QString time = "unknown time";
                if (edit.attributes().contains("timestamp"))
                {
                    time = edit.attribute("timestamp");
                }
                QString diff = "";
                if (edit.attributes().contains("revid"))
                {
                    diff = edit.attribute("revid");
                }
                QString link = Core::GetProjectScriptURL() + "index.php?title=" + page + "&diff=" + diff;
                this->ui->tableWidget->insertRow(0);
                this->ui->tableWidget->setItem(0, 0, new QTableWidgetItem(page));
                this->ui->tableWidget->setItem(0, 1, new QTableWidgetItem(time));
                this->ui->tableWidget->setItem(0, 2, new QTableWidgetItem(link));
                this->ui->tableWidget->setItem(0, 3, new QTableWidgetItem(diff));
                QCheckBox *Item = new QCheckBox(this);
                this->CheckBoxes.insert(0, Item);
                this->ui->tableWidget->setCellWidget(0, 4, Item);
                xx++;
            }
        }
        this->ui->tableWidget->resizeRowsToContents();
        this->qHistory->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
        this->qHistory = NULL;
        this->ui->pushButton->setEnabled(true);
        this->ui->pushButton->setText("Report");
    }
}

void ReportUser::On_DiffTick()
{
    if (this->qd == NULL)
    {
        return;
    }
    if (!this->qd->IsProcessed())
    {
        return;
    }
    if (this->qd->Result->Failed)
    {
        /// \todo LOCALIZE ME
        ui->webView->setHtml("Unable to retrieve diff: " + this->qd->Result->ErrorMessage);
        this->diff->stop();
        return;
    }

    QString Summary;
    QString Diff;

    QDomDocument d;
    d.setContent(this->qd->Result->Data);
    QDomNodeList l = d.elementsByTagName("rev");
    QDomNodeList diff = d.elementsByTagName("diff");
    if (diff.count() > 0)
    {
        QDomElement e = diff.at(0).toElement();
        if (e.nodeName() == "diff")
        {
            Diff = e.text();
        }
    } else
    {
        Huggle::Syslog::HuggleLogs->DebugLog(this->qd->Result->Data);
        /// \todo LOCALIZE ME
        this->ui->webView->setHtml("Unable to retrieve diff because api returned no data for it, debug information:<br><hr>" +
                                HuggleWeb::Encode(this->qd->Result->Data));
        this->diff->stop();
        return;
    }
    // get last id
    if (l.count() > 0)
    {
        QDomElement e = l.at(0).toElement();
        if (e.nodeName() == "rev")
        {
            if (e.attributes().contains("comment"))
            {
                Summary = e.attribute("comment");
            }
        }
    }

    if (Summary == "")
    {
        /// \todo LOCALIZE ME
        Summary = "<font color=red>No summary was provided</font>";
    } else
    {
        Summary = HuggleWeb::Encode(Summary);
    }

    this->ui->webView->setHtml(Core::HuggleCore->HtmlHeader + "<tr></td colspan=2><b>Summary:</b> "
                         + Summary + "</td></tr>" + Diff + Core::HuggleCore->HtmlFooter);
    this->diff->stop();
}

void ReportUser::Test()
{
    if (this->tq == NULL)
    {
        this->t2->stop();
        return;
    }

    if (!this->tq->IsProcessed())
    {
        return;
    }

    QDomDocument d;
    d.setContent(this->tq->Result->Data);
    QDomNodeList results = d.elementsByTagName("rev");
    this->ui->pushButton_3->setEnabled(true);
    if (results.count() == 0)
    {
        QMessageBox mb;
        /// \todo LOCALIZE ME
        mb.setText("Error unable to retrieve report page at " + Configuration::HuggleConfiguration->LocalConfig_ReportPath);
        mb.exec();
        this->timer->stop();
        this->tq->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
        this->tq = NULL;
        return;
    }
    QDomElement e = results.at(0).toElement();
    this->_p = e.text();
    if (!this->CheckUser())
    {
        QMessageBox mb;
        /// \todo LOCALIZE ME
        mb.setText("This user is already reported");
        mb.exec();
        this->timer->stop();
        this->tq->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
        this->user->IsReported = true;
        WikiUser::UpdateUser(this->user);
        this->tq = NULL;
        return;
    } else
    {
        QMessageBox mb;
        /// \todo LOCALIZE ME
        mb.setText("This user is not reported now");
        mb.exec();
        this->timer->stop();
        this->tq->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
        this->tq = NULL;
    }
}

void ReportUser::on_pushButton_clicked()
{
    this->ui->pushButton->setEnabled(false);
    // we need to get a report info for all selected diffs
    QString reports = "";
    int xx = 0;
    int EvidenceID = 0;
    while (xx < this->ui->tableWidget->rowCount())
    {
        if (this->CheckBoxes.count() > xx)
        {
            if (this->CheckBoxes.at(xx)->isChecked())
            {
                EvidenceID++;
                reports += "[" + QString(Core::GetProjectScriptURL() + "index.php?title=" +
                                 QUrl::toPercentEncoding(this->ui->tableWidget->item(xx, 0)->text()) + "&diff="
                                 + this->ui->tableWidget->item(xx, 3)->text()).toUtf8() + " #" + QString::number(EvidenceID) + "] ";
            }
        }
        xx++;
    }
    if (reports == "")
    {
        QMessageBox::StandardButton mb;
        /// \todo LOCALIZE ME
        mb = QMessageBox::question(this, "Question", Huggle::Localizations::HuggleLocalizations->Localize("report-evidence-none-provid")
                                   , QMessageBox::Yes|QMessageBox::No);
        if (mb == QMessageBox::No)
        {
            this->ui->pushButton->setEnabled(true);
            return;
        }
    }
    // obtain current page
    this->Loading = true;
    /// \todo LOCALIZE ME
    this->ui->pushButton->setText("Retrieving current report page");
    if (this->qHistory != NULL)
    {
        this->qHistory->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
    }

    this->qHistory = new ApiQuery();
    this->qHistory->RegisterConsumer(HUGGLECONSUMER_REPORTFORM);
    this->qHistory->SetAction(ActionQuery);
    this->qHistory->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") + "&titles=" +
            QUrl::toPercentEncoding(Configuration::HuggleConfiguration->LocalConfig_ReportPath);
    this->qHistory->Process();
    this->ReportText = reports;
    this->timer->start(800);
    return;
}

void ReportUser::on_pushButton_2_clicked()
{
    QUrl u = QUrl::fromEncoded(QString(Core::GetProjectWikiURL() + QUrl::toPercentEncoding
                                   (this->user->GetTalk()) + "?action=render").toUtf8());
    this->ui->webView->load(u);
}

void ReportUser::on_tableWidget_clicked(const QModelIndex &index)
{
    /// \todo LOCALIZE ME
    this->ui->webView->setHtml("Please wait...");
    this->diff->stop();
    if (this->qd != NULL)
    {
        this->qd->Kill();
        this->qd->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
    }
    this->qd = new ApiQuery();
    this->qd->RegisterConsumer(HUGGLECONSUMER_REPORTFORM);
    this->qd->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding( "ids|user|timestamp|comment" ) + "&rvlimit=1&rvtoken=rollback&rvstartid=" +
            this->ui->tableWidget->item(index.row(), 3)->text() + "&rvendid=" + this->ui->tableWidget->item(index.row(), 3)->text() + "&rvdiffto=prev&titles=" +
            QUrl::toPercentEncoding(ui->tableWidget->item(index.row(), 0)->text());
    this->qd->SetAction(ActionQuery);
    this->qd->Process();
    this->diff->start(200);
}

bool ReportUser::CheckUser()
{
    if (this->_p.contains(this->user->Username))
    {
        return false;
    }
    return true;
}

void ReportUser::InsertUser()
{
    QString xx = Configuration::HuggleConfiguration->LocalConfig_IPVTemplateReport;
    if (!this->user->IsIP())
    {
        xx = Configuration::HuggleConfiguration->LocalConfig_RUTemplateReport;
    }
    xx = xx.replace("$1", this->user->Username);
    xx = xx.replace("$2", ReportText);
    xx = xx.replace("$3", ui->lineEdit->text());
    this->_p = _p + "\n" + xx;
}

void ReportUser::on_pushButton_3_clicked()
{
    if (this->tq != NULL)
    {
        this->tq->UnregisterConsumer(HUGGLECONSUMER_REPORTFORM);
    }

    this->tq = new ApiQuery();
    this->tq->SetAction(ActionQuery);
    this->tq->RegisterConsumer(HUGGLECONSUMER_REPORTFORM);
    this->tq->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") + "&titles=" +
            QUrl::toPercentEncoding(Configuration::HuggleConfiguration->LocalConfig_ReportPath);
    this->tq->Process();
    if (this->t2 == NULL)
    {
        this->t2 = new QTimer(this);
    }
    connect(this->t2, SIGNAL(timeout()), this, SLOT(Test()));
    this->t2->start(100);
}

void Huggle::ReportUser::on_pushButton_4_clicked()
{
    if (this->BlockForm != NULL)
    {
        delete this->BlockForm;
    }
    this->BlockForm = new BlockUser(this);
    this->BlockForm->SetWikiUser(this->user);
    this->BlockForm->show();
}
