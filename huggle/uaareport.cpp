//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "uaareport.hpp"
#include "wikiutil.hpp"
#include "configuration.hpp"
#include "generic.hpp"
#include "ui_uaareport.h"

using namespace Huggle;

UAAReport::UAAReport(QWidget *parent) : QDialog(parent), ui(new Ui::UAAReport)
{
    this->ui->setupUi(this);
    this->User = NULL;
    this->ContentsOfUAA = "";
    this->Timer = new QTimer(this);
    connect(this->Timer, SIGNAL(timeout()), this, SLOT(onTick()));
    this->qUAApage = NULL;
    this->page = NULL;
    this->qCheckUAAUser = NULL;
    this->TimerCheck = new QTimer(this);
    connect(this->TimerCheck, SIGNAL(timeout()), this, SLOT(onStartOfSearch()));
    this->dr = "";
    this->OptionalReason = "";
    this->ta = "";
    this->UAAReportReason = "";
}

UAAReport::~UAAReport()
{
    this->DelRef();
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
    if (this->qUAApage != NULL)
    {
        this->qUAApage->DecRef();
    }
    this->qUAApage = Generic::RetrieveWikiPageContents(Configuration::HuggleConfiguration->ProjectConfig_UAAPath);
    /// \todo LOCALIZE THIS
    this->qUAApage->Target = "Getting content of UAA";
    this->qUAApage->IncRef();
    QueryPool::HugglePool->AppendQuery(this->qUAApage);
    this->qUAApage->Process();
    this->Timer->start(200);
}

void UAAReport::onTick()
{
    if (this->qUAApage == NULL || !this->qUAApage->IsProcessed())
        return;
    QDomDocument r;
    r.setContent(this->qUAApage->Result->Data);
    this->qUAApage->DecRef();
    this->qUAApage = NULL;
    QDomNodeList l = r.elementsByTagName("rev");
    if (l.count() == 0)
    {
        /// \todo LOCALIZE ME
        this->failed("the query for the page contents returned no data.");
        return;
    }
    QDomElement element = l.at(0).toElement();
    if (!element.text().length())
    {
        /// \todo LOCALIZE ME
        this->failed("the page contents weren't available.");
        return;
    }
    this->Timer->stop();
    this->dr = element.text();
    /// \todo Check if user isn't already reported
    Huggle::Syslog::HuggleLogs->DebugLog("Contents of UAA: " + this->dr);
    /// \todo LOCALIZE ME
    QString uaasum = "Reporting " + this->User->Username + " to UAA " + Configuration::HuggleConfiguration->ProjectConfig_EditSuffixOfHuggle;
    this->whatToReport();
    this->insertUsername();
    WikiUtil::EditPage(Configuration::HuggleConfiguration->UAAP, dr, uaasum, true)->DecRef();
    /// \todo LOCALIZE ME
    Huggle::Syslog::HuggleLogs->Log("Reporting" + this->User->Username + " to UAA" );
    this->ui->pushButton->setText("Reported");

}
void UAAReport::insertUsername()
{
    this->ta = Configuration::HuggleConfiguration->ProjectConfig_UAATemplate;
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
        /// \todo LOCALIZE ME
        g->setWindowTitle("No reason specified");
        /// \todo LOCALIZE ME
        g->setText("You didn't specify a reason as to why the username is a policy violation. "\
                   "Please specify a reason.");
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
    this->qCheckUAAUser = new ApiQuery(ActionQuery);
    this->qCheckUAAUser = Generic::RetrieveWikiPageContents(Configuration::HuggleConfiguration->ProjectConfig_UAAPath);
    this->qCheckUAAUser->IncRef();
    QueryPool::HugglePool->AppendQuery(this->qCheckUAAUser);
    this->qCheckUAAUser->Process();
    this->TimerCheck->start(100);
}

bool UAAReport::checkIfReported()
{
    return !this->dr.contains(this->User->Username);
}

void UAAReport::onStartOfSearch()
{
    if (this->qCheckUAAUser == NULL || !this->qCheckUAAUser->IsProcessed())
        return;
    QDomDocument tj;
    tj.setContent(this->qCheckUAAUser->Result->Data);
    QDomNodeList chkusr = tj.elementsByTagName("rev");
    this->TimerCheck->stop();
    this->qCheckUAAUser->DecRef();
    this->qCheckUAAUser = NULL;
    QMessageBox mb;
    if (chkusr.count() == 0)
    {
        mb.setWindowTitle("Cannot retrieve page");
        mb.setIcon(QMessageBox::Critical);
        mb.setText("Retrieving the page " + Configuration::HuggleConfiguration->ProjectConfig_UAAPath + " failed.");
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

void UAAReport::DelRef()
{
    if (this->qCheckUAAUser != NULL)
    {
        this->qCheckUAAUser->DecRef();
        this->qCheckUAAUser = NULL;
    }
    if (this->qUAApage != NULL)
    {
        this->qUAApage->DecRef();
        this->qUAApage = NULL;
    }
}
