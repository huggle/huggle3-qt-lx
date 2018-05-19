//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "reportuser.hpp"
#ifdef HUGGLE_WEBEN
    #include "web_engine/huggleweb.hpp"
#else
    #include "webkit/huggleweb.hpp"
#endif
#include "uigeneric.hpp"
#include "blockuserform.hpp"
#include "ui_reportuser.h"
#include <QRegExp>
#include <QMessageBox>
#include <QModelIndex>
#include <QtXml>
#include <huggle_core/apiqueryresult.hpp>
#include <huggle_core/configuration.hpp>
#include <huggle_core/exception.hpp>
#include <huggle_core/generic.hpp>
#include <huggle_core/localization.hpp>
#include <huggle_core/resources.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/wikisite.hpp>
#include <huggle_core/wikiuser.hpp>
#include <huggle_core/wikiutil.hpp>

using namespace Huggle;

void ReportUser::SilentReport(WikiUser *user)
{
    // We need to automatically report this user, that means we create a background instance
    // of this report form, prefill it and submit without even showing it to user
    if (Configuration::HuggleConfiguration->DeveloperMode)
    {
        Generic::DeveloperError();
        return;
    }
    if (user == nullptr)
        throw new Huggle::NullPointerException("local WikiUser *user", BOOST_CURRENT_FUNCTION);
    if (user->IsReported)
    {
        Syslog::HuggleLogs->ErrorLog(_l("report-duplicate"));
        return;
    }
    // only use this if current projects support it
    if (!user->GetSite()->GetProjectConfig()->AIV)
        return;
    ReportUser *window = new ReportUser();
    window->SetUser(user);
    window->SilentReport();
}

ReportUser::ReportUser(QWidget *parent, bool browser) : HW("reportuser", this, parent), ui(new Ui::ReportUser)
{
    this->isBrowser = browser;
    this->ui->setupUi(this);
    this->reportedUser = nullptr;
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->pushButton->setEnabled(false);
    this->ui->pushButton->setText(_l("report-history"));
    QStringList header;
    this->tPageDiff = new QTimer(this);
    connect(this->tPageDiff, SIGNAL(timeout()), this, SLOT(OnPageDiffTimer()));
    header << _l("page") <<
              _l("time") <<
              _l("link") <<
              _l("diffid");
    if (!this->isBrowser)
    {
        // In case we are in browser mode we don't want to use this column
        header << _l("report-include");
    }
    this->ui->tableWidget->setColumnCount(header.size());
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->isSendingMessageNow = false;
    this->reportTs = "null";
    this->tReportPageCheck = new QTimer(this);
    connect(this->tReportPageCheck, SIGNAL(timeout()), this, SLOT(OnReportCheckTimer()));
    this->blockUser = nullptr;
    this->reportText = "";
    this->loading = false;
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
    this->RestoreWindow();
    if (this->isBrowser)
    {
        this->ui->label_2->setVisible(false);
        this->ui->label_3->setVisible(false);
        this->ui->pushButton->setVisible(false);
        this->ui->pushButton_2->setVisible(false);
        this->ui->pushButton_4->setVisible(false);
        this->ui->pushButton_5->setVisible(false);
        this->ui->pushButton_6->setVisible(false);
        this->ui->lineEdit->setVisible(false);
    }
    this->webView = new HuggleWeb(this);
    this->layout()->addWidget(this->webView);
    this->webView->RenderHtml(_l("report-select"));
}

ReportUser::~ReportUser()
{
    delete this->tPageDiff;
    delete this->tReportPageCheck;
    delete this->tReportUser;
    delete this->reportedUser;
    delete this->blockUser;
    delete this->ui;
}

bool ReportUser::SetUser(WikiUser *user)
{
    if (!user)
        throw new Huggle::NullPointerException("WikiUser *user", BOOST_CURRENT_FUNCTION);

    if (this->reportedUser)
        delete this->reportedUser;
    this->reportedUser = new WikiUser(user);
    if (this->isBrowser)
    {
        this->ui->label->setText(_l("contribution-browser-user-info", user->Username));
        this->setWindowTitle("Contribution browser for: " + user->Username);
    } else
    {
        this->setWindowTitle(_l("report-title", this->reportedUser->Username));
        this->ui->label->setText(_l("report-intro", user->Username));
    }
    this->ui->lineEdit->setText(this->reportedUser->GetSite()->GetProjectConfig()->ReportDefaultReason);
    this->qHistory = new ApiQuery(ActionQuery, this->reportedUser->GetSite());
    this->qHistory->Parameters = "list=recentchanges&rcuser=" + QUrl::toPercentEncoding(user->Username) +
            "&rcprop=user%7Ccomment%7Ctimestamp%7Ctitle%7Cids%7Csizes&rclimit=20&rctype=edit%7Cnew";
    this->qHistory->Process();
    if (!this->reportedUser->GetSite()->ProjectConfig->Rights.contains("block"))
    {
        this->ui->pushButton_4->setEnabled(false);
    }
    this->qBlockHistory = new ApiQuery(ActionQuery, this->reportedUser->GetSite());
    this->qBlockHistory->Parameters = "list=logevents&leprop=ids%7Ctitle%7Ctype%7Cuser%7Ctimestamp%7Ccomment%7Cdetails%7Ctags&letype=block&"\
                                      "ledir=newer&letitle=User:" + QUrl::toPercentEncoding(this->reportedUser->Username);
    this->qBlockHistory->Process();
    this->tReportUser = new QTimer(this);
    connect(this->tReportUser, SIGNAL(timeout()), this, SLOT(OnReportUserTimer()));
    this->tReportUser->start(HUGGLE_TIMER);
    return true;
}

void ReportUser::SilentReport()
{
    this->flagSilent = true;
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->ui->lineEdit->setText(this->reportedUser->GetSite()->GetProjectConfig()->ReportAutoSummary);
    this->reportUser();
}

void ReportUser::OnReportUserTimer()
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
                QDomNodeList qlparams = node.childNodes();
                int flag_n = 0;
                while (flag_n < qlparams.count())
                {
                    QDomElement fe = qlparams.at(flag_n).toElement();
                    ++flag_n;
                    if (fe.nodeName() == "params")
                    {
                        if (fe.attributes().contains("duration"))
                            duration = fe.attribute("duration");

                        if (fe.attributes().contains("expiry"))
                            expiration = fe.attribute("expiry");

                        // Look for the flags
                        QDomNodeList params_l = node.childNodes();
                        int param_n = 0;
                        while (param_n++ < params_l.count())
                        {
                            QDomElement parameter = params_l.at(param_n).toElement();
                            if (parameter.nodeName() == "flags")
                            {
                                // Flags
                                QDomNodeList flags_l = parameter.childNodes();
                                int f = 0;
                                while (f++ < flags_l.count())
                                {
                                    QDomElement fx = flags_l.at(f).toElement();
                                    if (fx.nodeName() != "f")
                                        continue;
                                    flags += fx.text() + ", ";
                                }
                            }
                        }
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
                if (this->flagSilent)
                {
                    Syslog::HuggleLogs->ErrorLog(_l("report-fail", this->qEdit->GetFailureReason()));
                    Syslog::HuggleLogs->DebugLog("REPORT: " + this->qEdit->Result->Data);
                    this->kill();
                    //this->close();
                    delete this;
                } else
                {
                    UiGeneric::pMessageBox(this, "Failure", _l("report-fail", this->qEdit->GetFailureReason()), MessageBoxStyleError);
                    Syslog::HuggleLogs->DebugLog("REPORT: " + this->qEdit->Result->Data);
                    this->kill();
                    return;
                }
            }
            this->reportedUser->IsReported = true;
            this->ui->pushButton->setText(_l("report-done"));
            WikiUser::UpdateUser(this->reportedUser);
            this->kill();
            if (this->flagSilent)
            {
                Syslog::HuggleLogs->Log(_l("report-auto", this->reportedUser->Username));
                //this->close();
                delete this;
            }
        }
        return;
    }

    if (this->qHistory == nullptr)
        return;

    if (this->loading)
    {
        if (this->qHistory->IsProcessed())
        {
            // we are now checking the report page for existing report
            QDomDocument d;
            d.setContent(this->qHistory->Result->Data);
            QDomNodeList results = d.elementsByTagName("rev");
            if (results.count() == 0)
            {
                this->ui->pushButton->setText(_l("report-fail2", this->reportedUser->GetSite()->GetProjectConfig()->ReportAIV));
                this->qHistory = nullptr;
                return;
            }
            QDomElement e = results.at(0).toElement();
            if (!e.attributes().contains("timestamp"))
            {
                UiGeneric::MessageBox(_l("error"), _l("report-page-fail-time",this->qReport->Result->Data), Huggle::MessageBoxStyleError);
                this->kill();
                return;
            } else
            {
                this->reportTs = e.attribute("timestamp");
            }
            this->reportContent = e.text();
            if (this->checkUserIsReported())
            {
                this->ui->pushButton->setText(_l("report-duplicate"));
                this->reportedUser->IsReported = true;
                WikiUser::UpdateUser(this->reportedUser);
                this->kill();
                if (this->flagSilent)
                {
                    Syslog::HuggleLogs->ErrorLog(_l("report-duplicate"));
                    this->close();
                }
                return;
            }
            this->insertUser();
            // everything is ok we report user
            QString summary = this->reportedUser->GetSite()->GetProjectConfig()->ReportSummary;
            summary = summary.replace("$1", this->reportedUser->Username);
            this->qEdit = WikiUtil::EditPage(this->reportedUser->GetSite()->GetProjectConfig()->AIVP, this->reportContent, summary,
                                             false, this->reportTs);
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
            this->checkBoxes.clear();
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
                QString link = Configuration::GetProjectScriptURL(this->reportedUser->GetSite()) + "index.php?title=" + page + "&diff=" + diff;
                this->ui->tableWidget->insertRow(0);
                this->ui->tableWidget->setItem(0, 0, new QTableWidgetItem(page));
                this->ui->tableWidget->setItem(0, 1, new QTableWidgetItem(time));
                this->ui->tableWidget->setItem(0, 2, new QTableWidgetItem(link));
                this->ui->tableWidget->setItem(0, 3, new QTableWidgetItem(diff));
                QCheckBox *Item = new QCheckBox(this);
                this->checkBoxes.insert(0, Item);
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

void ReportUser::OnPageDiffTimer()
{
    if (this->qDiff == nullptr || !this->qDiff->IsProcessed())
        return;

    if (this->qDiff->IsFailed())
    {
        this->webView->RenderHtml(_l("browser-fail", this->qDiff->GetFailureReason()));
        this->tPageDiff->stop();
        return;
    }
    QString Summary;
    QString Diff;

    ApiQueryResultNode *diff = this->qDiff->GetApiQueryResult()->GetNode("compare");

    if (diff)
    {
        Diff = diff->Value;
    } else
    {
        Huggle::Syslog::HuggleLogs->DebugLog(this->qDiff->Result->Data);
        this->webView->RenderHtml("Unable to retrieve diff because api returned no data for it, debug information:<br><hr>" +
                                Generic::HtmlEncode(this->qDiff->Result->Data));
        this->tPageDiff->stop();
        return;
    }
    Summary = diff->GetAttribute("tocomment", "<font color=red>Unable to retrieve the edit summary</font>");

    if (!Summary.size())
        Summary = "<font color=red>" + _l("browser-miss-summ") + "</font>";
    else
        Summary = Generic::HtmlEncode(Summary);

    this->webView->RenderHtml(Resources::GetHtmlHeader() + Resources::DiffHeader + "<tr><td colspan=2><b>" + _l("summary")
                               + ":</b> " + Summary + "</td></tr>" + Diff + Resources::DiffFooter + Resources::HtmlFooter);
    this->tPageDiff->stop();
}

void ReportUser::OnReportCheckTimer()
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
        QString result;
        QDomNodeList l = d.elementsByTagName("block");
        if (l.count() > 0)
        {
            result = _l("block-alreadyblocked");
            this->reportedUser->IsBlocked = true;
            this->reportedUser->Update();
        } else
        {
            result = _l("block-not");
        }
        UiGeneric::pMessageBox(this, _l("result"), result);
        this->qCheckIfBlocked.Delete();
        this->ui->pushButton_7->setEnabled(true);
    }
    // If qReport is no null it means we are now retrieving the report page and we need to test
    // report status of user
    if (this->qReport != nullptr && this->qReport->IsProcessed())
    {
        QDomDocument d;
        d.setContent(this->qReport->Result->Data);
        QDomNodeList results = d.elementsByTagName("rev");
        this->ui->pushButton_3->setEnabled(true);
        if (results.count() == 0)
        {
            this->errorMessage(_l("report-page-fail", this->reportedUser->GetSite()->GetProjectConfig()->ReportAIV));
            return;
        }
        this->ui->pushButton_3->setEnabled(true);
        QDomElement e = results.at(0).toElement();
        if (e.attributes().contains("timestamp"))
        {
            this->reportTs = e.attribute("timestamp");
        } else
        {
            this->errorMessage(_l("report-page-fail-time", this->qReport->Result->Data));
            return;
        }
        // Save the contents of report page for later
        this->reportContent = e.text();

        // Check if user is reported or not
        if (this->checkUserIsReported())
        {
            this->errorMessage(_l("report-duplicate"));
            this->reportedUser->IsReported = true;
            WikiUser::UpdateUser(this->reportedUser);
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
    this->reportUser();
}

void ReportUser::on_pushButton_2_clicked()
{
    this->webView->DisplayPage(Configuration::GetProjectWikiURL(this->reportedUser->GetSite()) + QUrl::toPercentEncoding
                              (this->reportedUser->GetTalk()) + "?action=render");
}

void ReportUser::on_tableWidget_clicked(const QModelIndex &index)
{
    this->webView->RenderHtml(_l("wait"));
    this->tPageDiff->stop();
    if (this->qDiff != nullptr)
        this->qDiff->Kill();

    this->qDiff = WikiUtil::APIRequest(ActionCompare, this->reportedUser->GetSite(), "fromrev=" + this->ui->tableWidget->item(index.row(), 3)->text() + "&torelative=prev"\
                                       "&prop=" + QUrl::toPercentEncoding("diff|comment|parsedcomment"));
    this->tPageDiff->start(HUGGLE_TIMER);
}

void Huggle::ReportUser::on_pushButton_5_clicked()
{
    int xx = 0;
    while (xx < this->ui->tableWidget->rowCount())
    {
        if (this->checkBoxes.count() > xx)
            this->checkBoxes.at(xx)->setChecked(true);

        ++xx;
    }
}

void Huggle::ReportUser::on_pushButton_6_clicked()
{
    int xx = 0;
    while (xx < this->ui->tableWidget->rowCount())
    {
        if (this->checkBoxes.count() > xx)
            this->checkBoxes.at(xx)->setChecked(false);

        ++xx;
    }
}

bool ReportUser::checkUserIsReported()
{
    QString regex = this->reportedUser->GetSite()->GetProjectConfig()->ReportUserCheckPattern;
    regex.replace("$username", QRegExp::escape(this->reportedUser->Username));
    QRegExp pattern(regex);
    return pattern.exactMatch(this->reportContent);
}

void ReportUser::insertUser()
{
    QString text = this->reportedUser->GetSite()->GetProjectConfig()->IPVTemplateReport;
    if (!this->reportedUser->IsIP())
    {
        text = this->reportedUser->GetSite()->GetProjectConfig()->RUTemplateReport;
    }
    text = text.replace("$1", this->reportedUser->UnderscorelessUsername());
    text = text.replace("$2", reportText);
    text = text.replace("$3", ui->lineEdit->text());
    this->reportContent = reportContent + "\n" + text;
}

void ReportUser::reportUser()
{
    this->ui->pushButton->setEnabled(false);
    // we need to get a report info for all selected diffs
    QString reports = "";
    int xx = 0;
    int EvidenceID = 0;
    while (xx < this->ui->tableWidget->rowCount())
    {
        if (this->checkBoxes.count() > xx)
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
    if (!this->flagSilent && reports.isEmpty())
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
    this->loading = true;
    this->ui->pushButton->setText(_l("report-retrieving"));
    this->qHistory = WikiUtil::RetrieveWikiPageContents(this->reportedUser->GetSite()->GetProjectConfig()->ReportAIV, this->reportedUser->GetSite());
    this->qHistory->Site = this->reportedUser->GetSite();
    this->qHistory->Process();
    this->reportText = reports;
    this->tReportUser->start(HUGGLE_TIMER);
}

void ReportUser::kill()
{
    this->qHistory.Delete();
    this->qEdit.Delete();
    this->tReportUser->stop();
}

void ReportUser::errorMessage(QString reason)
{
    QMessageBox mb;
    mb.setWindowTitle(_l("report-unable"));
    mb.setText(reason);
    mb.exec();
    this->tReportUser->stop();
    this->qReport = nullptr;
}

void ReportUser::on_pushButton_3_clicked()
{
    this->ui->pushButton_3->setEnabled(false);
    this->qReport = WikiUtil::RetrieveWikiPageContents(this->reportedUser->GetSite()->GetProjectConfig()->ReportAIV,
                                                       this->reportedUser->GetSite());
    this->qReport->Process();
    this->tReportPageCheck->start(HUGGLE_TIMER);
}

void Huggle::ReportUser::on_pushButton_4_clicked()
{
    if (this->blockUser != nullptr)
    {
        delete this->blockUser;
    }
    this->blockUser = new BlockUserForm(this);
    this->blockUser->SetWikiUser(this->reportedUser);
    this->blockUser->show();
}

void Huggle::ReportUser::on_pushButton_7_clicked()
{
    this->ui->pushButton_7->setEnabled(false);
    this->qCheckIfBlocked = new ApiQuery(ActionQuery, this->reportedUser->GetSite());
    this->qCheckIfBlocked->Target = "user";
    this->qCheckIfBlocked->Parameters = "list=blocks&";
    if (!this->reportedUser->IsIP())
    {
        this->qCheckIfBlocked->Parameters += "bkusers=" + QUrl::toPercentEncoding(this->reportedUser->Username);
    } else
    {
        this->qCheckIfBlocked->Parameters += "bkip=" + QUrl::toPercentEncoding(this->reportedUser->Username);
    }
    this->qCheckIfBlocked->Process();
    this->tReportPageCheck->start(HUGGLE_TIMER);
}
