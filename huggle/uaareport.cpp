//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "uaareport.hpp"
#include "ui_uaareport.h"

using namespace Huggle;

UAAReport::UAAReport(QWidget *parent) : QDialog(parent), ui(new Ui::UAAReport)
{
    this->ui->setupUi(this);
    this->User = NULL;
    this->ContentsOfUAA = "";
    this->qUAApage = NULL;
    this->page = NULL;
    this->uT = NULL;
    this->qChUAApage = NULL;
    this->cuT = NULL;
    this->dr = "";
    this->OptionalReason = "";
    this->ta = "";
    this->UAAReportReason = "";
}

UAAReport::~UAAReport()
{
    delete this->ui;
    delete this->User;
    delete this->uT;
    delete this->page;
}

void UAAReport::setUserForUAA(WikiUser *user)
{
    this->User = user;
}

void UAAReport::getPageContents()
{
    this->qUAApage = new ApiQuery();
    this->qUAApage->SetAction(ActionQuery);
    this->qUAApage->Parameters = "prop=revisions&rvprop=content&titles=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->LocalConfig_UAAPath);
    /// \todo LOCALIZE THIS
    this->qUAApage->Target = "Getting content of UAA";
    this->qUAApage->RegisterConsumer("UAAReport::getPageContents()");
    Core::HuggleCore->AppendQuery(qUAApage);
    this->qUAApage->Process();

    if (this->uT != NULL)
    {
        delete this->uT;
    }
    this->uT = new QTimer(this);
    connect(this->uT, SIGNAL(timeout()), this, SLOT(onTick()));
    this->uT->start(200);
}

void UAAReport::onTick()
{
    if (this->qUAApage == NULL)
    {
        return;
    }

    if (!this->qUAApage->Processed())
    {
        return;
    }
    QDomDocument r;
    r.setContent(this->qUAApage->Result->Data);
    QDomNodeList l = r.elementsByTagName("rev");
    if (l.count() == 0)
    {
        /// \todo LOCALIZE ME
        this->failed("the query for the page contents returned no data.");
        this->qUAApage->UnregisterConsumer("UAAReport::getPageContents()");
        return;
    }
    QDomElement element = l.at(0).toElement();

    if (element.text() == "")
    {
        /// \todo LOCALIZE ME
        this->failed("the page contents weren't available.");
        this->qUAApage->UnregisterConsumer("UAAReport::getPageContents()");
        return;
    }

    this->uT->stop();
    this->dr = element.text();
    this->qUAApage->UnregisterConsumer("UAAReport::getPageContents()");
    /// \todo Check if user isn't already reported
    this->qUAApage = NULL;
    Huggle::Syslog::HuggleLogs->DebugLog("Contents of UAA: " + this->dr);
    /// \todo LOCALIZE ME
    QString uaasum = "Reporting " + this->User->Username + " to UAA " + Configuration::HuggleConfiguration->EditSuffixOfHuggle;
    this->whatToReport();
    this->insertUsername();
    Core::HuggleCore->EditPage(Configuration::HuggleConfiguration->UAAP, dr, uaasum, true);
    /// \todo LOCALIZE ME
    Huggle::Syslog::HuggleLogs->Log("Reporting" + this->User->Username + " to UAA" );
    this->ui->pushButton->setText("Reported");

}
void UAAReport::insertUsername()
{
    this->ta = Configuration::HuggleConfiguration->LocalConfig_UAATemplate;
    this->ta.replace("$1", this->User->Username);
    this->ta.replace("$2", this->UAAReportReason + this->OptionalReason);
    this->ContentsOfUAA = this->ta + this->UAAReportReason + this->OptionalReason;
    this->dr = this->dr + "\n" + this->ContentsOfUAA;
}

void UAAReport::whatToReport()
{
    this->OptionalReason = this->ui->lineEdit->text();
    if (this->ui->checkBox->isChecked())
    {
        this->UAAReportReason = "Username is a policy violation because it is disruptive.";
    }
    if (this->ui->checkBox_2->isChecked())
    {
        this->UAAReportReason = "Username is a policy violation because it is offensive.";
    }
    if (this->ui->checkBox_3->isChecked())
    {
        this->UAAReportReason = "Username is a policy violation because it is a promotional username.";
    }
    if (this->ui->checkBox_4->isChecked())
    {
        this->UAAReportReason = "Username is a policy violation because it is a misleading username.";
    }
}

void UAAReport::failed(QString reason)
{
    QMessageBox *_b = new QMessageBox();
    _b->setWindowTitle("Unable to report user to UAA");
    _b->setText("Unable to report the user because " + reason);
    _b->setAttribute(Qt::WA_DeleteOnClose);
    _b->exec();
    this->uT->stop();
    return;
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
    this->qChUAApage = new ApiQuery();
    this->qChUAApage->SetAction(ActionQuery);
    this->qChUAApage->Parameters = "prop=revisons&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") + "titles="
            + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->LocalConfig_UAAPath);
    this->qChUAApage->RegisterConsumer("UAAReport::checkIfReported()");
    Core::HuggleCore->AppendQuery(qChUAApage);
    this->qChUAApage->Process();

    this->cuT = new QTimer(this);
    connect(this->cuT, SIGNAL(timeout()), this, SLOT(onStartOfSearch()));
    this->cuT->start(100);
}

bool UAAReport::checkIfReported()
{
    if (this->dr.contains(this->User->Username))
    {
       return false;
    }
    return true;
}

void UAAReport::onStartOfSearch()
{
    if (this->qChUAApage == NULL)
    {
        return;
    }
    if (!this->qChUAApage->Processed())
    {
        return;
    }
    QDomDocument tj;
    tj.setContent(qChUAApage->Result->Data);
    QDomNodeList chkusr = tj.elementsByTagName("rev");
    if (chkusr.count() == 0)
    {
        QMessageBox *msgb = new QMessageBox();
        msgb->setWindowTitle("Cannot retrieve page");
        msgb->setIcon(QMessageBox::Critical);
        msgb->setText("Retrieving the page " + Configuration::HuggleConfiguration->LocalConfig_UAAPath + " failed.");
        msgb->setAttribute(Qt::WA_DeleteOnClose);
        msgb->exec();
        this->qChUAApage->UnregisterConsumer("UAAReport::on_pushButton_3_clicked()");
        this->cuT->stop();
        return;
    }
    QDomElement h = chkusr.at(0).toElement();
    dr = h.text();
    if (!this->checkIfReported())
    {
        QMessageBox *msg = new QMessageBox();
        msg->setWindowTitle("User is already reported");
        msg->setText("This user has already been reported to UAA.");
        msg->setAttribute(Qt::WA_DeleteOnClose);
        msg->exec();
        this->qChUAApage->UnregisterConsumer("UAAReport::on_pushButton_3_clicked()");
        this->cuT->stop();
        return;
    }else
    {
        QMessageBox *msga = new QMessageBox();
        msga->setWindowTitle("User is not reported");
        msga->setText("This user is not reported to UAA.");
        msga->setAttribute(Qt::WA_DeleteOnClose);
        msga->exec();
        this->qChUAApage->UnregisterConsumer("UAAReport::on_pushButton_3_clicked()");
        this->cuT->stop();
        return;
    }
}
