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
#include "wikiutil.hpp"
#include "configuration.hpp"
#include "generic.hpp"
#include "syslog.hpp"
#include "localization.hpp"
#include "querypool.hpp"
#include "ui_uaareport.h"

using namespace Huggle;

UAAReport::UAAReport(QWidget *parent) : QDialog(parent), ui(new Ui::UAAReport)
{
    this->ui->setupUi(this);
    this->User = nullptr;
    this->ContentsOfUAA = "";
    this->Timer = new QTimer(this);
    connect(this->Timer, SIGNAL(timeout()), this, SLOT(onTick()));
    this->qUAApage = nullptr;
    this->page = nullptr;
    this->qCheckUAAUser = nullptr;
    this->TimerCheck = new QTimer(this);
    connect(this->TimerCheck, SIGNAL(timeout()), this, SLOT(onStartOfSearch()));
    this->dr = "";
    this->OptionalReason = "";
    this->ta = "";
    this->UAAReportReason = "";
}

UAAReport::~UAAReport()
{
    delete this->TimerCheck;
    delete this->User;
    delete this->ui;
    delete this->Timer;
    delete this->page;
    GC_DECREF(this->qCheckUAAUser);
    GC_DECREF(this->qUAApage);
}

void UAAReport::setUserForUAA(WikiUser *user)
{
    // we copy the user so that original can be deleted
    this->User = new WikiUser(user);
}

void UAAReport::getPageContents()
{
    if (this->qUAApage != nullptr)
        this->qUAApage->DecRef();
    this->qUAApage = Generic::RetrieveWikiPageContents(Configuration::HuggleConfiguration->ProjectConfig->UAAPath);
    this->qUAApage->Target = _l("uaa-g1");
    this->qUAApage->IncRef();
    QueryPool::HugglePool->AppendQuery(this->qUAApage);
    this->qUAApage->Process();
    this->Timer->start(200);
}

void UAAReport::onTick()
{
    if (this->qUAApage == nullptr || !this->qUAApage->IsProcessed())
        return;
    QDomDocument r;
    r.setContent(this->qUAApage->Result->Data);
    this->qUAApage->DecRef();
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
    QString uaasum = "Reporting " + this->User->Username + " to UAA " + Configuration::HuggleConfiguration->ProjectConfig->EditSuffixOfHuggle;
    this->whatToReport();
    this->insertUsername();
    WikiUtil::EditPage(Configuration::HuggleConfiguration->UAAP, dr, uaasum, true)->DecRef();
    Huggle::Syslog::HuggleLogs->Log(_l("uaa-reporting", this->User->Username));
    this->ui->pushButton->setText(_l("uaa-reported"));

}
void UAAReport::insertUsername()
{
    this->ta = Configuration::HuggleConfiguration->ProjectConfig->UAATemplate;
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
    this->qCheckUAAUser = new ApiQuery(ActionQuery);
    this->qCheckUAAUser = Generic::RetrieveWikiPageContents(Configuration::HuggleConfiguration->ProjectConfig->UAAPath);
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
    if (this->qCheckUAAUser == nullptr || !this->qCheckUAAUser->IsProcessed())
        return;
    QDomDocument tj;
    tj.setContent(this->qCheckUAAUser->Result->Data);
    QDomNodeList chkusr = tj.elementsByTagName("rev");
    this->TimerCheck->stop();
    this->qCheckUAAUser->DecRef();
    this->qCheckUAAUser = nullptr;
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
