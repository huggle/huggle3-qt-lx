//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "historyform.hpp"
#include "ui_historyform.h"

using namespace Huggle;

HistoryForm::HistoryForm(QWidget *parent) : QDockWidget(parent), ui(new Ui::HistoryForm)
{
    ui->setupUi(this);
    ui->pushButton->setEnabled(false);
    ui->pushButton->setText(Core::Localize("historyform-no-info"));
    ui->tableWidget->setColumnCount(5);
    QStringList header;
    header << "ID" << "Date" << "User" << "Size" << "Summary";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
#if QT_VERSION >= 0x050000
// Qt5 code
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    ui->tableWidget->setShowGrid(false);
    this->query = NULL;
    this->t1 = NULL;
}

HistoryForm::~HistoryForm()
{
    delete t1;
    delete ui;
}

void HistoryForm::Update(WikiEdit *edit)
{
    if (edit == NULL)
    {
        throw new Exception("WikiEdit edit must not be NULL", "void HistoryForm::Update(WikiEdit *edit)");
    }
    this->CurrentEdit = edit;
    this->ui->pushButton->setText(Core::Localize("historyform-retrieve-history"));
    this->ui->pushButton->setEnabled(true);
    if (this->t1 != NULL)
    {
        this->t1->stop();
        delete this->t1;
        this->t1 = NULL;
    }
    if (this->query != NULL)
    {
        this->query->UnregisterConsumer("HistoryForm");
        this->query = NULL;
    }
}

void HistoryForm::onTick01()
{
    if (this->query == NULL)
    {
        return;
    }
    if (!this->query->Processed())
    {
        return;
    }
    if (this->query->Result->Failed)
    {
        /// \todo Here we should log this error to debug log
        ui->pushButton->setEnabled(true);
        Core::Log("Error: unable to retrieve history");
        this->query->UnregisterConsumer("HistoryForm");
        this->query = NULL;
        this->t1->stop();
        return;
    }
    QDomDocument d;
    d.setContent(this->query->Result->Data);
    QDomNodeList l = d.elementsByTagName("rev");
    int x=0;
    while (x < l.count())
    {
        QDomElement e = l.at(x).toElement();
        QString RevID;
        if (e.attributes().contains("revid"))
        {
            RevID = e.attribute("revid");
        } else
        {
            x++;
            continue;
        }
        QString user = "<Unknown>";
        if (e.attributes().contains("user"))
        {
            user = e.attribute("user");
        }
        QString size = "<Unknown>";
        if (e.attributes().contains("size"))
        {
            size = e.attribute("size");
        }
        QString date = "<unknown>";
        if (e.attributes().contains("timestamp"))
        {
            date = e.attribute("timestamp");
        }
        QString summary = "No summary";
        if (e.attributes().contains("comment"))
        {
            if (e.attribute("comment") != "")
            {
                summary = e.attribute("comment");
            }
        }
        ui->tableWidget->insertRow(x);
        ui->tableWidget->setItem(x, 0, new QTableWidgetItem(RevID));
        ui->tableWidget->setItem(x, 1, new QTableWidgetItem(date));
        ui->tableWidget->setItem(x, 2, new QTableWidgetItem(user));
        ui->tableWidget->setItem(x, 3, new QTableWidgetItem(size));
        ui->tableWidget->setItem(x, 4, new QTableWidgetItem(summary));
        x++;
    }
    this->query->UnregisterConsumer("HistoryForm");
    this->query = NULL;
    this->t1->stop();
}

void HistoryForm::on_pushButton_clicked()
{
    ui->pushButton->setText(Core::Localize("historyform-retrieving-history"));
    ui->pushButton->setEnabled(false);
    this->query = new ApiQuery();
    this->query->SetAction(ActionQuery);
    this->query->Parameters = "prop=revisions&rvprop=ids%7Cflags%7Ctimestamp%7Cuser%7Cuserid%7Csize%7Csha1%7Ccomment&rvlimit=20&titles="
                                    + QUrl::toPercentEncoding(this->CurrentEdit->Page->PageName);
    this->query->RegisterConsumer("HistoryForm");
    this->query->Process();
    if (t1 != NULL)
    {
        delete t1;
    }
    this->t1 = new QTimer(this);
    ui->tableWidget->clear();
    QStringList header;
    header << "ID" << "Date" << "User" << "Size" << "Summary";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    connect(t1, SIGNAL(timeout()), this, SLOT(onTick01()));
    this->t1->start(200);
}
