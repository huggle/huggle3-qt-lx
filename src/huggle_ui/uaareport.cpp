//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "uaareport.hpp"
#include <QtXml>
#include <QMessageBox>
#include <QUrl>
#include <huggle_core/configuration.hpp>
#include <huggle_core/exception.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/localization.hpp>
#include <huggle_core/querypool.hpp>
#include <huggle_core/wikiuser.hpp>
#include <huggle_core/wikisite.hpp>
#include <huggle_core/wikipage.hpp>
#include <huggle_core/wikiutil.hpp>
#include "ui_uaareport.h"

using namespace Huggle;

UAAReport::UAAReport(QWidget *parent) : HW("uaareport", this, parent), ui(new Ui::UAAReport)
{
    this->ui->setupUi(this);
    this->User = nullptr;
    this->Timer = new QTimer(this);
    connect(this->Timer, &QTimer::timeout, this, &UAAReport::onTick);
    this->page = nullptr;
    this->TimerCheck = new QTimer(this);
    connect(this->TimerCheck, &QTimer::timeout, this, &UAAReport::onStartOfSearch);
    this->reportPageContents = "";
    this->OptionalReason = "";
    this->UAAReportReason = "";
    this->RestoreWindow();
}

UAAReport::~UAAReport()
{
    delete this->TimerCheck;
    delete this->User;
    delete this->ui;
    delete this->Timer;
    delete this->page;
}

void UAAReport::SetUserForUAA(WikiUser *user)
{
    // we copy the user so that original can be deleted
    this->User = new WikiUser(user);
}

void UAAReport::getPageContents()
{
    if (this->User == nullptr)
        throw new Huggle::NullPointerException("local WikiUser User", BOOST_CURRENT_FUNCTION);
    if (this->qUAApage != nullptr)
        this->qUAApage->DecRef();
    this->qUAApage = WikiUtil::RetrieveWikiPageContents(this->User->GetSite()->GetProjectConfig()->UAAPath, this->User->GetSite());
    this->qUAApage->Site = this->User->GetSite();
    this->qUAApage->Target = _l("uaa-g1");
    QueryPool::HugglePool->AppendQuery(this->qUAApage);
    this->qUAApage->Process();
    this->Timer->start(HUGGLE_TIMER);
}

void UAAReport::onTick()
{
    if (this->User == nullptr)
    {
        this->Timer->stop();
        throw new Huggle::NullPointerException("local WikiUser User", BOOST_CURRENT_FUNCTION);
    }
    if (this->qUAApage == nullptr || !this->qUAApage->IsProcessed())
        return;
    QDomDocument r;
    r.setContent(this->qUAApage->Result->Data);
    this->qUAApage = nullptr;
    QDomNodeList l = r.elementsByTagName("rev");
    if (l.count() == 0)
    {
        // the query for the page contents returned no data
        this->failed(_l("uaa-e1"));
        return;
    }
    QDomElement element = l.at(0).toElement();
    if (!element.text().length())
    {
        // the page contents weren't available
        this->failed(_l("uaa-e2"));
        return;
    }
    this->Timer->stop();
    this->reportPageContents = element.text();
    /// \todo Check if user isn't already reported
    Huggle::Syslog::HuggleLogs->DebugLog("Contents of UAA (before): " + this->reportPageContents);
    /// \todo Insert this to project config so that each project can have their own system here
    QString uaasum = Configuration::GenerateSuffix(QString("Reporting ") + this->User->Username + " to UAA",
                                                                        this->User->GetSite()->GetProjectConfig());
    this->whatToReport();
    if (!this->insertUsername())
        return;
    Huggle::Syslog::HuggleLogs->DebugLog("Contents of UAA (after): " + this->reportPageContents);
    WikiUtil::EditPage(Configuration::HuggleConfiguration->ProjectConfig->UAAP, this->reportPageContents, uaasum, true);
    Huggle::Syslog::HuggleLogs->Log(_l("uaa-reporting", this->User->Username));
    this->ui->btnReport->setText(_l("uaa-reported"));
    this->ui->btnCancel->setText(_l("uaa-close"));
}
bool UAAReport::insertUsername()
{
    QString text = this->User->GetSite()->GetProjectConfig()->UAATemplate;
    text.replace("$1", this->User->Username);
    text.replace("$2", this->UAAReportReason + this->OptionalReason);

    if (text.isEmpty())
    {
        this->failed("template processing yielded empty text (is the uaa-template empty?)");
        return false;
    }

    this->reportPageContents = this->reportPageContents + "\n" + text;
    return true;
}

void UAAReport::disableForm()
{
    this->ui->checkBoxDisruptive->setEnabled(false);
    this->ui->checkBoxOffensive->setEnabled(false);
    this->ui->checkBoxPromotional->setEnabled(false);
    this->ui->checkBoxMisleading->setEnabled(false);
    this->ui->lineEditOtherReason->setEnabled(false);
}

void UAAReport::whatToReport()
{
    this->OptionalReason = this->ui->lineEditOtherReason->text();
    QStringList reasons;
    if (this->ui->checkBoxDisruptive->isChecked())
        reasons.append("disruptive");
    if (this->ui->checkBoxOffensive->isChecked())
        reasons.append("offensive");
    if (this->ui->checkBoxPromotional->isChecked())
        reasons.append("promotional");
    if (this->ui->checkBoxMisleading->isChecked())
        reasons.append("misleading");
    this->UAAReportReason = "Violation of the username policy as a ";
    if (reasons.count() > 1)
    {
        int index = 0;
        while (index < (reasons.count() - 1))
        {
            this->UAAReportReason += reasons.at(index) + ", ";
            index++;
        }
        this->UAAReportReason = this->UAAReportReason.mid(0, this->UAAReportReason.length() - 2);
        this->UAAReportReason += " and " + reasons.at(index) + " username.";
    } else if (reasons.count() == 1)
    {
        this->UAAReportReason += reasons.at(0) + " username.";
    }
}

void UAAReport::failed(QString reason)
{
    QMessageBox m_;
    m_.setWindowTitle("Unable to report user to UAA");
    m_.setText("Unable to report the user because " + reason);
    m_.exec();
    this->Timer->stop();
}

void UAAReport::on_btnReport_clicked()
{
    if (!this->ui->checkBoxDisruptive->isChecked() && !this->ui->checkBoxOffensive->isChecked() && !this->ui->checkBoxPromotional->isChecked()
            && !this->ui->checkBoxMisleading->isChecked() && this->ui->lineEditOtherReason->text().isEmpty())
    {
        QMessageBox *g = new QMessageBox();
        g->setWindowTitle(_l("uaa-nr"));
        // You didn't specify a reason as to why the username is a policy violation
        g->setText(_l("uaa-no-reason-warn"));
        g->setAttribute(Qt::WA_DeleteOnClose);
        g->exec();
        return;
    }
    this->ui->btnReport->setEnabled(false);
    this->disableForm();
    this->getPageContents();
}

void UAAReport::on_btnCancel_clicked()
{
    this->hide();
}

void UAAReport::on_btnCheck_clicked()
{
    this->ui->btnCheck->setEnabled(false);
    this->qCheckUAAUser = WikiUtil::RetrieveWikiPageContents(this->User->GetSite()->GetProjectConfig()->UAAPath, this->User->GetSite());
    this->qCheckUAAUser->Site = this->User->GetSite();
    QueryPool::HugglePool->AppendQuery(this->qCheckUAAUser);
    this->qCheckUAAUser->Process();
    this->TimerCheck->start(HUGGLE_TIMER);
}

bool UAAReport::checkIfReported()
{
    return (!this->reportPageContents.contains(this->User->Username));
}

void UAAReport::onStartOfSearch()
{
    if (this->qCheckUAAUser == nullptr || !this->qCheckUAAUser->IsProcessed())
        return;
    QDomDocument tj;
    tj.setContent(this->qCheckUAAUser->Result->Data);
    QDomNodeList chkusr = tj.elementsByTagName("rev");
    this->qCheckUAAUser.Delete();
    this->TimerCheck->stop();
    QMessageBox mb;
    if (chkusr.count() == 0)
    {
        mb.setWindowTitle("Cannot retrieve page");
        mb.setIcon(QMessageBox::Critical);
        mb.setText("Retrieving the page " + Configuration::HuggleConfiguration->ProjectConfig->UAAPath + " failed.");
        mb.exec();
        return;
    }
    QDomElement h = chkusr.at(0).toElement();
    this->reportPageContents = h.text();
    if (!this->checkIfReported())
    {
        mb.setWindowTitle(_l("uaa-user-reported-title"));
        mb.setText(_l("uaa-user-reported"));
    } else
    {
        mb.setWindowTitle(_l("uaa-user-unreported-title"));
        mb.setText(_l("uaa-user-unreported"));
    }
    mb.exec();
}
