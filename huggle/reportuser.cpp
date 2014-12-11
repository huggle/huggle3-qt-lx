//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "reportuser.hpp"
#include <QMessageBox>
#include <QtXml>
#include <QWebView>
#include "configuration.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "huggleweb.hpp"
#include "syslog.hpp"
#include "localization.hpp"
#include "resources.hpp"
#include "blockuser.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"
#include "wikiutil.hpp"
#include "ui_reportuser.h"
using namespace Huggle;

ReportUser::ReportUser(QWidget *parent) : QDialog(parent), ui(new Ui::ReportUser)
{
    this->ui->setupUi(this);
    this->ReportedUser = nullptr;
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->pushButton->setEnabled(false);
    this->ui->pushButton->setText(_l("report-history"));
    QStringList header;
    this->ui->tableWidget->setColumnCount(5);
    this->tPageDiff = new QTimer(this);
    connect(this->tPageDiff, SIGNAL(timeout()), this, SLOT(On_DiffTick()));
    header << _l("page") <<
              _l("time") <<
              _l("link") <<
              _l("diffid") <<
              _l("report-include");
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->Messaging = false;
    this->ReportTs = "null";
    this->tReportPageCheck = new QTimer(this);
    connect(this->tReportPageCheck, SIGNAL(timeout()), this, SLOT(Test()));
    this->BlockForm = nullptr;
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
    header_bl << _l("id") <<
                 _l("time") <<
                 _l("block-type") <<
                 _l("block-admin") <<
                 _l("reason") <<
                 _l("duration") <<
                 _l("expiry-time") <<
                 _l("flags");
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
    this->tReportUser = nullptr;
    this->ui->webView->setHtml(_l("report-select"));
}

ReportUser::~ReportUser()
{
    delete this->tPageDiff;
    delete this->ReportedUser;
    delete this->BlockForm;
    delete this->ui;
}

bool ReportUser::SetUser(WikiUser *user)
{
    if (!user)
        throw new Huggle::NullPointerException("WikiUser *user", BOOST_CURRENT_FUNCTION);

    if (this->ReportedUser)
        delete this->ReportedUser;
    this->ReportedUser = new WikiUser(user);
    this->setWindowTitle(_l("report-title", this->ReportedUser->Username));
    this->ui->label->setText(_l("report-intro", user->Username));
    this->ui->lineEdit->setText(this->ReportedUser->GetSite()->GetProjectConfig()->ReportDefaultReason);
    this->qHistory = new ApiQuery(ActionQuery, this->ReportedUser->GetSite());
    this->qHistory->Parameters = "list=recentchanges&rcuser=" + QUrl::toPercentEncoding(user->Username) +
            "&rcprop=user%7Ccomment%7Ctimestamp%7Ctitle%7Cids%7Csizes&rclimit=20&rctype=edit%7Cnew";
    this->qHistory->Process();
    if (!this->ReportedUser->GetSite()->ProjectConfig->Rights.contains("block"))
    {
        this->ui->pushButton_4->setEnabled(false);
    }
    this->qBlockHistory = new ApiQuery(ActionQuery, this->ReportedUser->GetSite());
    this->qBlockHistory->Parameters = "list=logevents&leprop=ids%7Ctitle%7Ctype%7Cuser%7Ctimestamp%7Ccomment%7Cdetails%7Ctags&letype"\
                                      "=block&ledir=newer&letitle=User:" + QUrl::toPercentEncoding(this->ReportedUser->Username);
    this->qBlockHistory->Process();
    this->tReportUser = new QTimer(this);
    connect(this->tReportUser, SIGNAL(timeout()), this, SLOT(Tick()));
    this->tReportUser->start(HUGGLE_TIMER);
    return true;
}

void ReportUser::Tick()
{
    if (this->qBlockHistory != nullptr)
    {
        if (this->qBlockHistory->IsProcessed())
        {
            QDomDocument BlockHistory;
            BlockHistory.setContent(this->qBlockHistory->Result->Data);
            QDomNodeList results = BlockHistory.elementsByTagName("item");
            int CurrentId = 0;
            while (CurrentId < results.count())
            {
                QDomNode node = results.at(CurrentId);
                QDomElement _e = results.at(CurrentId).toElement();
                ++CurrentId;
                if (!_e.attributes().contains("logid"))
                    continue;
                if (!_e.attributes().contains("type"))
                    continue;
                else if (_e.attribute("type") != "block")
                    continue;
                if (!_e.attributes().contains("action") ||
                    !_e.attributes().contains("user") ||
                    !_e.attributes().contains("timestamp") ||
                    !_e.attributes().contains("comment"))
                {
                    continue;
                }
                QString id =          _e.attribute("logid");
                QString action =      _e.attribute("action");
                QString user =        _e.attribute("user");
                QString timestamp =   _e.attribute("timestamp");
                QString comment =     _e.attribute("comment");
                QString duration =    "indefinite";
                QString expiration =  "will not expire";
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
                    ++flag_n;
                    if (fe.nodeName() != "block")
                        continue;
                    if (fe.attributes().contains("duration"))
                        duration = fe.attribute("duration");

                    if (fe.attributes().contains("expiry"))
                        expiration = fe.attribute("expiry");
                    if (fe.attributes().contains("flags"))
                        flags = fe.attribute("flags");
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
            this->ui->tableWidget_2->resizeRowsToContents();
            this->qBlockHistory = nullptr;
        }
    }

    if (this->qEdit != nullptr)
    {
        // we already reported user and now we need to check if it was written or not
        if (this->qEdit->IsProcessed())
        {
            // it finished, let's check if there was an error or not
            if (this->qEdit->IsFailed())
            {
                this->tReportUser->stop();
                this->ui->pushButton->setText(_l("report-user"));
                this->ui->pushButton->setEnabled(true);
                QMessageBox mb;
                mb.setText("Failed to report user because: " + this->qEdit->Result->ErrorMessage);
                Syslog::HuggleLogs->DebugLog("REPORT: " + this->qEdit->Result->Data);
                mb.exec();
                this->Kill();
                return;
            }
            this->ReportedUser->IsReported = true;
            this->ui->pushButton->setText(_l("report-done"));
            WikiUser::UpdateUser(this->ReportedUser);
            this->Kill();
        }
        return;
    }

    if (this->qHistory == nullptr)
        return;

    if (this->Loading)
    {
        if (this->qHistory->IsProcessed())
        {
            // we are now checking the report page for existing report
            QDomDocument d;
            d.setContent(this->qHistory->Result->Data);
            QDomNodeList results = d.elementsByTagName("rev");
            if (results.count() == 0)
            {
                this->ui->pushButton->setText(_l("report-fail2", this->ReportedUser->GetSite()->GetProjectConfig()->ReportAIV));
                this->qHistory = nullptr;
                return;
            }
            QDomElement e = results.at(0).toElement();
            if (!e.attributes().contains("timestamp"))
            {
                QMessageBox mb;
                mb.setText("Unable to retrieve timestamp of current report page, api failure:\n\n" + this->qReport->Result->Data);
                mb.exec();
                this->Kill();
                return;
            } else
            {
                this->ReportTs = e.attribute("timestamp");
            }
            this->ReportContent = e.text();
            if (!this->CheckUser())
            {
                this->ui->pushButton->setText(_l("report-duplicate"));
                this->ReportedUser->IsReported = true;
                WikiUser::UpdateUser(this->ReportedUser);
                this->Kill();
                return;
            }
            this->InsertUser();
            // everything is ok we report user
            QString summary = this->ReportedUser->GetSite()->GetProjectConfig()->ReportSummary;
            summary = summary.replace("$1",this->ReportedUser->Username);
            this->qEdit = WikiUtil::EditPage(this->ReportedUser->GetSite()->GetProjectConfig()->AIVP, this->ReportContent, summary,
                                             false, this->ReportTs);
            this->ui->pushButton->setText(_l("report-write"));
            this->qHistory.Delete();
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
            this->CheckBoxes.clear();
            while (results.count() > xx)
            {
                QDomElement edit = results.at(xx).toElement();
                if (!edit.attributes().contains("type"))
                    continue;
                QString page = "unknown page";
                if (edit.attributes().contains("title"))
                    page = edit.attribute("title");
                QString time = "unknown time";
                if (edit.attributes().contains("timestamp"))
                    time = edit.attribute("timestamp");
                QString diff = "";
                if (edit.attributes().contains("revid"))
                {
                    diff = edit.attribute("revid");
                }
                QString link = Configuration::GetProjectScriptURL(this->ReportedUser->GetSite()) + "index.php?title=" + page + "&diff=" + diff;
                this->ui->tableWidget->insertRow(0);
                this->ui->tableWidget->setItem(0, 0, new QTableWidgetItem(page));
                this->ui->tableWidget->setItem(0, 1, new QTableWidgetItem(time));
                this->ui->tableWidget->setItem(0, 2, new QTableWidgetItem(link));
                this->ui->tableWidget->setItem(0, 3, new QTableWidgetItem(diff));
                QCheckBox *Item = new QCheckBox(this);
                this->CheckBoxes.insert(0, Item);
                this->ui->tableWidget->setCellWidget(0, 4, Item);
                ++xx;
            }
            this->ui->tableWidget->sortByColumn(1, Qt::DescendingOrder);
        }
        this->ui->tableWidget->resizeRowsToContents();
        this->qHistory.Delete();
        this->ui->pushButton->setEnabled(true);
        this->ui->pushButton->setText(_l("report-tu"));
    }
}

void ReportUser::On_DiffTick()
{
    if (this->qDiff == nullptr || !this->qDiff->IsProcessed())
        return;

    if (this->qDiff->Result->IsFailed())
    {
        ui->webView->setHtml(_l("browser-fail", this->qDiff->Result->ErrorMessage));
        this->tPageDiff->stop();
        return;
    }
    QString Summary;
    QString Diff;
    QDomDocument d;
    d.setContent(this->qDiff->Result->Data);
    QDomNodeList l = d.elementsByTagName("rev");
    QDomNodeList diff = d.elementsByTagName("diff");
    if (diff.count() > 0)
    {
        QDomElement e = diff.at(0).toElement();
        if (e.nodeName() == "diff")
            Diff = e.text();
    } else
    {
        Huggle::Syslog::HuggleLogs->DebugLog(this->qDiff->Result->Data);
        this->ui->webView->setHtml("Unable to retrieve diff because api returned no data for it, debug information:<br><hr>" +
                                HuggleWeb::Encode(this->qDiff->Result->Data));
        this->tPageDiff->stop();
        return;
    }
    // get last id
    if (l.count() > 0)
    {
        QDomElement e = l.at(0).toElement();
        if (e.nodeName() == "rev" && e.attributes().contains("comment"))
                Summary = e.attribute("comment");
    }

    if (!Summary.size())
        Summary = "<font color=red>" + _l("browser-miss-summ") + "</font>";
    else
        Summary = HuggleWeb::Encode(Summary);

    this->ui->webView->setHtml(Resources::GetHtmlHeader() + Resources::DiffHeader + "<tr></td colspan=2><b>" + _l("summary")
                               + ":</b> " + Summary + "</td></tr>" + Diff + Resources::DiffFooter + Resources::HtmlFooter);
    this->tPageDiff->stop();
}

void ReportUser::Test()
{
    if (this->qReport == nullptr && this->qCheckIfBlocked == nullptr)
    {
        this->tReportPageCheck->stop();
        return;
    }

    if (this->qCheckIfBlocked != nullptr && this->qCheckIfBlocked->IsProcessed())
    {
        QDomDocument d;
        d.setContent(this->qCheckIfBlocked->Result->Data);
        QMessageBox mb;
        mb.setWindowTitle(_l("result"));
        QDomNodeList l = d.elementsByTagName("block");
        if (l.count() > 0)
        {
            mb.setText("User is already blocked");
            this->ReportedUser->IsBlocked = true;
            this->ReportedUser->Update();
        } else
        {
            mb.setText(_l("block-not"));
        }
        mb.exec();
        this->qCheckIfBlocked.Delete();
        this->ui->pushButton_7->setEnabled(true);
    }
    // check if user was reported is here
    if (this->qReport != nullptr && this->qReport->IsProcessed())
    {
        QDomDocument d;
        d.setContent(this->qReport->Result->Data);
        QDomNodeList results = d.elementsByTagName("rev");
        this->ui->pushButton_3->setEnabled(true);
        if (results.count() == 0)
        {
            this->failCheck("Error unable to retrieve report page at " + this->ReportedUser->GetSite()->GetProjectConfig()->ReportAIV);
            return;
        }
        this->ui->pushButton_3->setEnabled(true);
        QDomElement e = results.at(0).toElement();
        if (e.attributes().contains("timestamp"))
        {
            this->ReportTs = e.attribute("timestamp");
        } else
        {
            this->failCheck("Unable to retrieve timestamp of current report page, api failure:\n\n" + this->qReport->Result->Data);
            return;
        }
        this->ReportContent = e.text();
        if (!this->CheckUser())
        {
            this->failCheck(_l("report-duplicate"));
            this->ReportedUser->IsReported = true;
            WikiUser::UpdateUser(this->ReportedUser);
            return;
        } else
        {
            QMessageBox mb;
            mb.setText(_l("reportuser-not"));
            mb.exec();
            this->qReport = nullptr;
        }
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
            QCheckBox *checkBox = (QCheckBox*)this->ui->tableWidget->cellWidget(xx, 4);
            if (checkBox->isChecked())
            {
                ++EvidenceID;
                reports += "[[Special:Diff/" + this->ui->tableWidget->item(xx, 3)->text() + "|" +
                           QString::number(EvidenceID) + "]] ";
            }
        }
        ++xx;
    }
    if (reports.isEmpty())
    {
        QMessageBox::StandardButton mb;
        mb = QMessageBox::question(this, "Question", _l("report-evidence-none-provid"), QMessageBox::Yes|QMessageBox::No);
        if (mb == QMessageBox::No)
        {
            this->ui->pushButton->setEnabled(true);
            return;
        }
    }
    // obtain current page
    this->Loading = true;
    this->ui->pushButton->setText(_l("report-retrieving"));
    this->qHistory = Generic::RetrieveWikiPageContents(this->ReportedUser->GetSite()->GetProjectConfig()->ReportAIV, this->ReportedUser->GetSite());
    this->qHistory->Site = this->ReportedUser->GetSite();
    this->qHistory->Process();
    this->ReportText = reports;
    this->tReportUser->start(HUGGLE_TIMER);
    return;
}

void ReportUser::on_pushButton_2_clicked()
{
    QUrl u = QUrl::fromEncoded(QString(Configuration::GetProjectWikiURL(this->ReportedUser->GetSite()) + QUrl::toPercentEncoding
                                 (this->ReportedUser->GetTalk()) + "?action=render").toUtf8());
    this->ui->webView->load(u);
}

void ReportUser::on_tableWidget_clicked(const QModelIndex &index)
{
    this->ui->webView->setHtml(_l("wait"));
    this->tPageDiff->stop();
    if (this->qDiff != nullptr)
        this->qDiff->Kill();

    this->qDiff = new ApiQuery(ActionQuery, this->ReportedUser->GetSite());
    this->qDiff->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding( "ids|user|timestamp|comment" ) +
                      "&rvlimit=1&rvtoken=rollback&rvstartid=" + this->ui->tableWidget->item(index.row(), 3)->text() +
                      "&rvendid=" + this->ui->tableWidget->item(index.row(), 3)->text() + "&rvdiffto=prev&titles=" +
                      QUrl::toPercentEncoding(ui->tableWidget->item(index.row(), 0)->text());
    this->qDiff->Process();
    this->tPageDiff->start(HUGGLE_TIMER);
}

void Huggle::ReportUser::on_pushButton_5_clicked()
{
    int xx = 0;
    while (xx < this->ui->tableWidget->rowCount())
    {
        if (this->CheckBoxes.count() > xx)
            this->CheckBoxes.at(xx)->setChecked(true);

        ++xx;
    }
}

void Huggle::ReportUser::on_pushButton_6_clicked()
{
    int xx = 0;
    while (xx < this->ui->tableWidget->rowCount())
    {
        if (this->CheckBoxes.count() > xx)
            this->CheckBoxes.at(xx)->setChecked(false);

        ++xx;
    }
}

bool ReportUser::CheckUser()
{
    if (this->ReportContent.contains(this->ReportedUser->Username))
    {
        return false;
    }
    return true;
}

void ReportUser::InsertUser()
{
    QString text = this->ReportedUser->GetSite()->GetProjectConfig()->IPVTemplateReport;
    if (!this->ReportedUser->IsIP())
    {
        text = this->ReportedUser->GetSite()->GetProjectConfig()->RUTemplateReport;
    }
    text = text.replace("$1", this->ReportedUser->Username);
    text = text.replace("$2", ReportText);
    text = text.replace("$3", ui->lineEdit->text());
    this->ReportContent = ReportContent + "\n" + text;
}

void ReportUser::Kill()
{
    this->qHistory.Delete();
    this->qEdit.Delete();
    this->tReportUser->stop();
}

void ReportUser::failCheck(QString reason)
{
    QMessageBox mb;
    mb.setWindowTitle("Unable to report user :(");
    mb.setText(reason);
    mb.exec();
    this->tReportUser->stop();
    this->qReport = nullptr;
}

void ReportUser::on_pushButton_3_clicked()
{
    this->ui->pushButton_3->setEnabled(false);
    this->qReport = Generic::RetrieveWikiPageContents(this->ReportedUser->GetSite()->GetProjectConfig()->ReportAIV,
                                                      this->ReportedUser->GetSite());
    this->qReport->Process();
    this->tReportPageCheck->start(HUGGLE_TIMER);
}

void Huggle::ReportUser::on_pushButton_4_clicked()
{
    if (this->BlockForm != nullptr)
    {
        delete this->BlockForm;
    }
    this->BlockForm = new BlockUser(this);
    this->BlockForm->SetWikiUser(this->ReportedUser);
    this->BlockForm->show();
}

void Huggle::ReportUser::on_pushButton_7_clicked()
{
    this->ui->pushButton_7->setEnabled(false);
    this->qCheckIfBlocked = new ApiQuery(ActionQuery, this->ReportedUser->GetSite());
    this->qCheckIfBlocked->Target = "user";
    this->qCheckIfBlocked->Parameters = "list=blocks&";
    if (!this->ReportedUser->IsIP())
    {
        this->qCheckIfBlocked->Parameters += "bkusers=" + QUrl::toPercentEncoding(this->ReportedUser->Username);
    } else
    {
        this->qCheckIfBlocked->Parameters += "bkip=" + QUrl::toPercentEncoding(this->ReportedUser->Username);
    }
    this->qCheckIfBlocked->Process();
    this->tReportPageCheck->start(HUGGLE_TIMER);
}
