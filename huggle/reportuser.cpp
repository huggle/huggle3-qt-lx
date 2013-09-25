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
    ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->pushButton->setEnabled(false);
    ui->pushButton->setText("Retrieving history...");
    QStringList header;
    ui->tableWidget->setColumnCount(5);
    header << "Page" << "Time" << "Link" << "DiffID" << "Include in report";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
#if QT_VERSION >= 0x050000
// Qt5 code
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    //ui->tableWidget->horizontalHeaderItem(0)->setSizeHint(QSize(20,-1));
    ui->tableWidget->setShowGrid(false);
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
        QDomDocument d;
        d.setContent(q->Result->Data);
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
                ui->tableWidget->insertRow(0);
                ui->tableWidget->setItem(0, 0, new QTableWidgetItem(page));
                ui->tableWidget->setItem(0, 1, new QTableWidgetItem(time));
                ui->tableWidget->setItem(0, 2, new QTableWidgetItem(link));
                ui->tableWidget->setItem(0, 3, new QTableWidgetItem(diff));
                QCheckBox *Item = new QCheckBox(this);
                ui->tableWidget->setCellWidget(0, 4, Item);
                xx++;
            }
        }
        this->timer->stop();
        ui->pushButton->setEnabled(true);
        ui->pushButton->setText("Report");
    }
}

void ReportUser::on_pushButton_clicked()
{

}

void ReportUser::on_pushButton_2_clicked()
{
    QUrl u = QUrl::fromEncoded(QString(Core::GetProjectWikiURL() + QUrl::toPercentEncoding
                                   (this->user->GetTalk()) + "?action=render").toUtf8());
    ui->webView->load(u);
}

void ReportUser::on_tableWidget_clicked(const QModelIndex &index)
{
    QUrl u = QUrl::fromEncoded(QString(Core::GetProjectScriptURL() + "index.php?title=" + QUrl::toPercentEncoding(ui->tableWidget->itemAt(index.row(), 0)->text()) + "&diff="
              + ui->tableWidget->itemAt(index.row(), 4)->text()).toUtf8());
    ui->webView->load(u);
}
