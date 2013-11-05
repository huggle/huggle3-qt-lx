//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "huggletool.h"
#include "ui_huggletool.h"

using namespace Huggle;

HuggleTool::HuggleTool(QWidget *parent) : QDockWidget(parent), ui(new Ui::HuggleTool)
{
    ui->setupUi(this);
    this->query = NULL;
    this->tick = new QTimer(this);
    connect(this->tick, SIGNAL(timeout()), this, SLOT(onTick()));
    this->DefaultFont = ui->comboBox->lineEdit()->font();
    this->edit = NULL;
}

HuggleTool::~HuggleTool()
{
    delete tick;
    delete ui;
}

void HuggleTool::SetTitle(QString title)
{
    ui->lineEdit->setText(title);
    ui->comboBox_2->lineEdit()->setText(title);
}

void HuggleTool::SetInfo(QString info)
{
    ui->lineEdit->setText(info);
}

void HuggleTool::SetUser(QString user)
{
    ui->comboBox->lineEdit()->setText(user);
}

void HuggleTool::SetPage(WikiPage *page)
{
    if (page == NULL)
    {
        throw new Exception("HuggleTool::SetPage(WikiPage* page) page must not be null");
    }
    this->ui->comboBox_2->lineEdit()->setText(page->PageName);
    this->tick->stop();
    this->DeleteQuery();
    this->ui->pushButton->setEnabled(true);
    // change color to default
    this->ui->comboBox_2->lineEdit()->setStyleSheet("color: black;");
}

void Huggle::HuggleTool::on_pushButton_clicked()
{
    if (this->ui->comboBox_2->lineEdit()->text() == "")
    {
        return;
    }
    ui->pushButton->setEnabled(false);
    this->ui->comboBox_2->lineEdit()->setStyleSheet("color: green;");
    // retrieve information about the page
    this->query = new ApiQuery();
    QueryPhase = 1;
    this->query->SetAction(ActionQuery);
    this->query->Parameters = "prop=revisions&rvprop=ids%7Cflags%7Ctimestamp%7Cuser%7Cuserid%7Csize%7Csha1%7Ccomment&rvlimit=1&titles="
                                + QUrl::toPercentEncoding(this->ui->comboBox_2->lineEdit()->text());
    this->query->RegisterConsumer(HUGGLECONSUMER_HUGGLETOOL);
    this->query->Process();
    this->tick->start(200);
}

void HuggleTool::onTick()
{
    switch (this->QueryPhase)
    {
    case 0:
        this->tick->stop();
        return;
    case 1:
        this->FinishPage();
        return;
    case 2:
        this->FinishEdit();
        return;
    }
}

void HuggleTool::FinishPage()
{
    if (this->query == NULL)
    {
        return;
    }
    if (!this->query->Processed())
    {
        return;
    }

    edit = new WikiEdit();
    edit->RegisterConsumer("MainForm");
    edit->Page = new WikiPage(this->ui->comboBox_2->lineEdit()->text());
    QDomDocument d;
    d.setContent(this->query->Result->Data);
    QDomNodeList l = d.elementsByTagName("rev");
    if (l.count() > 0)
    {
        QDomElement e = l.at(0).toElement();
        if (e.attributes().contains("missing"))
        {
            // there is no such a page
            this->DeleteQuery();
            this->ui->comboBox_2->lineEdit()->setStyleSheet("color: red;");
            /// \todo LOCALIZE ME
            Core::Log("There is no page " + ui->comboBox_2->lineEdit()->text() + " on wiki");
            this->tick->stop();
            return;
        }
        if (e.attributes().contains("user"))
        {
            edit->User = new WikiUser(e.attribute("user"));
        }
        if (e.attributes().contains("revid"))
        {
            edit->RevID = e.attribute("revid").toInt();
        }
    }
    if (edit->User == NULL)
    {
        edit->User = new WikiUser();
    }
    Core::PostProcessEdit(edit);
    edit->UnregisterConsumer(HUGGLECONSUMER_WIKIEDIT);
    this->QueryPhase = 2;
}

void HuggleTool::FinishEdit()
{
    if (this->edit == NULL)
    {
        return;
    }
    if (!this->edit->IsPostProcessed())
    {
        return;
    }
    this->tick->stop();
    Core::Main->ProcessEdit(edit);
}

void HuggleTool::DeleteQuery()
{
    if (this->query == NULL)
    {
        return;
    }
    this->query->UnregisterConsumer(HUGGLECONSUMER_HUGGLETOOL);
    this->query = NULL;
}
