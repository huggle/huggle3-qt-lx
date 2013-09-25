//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.


#include "reportuser.h"
#include "ui_reportuser.h"

ReportUser::ReportUser(QWidget *parent) : QDialog(parent), ui(new Ui::ReportUser)
{
    ui->setupUi(this);
    this->user = NULL;
    this->q = NULL;
    ui->pushButton->setEnabled(false);
    ui->pushButton->setText("Retrieving history...");
    QStringList header;
    header << "Page" << "Time" << "Link" << "DiffID";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    this->timer = NULL;
}

bool ReportUser::SetUser(WikiUser *u)
{
    if (q != NULL)
    {
        delete q;
        q = NULL;
    }
    this->user = u;
    ui->label->setText(u->Username);
    q = new ApiQuery();
    q->Parameters = "list=recentchanges&rcuser=" + QUrl::toPercentEncoding(u->Username) +
            "&rcprop=user%7Ccomment%7Ctimestamp%7Ctitle%7Cids%7Csizes&rclimit=20&rctype=edit%7Cnew";
    q->SetAction(ActionQuery);
    q->Process();
    this->timer = new QTimer(this);
    connect(this->timer, SIGNAL(timeout()), this, SLOT(Tick()));
    this->timer->start(200);
    return true;
}

ReportUser::~ReportUser()
{
    delete q;
    delete ui;
}

void ReportUser::Tick()
{
    if (this->q == NULL)
    {
        return;
    }
    if (q->Processed())
    {
        Core::DebugLog(q->Result->Data);
        this->timer->stop();
    }
}
