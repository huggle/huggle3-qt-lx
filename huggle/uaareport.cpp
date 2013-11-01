//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "uaareport.h"
#include "ui_uaareport.h"

using namespace Huggle;

UAAReport::UAAReport(QWidget *parent) : QDialog(parent), ui(new Ui::UAAReport)
{
    ui->setupUi(this);
    uaat = NULL;
    User = NULL;
}

UAAReport::~UAAReport()
{
    delete ui;
    delete uaat;
}

void UAAReport::setUserForUAA(WikiUser *user)
{
    this->User = user;
}

void UAAReport::whatToReport()
{
    QString optionalreason;
    optionalreason = this->ui->lineEdit->text();
    QString uaaReportReason;

    if (this->ui->checkBox->isChecked())
    {
        uaaReportReason = "Username is a policy violation because it is disruptive.";
    }
    if (this->ui->checkBox_2->isChecked())
    {
        uaaReportReason = "Username is a policy violation because it is offensive.";
    }
    if (this->ui->checkBox_3->isChecked())
    {
        uaaReportReason = "Username is a policy violation because it is a promotional username.";
    }
    if (this->ui->checkBox_4->isChecked())
    {
        uaaReportReason = "Username is a policy violation because it is a misleading username.";
    }
    if (!this->ui->checkBox->isChecked() && !this->ui->checkBox_2->isChecked() &&
            !this->ui->checkBox_3->isChecked() && !this->ui->checkBox_4->isChecked()
            && this->ui->lineEdit->text().isEmpty())
    {
        QMessageBox *g = new QMessageBox();
        g->setWindowTitle("No reason specified");
        g->setText("You didn't specify a reason as to why the username is a policy violation."\
                   "Please specify a reason.");
        g->exec();
        delete g;
    }
    QString ta = Configuration::LocalConfig_UAATemplate;
    ta.replace("$1", this->User->Username);
    contentsOfUAA = ta + uaaReportReason + optionalreason;
}

void UAAReport::onTick()
{
    
}

void UAAReport::insertUsername()
{
    QString uaasection;
}

void UAAReport::ReportUsername()
{

}

void UAAReport::Failed(QString reason)
{

}

void UAAReport::on_pushButton_clicked()
{ 
    QString uaasum = "Reporting " + this->User->Username + " to UAA " + Configuration::EditSuffixOfHuggle;
    Core::EditPage(Core::UAAP, contentsOfUAA, uaasum, true);
    this->ui->pushButton->setText("Reported");
}

void UAAReport::on_pushButton_2_clicked()
{
    this->hide();
}
