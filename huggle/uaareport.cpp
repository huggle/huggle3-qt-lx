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
#include "configuration.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "syslog.hpp"
#include "localization.hpp"
#include "querypool.hpp"
#include "ui_uaareport.h"
#include "wikiuser.hpp"
#include "wikisite.hpp"
#include "wikipage.hpp"
#include "wikiutil.hpp"

using namespace Huggle;

UAAReport::UAAReport(QWidget *parent) : HW("uaareport", this, parent), ui(new Ui::UAAReport)
{
    this->ui->setupUi(this);
    this->User = nullptr;
    this->ContentsOfUAA = "";
    this->Timer = new QTimer(this);
    connect(this->Timer, SIGNAL(timeout()), this, SLOT(onTick()));
    this->page = nullptr;
    this->TimerCheck = new QTimer(this);
    connect(this->TimerCheck, SIGNAL(timeout()), this, SLOT(onStartOfSearch()));
    this->dr = "";
    this->OptionalReason = "";
    this->ta = "";
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

void UAAReport::setUserForUAA(WikiUser *user)
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
    this->qUAApage = Generic::RetrieveWikiPageContents(this->User->GetSite()->GetProjectConfig()->UAAPath, this->User->GetSite());
    this->qUAApage->Site = this->User->GetSite();
    this->qUAApage->Target = _l("uaa-g1");
    QueryPool::HugglePool->AppendQuery(this->qUAApage);
    this->qUAApage->Process();
    this->Timer->start(200);
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
    this->dr = element.text();
    /// \todo Check if user isn't already reported
    Huggle::Syslog::HuggleLogs->DebugLog("Contents of UAA: " + this->dr);
    /// \todo Insert this to project config so that each project can have their own system here
    QString uaasum = Configuration::GenerateSuffix(QString("Reporting ") + this->User->Username + " to UAA",
                                                                        this->User->GetSite()->GetProjectConfig());
    this->whatToReport();
    this->insertUsername();
    WikiUtil::EditPage(Configuration::HuggleConfiguration->ProjectConfig->UAAP, this->dr, uaasum, true);
    Huggle::Syslog::HuggleLogs->Log(_l("uaa-reporting", this->User->Username));
    this->ui->pushButton->setText(_l("uaa-reported"));

}
void UAAReport::insertUsername()
{
    this->ta = this->User->GetSite()->GetProjectConfig()->UAATemplate;
    this->ta.replace("$1", this->User->Username);
    this->ta.replace("$2", this->UAAReportReason + this->OptionalReason);
    this->ContentsOfUAA = this->ta;
    this->dr = this->dr + "\n" + this->ContentsOfUAA;
}

void UAAReport::whatToReport()
{
    this->OptionalReason = this->ui->lineEdit->text();
    QStringList reasons;
    if (this->ui->checkBox->isChecked())
        reasons.append("disruptive");
    if (this->ui->checkBox_2->isChecked())
        reasons.append("offensive");
    if (this->ui->checkBox_3->isChecked())
        reasons.append("promotional username");
    if (this->ui->checkBox_4->isChecked())
        reasons.append("misleading username");
    this->UAAReportReason = "Username is a policy violation because it is ";
    if (reasons.count() > 1)
    {
        int index = 0;
        while (index < (reasons.count() - 1))
        {
            this->UAAReportReason += reasons.at(index) + ", ";
            index++;
        }
        this->UAAReportReason = this->UAAReportReason.mid(0, this->UAAReportReason.length() - 2);
        this->UAAReportReason += " and " + reasons.at(index);
    } else if (reasons.count() == 1)
    {
        this->UAAReportReason += reasons.at(0) + ".";
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

void UAAReport::on_pushButton_clicked()
{
    if (!this->ui->checkBox->isChecked() && !this->ui->checkBox_2->isChecked() && !this->ui->checkBox_3->isChecked()
            && !this->ui->checkBox_4->isChecked() && this->ui->lineEdit->text().isEmpty())
    {
        QMessageBox *g = new QMessageBox();
        g->setWindowTitle(_l("uaa-nr"));
        // You didn't specify a reason as to why the username is a policy violatio
        g->setText(_l("uaa-no-reason-warn"));
        g->setAttribute(Qt::WA_DeleteOnClose);
        g->exec();
        return;
    }
    this->ui->pushButton->setEnabled(false);
    this->getPageContents();
}

void UAAReport::on_pushButton_2_clicked()
{
    this->hide();
}

void UAAReport::on_pushButton_3_clicked()
{
    this->ui->pushButton_3->setEnabled(false);
    this->qCheckUAAUser = Generic::RetrieveWikiPageContents(this->User->GetSite()->GetProjectConfig()->UAAPath, this->User->GetSite());
    this->qCheckUAAUser->Site = this->User->GetSite();
    QueryPool::HugglePool->AppendQuery(this->qCheckUAAUser);
    this->qCheckUAAUser->Process();
    this->TimerCheck->start(HUGGLE_TIMER);
}

bool UAAReport::checkIfReported()
{
    return (!this->dr.contains(this->User->Username));
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
    this->dr = h.text();
    if (!this->checkIfReported())
    {
        mb.setWindowTitle("User is already reported");
        mb.setText("This user has already been reported to UAA.");
    }else
    {
        mb.setWindowTitle("User is not reported");
        mb.setText("This user is not reported to UAA.");
    }
    mb.exec();
}
