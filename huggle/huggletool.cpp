//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "huggletool.hpp"
#include "core.hpp"
#include "exception.hpp"
#include "configuration.hpp"
#include "querypool.hpp"
#include "syslog.hpp"
#include "ui_huggletool.h"

using namespace Huggle;

HuggleTool::HuggleTool(QWidget *parent) : QDockWidget(parent), ui(new Ui::HuggleTool)
{
    this->ui->setupUi(this);
    this->query = NULL;
    this->tick = new QTimer(this);
    this->ui->label->setText(Localizations::HuggleLocalizations->Localize("User"));
    this->ui->label_2->setText(Localizations::HuggleLocalizations->Localize("Page"));
    connect(this->tick, SIGNAL(timeout()), this, SLOT(onTick()));
    this->edit = NULL;
}

HuggleTool::~HuggleTool()
{
    this->DeleteQuery();
    delete this->tick;
    delete this->ui;
}

void HuggleTool::SetTitle(QString title)
{
    this->ui->lineEdit->setText(title);
    this->ui->lineEdit_3->setText(title);
}

void HuggleTool::SetInfo(QString info)
{
    this->ui->lineEdit->setText(info);
}

void HuggleTool::SetUser(QString user)
{
    this->ui->lineEdit_2->setText(user);
}

void HuggleTool::SetPage(WikiPage *page)
{
    if (page == NULL)
    {
        throw new Exception("HuggleTool::SetPage(WikiPage* page) page must not be null");
    }
    this->ui->lineEdit_3->setText(page->PageName);
    this->tick->stop();
    this->DeleteQuery();
    this->ui->pushButton->setEnabled(true);
    // change color to default
    this->ui->lineEdit_2->setStyleSheet("color: black;");
    this->ui->lineEdit_3->setStyleSheet("color: black;");
}

void HuggleTool::RenderEdit()
{
    if (!this->ui->pushButton->isEnabled() || !this->ui->lineEdit_3->text().length())
        return;
    this->ui->pushButton->setEnabled(false);
    this->ui->lineEdit_3->setStyleSheet("color: green;");
    // retrieve information about the page
    this->DeleteQuery();
    this->query = new ApiQuery(ActionQuery);
    this->QueryPhase = 1;
    this->query->Parameters = "prop=revisions&rvprop=ids%7Cflags%7Ctimestamp%7Cuser%7Cuserid%7Csize%7Csha1%7Ccomment&rvlimit=1&titles="
                                + QUrl::toPercentEncoding(this->ui->lineEdit_3->text());
    this->query->IncRef();
    this->query->Process();
    this->tick->start(200);
}

void Huggle::HuggleTool::on_pushButton_clicked()
{
    this->RenderEdit();
}

void HuggleTool::onTick()
{
    switch (this->QueryPhase)
    {
        case 0:
            this->tick->stop();
            return;
        case 1:
        case 3:
            this->FinishPage();
            return;
        case 2:
        case 4:
            this->FinishEdit();
            return;
    }
}

void HuggleTool::FinishPage()
{
    if (this->query == NULL || !this->query->IsProcessed())
        return;
    QDomDocument d;
    d.setContent(this->query->Result->Data);
    if (this->QueryPhase == 3)
    {
        this->DeleteQuery();
        QDomNodeList l = d.elementsByTagName("item");
        if (l.count() == 0)
        {
            this->ui->lineEdit_2->setStyleSheet("color: red;");
            this->tick->stop();
            return;
        }
        QDomElement first_one = l.at(0).toElement();
        if (!first_one.attributes().contains("title"))
        {
            this->ui->lineEdit_2->setStyleSheet("color: red;");
            this->tick->stop();
            return;
        }
        this->edit = new WikiEdit();
        this->edit->Page = new WikiPage(first_one.attribute("title"));
        this->edit->User = new WikiUser(first_one.attribute("user"));
        this->edit->RevID = first_one.attribute("revid").toInt();
        QueryPool::HugglePool->PostProcessEdit(this->edit);
        this->QueryPhase = 4;
    } else
    {
        this->edit = new WikiEdit();
        this->edit->Page = new WikiPage(this->ui->lineEdit_3->text());
        QDomNodeList l = d.elementsByTagName("rev");
        if (l.count() > 0)
        {
            QDomElement e = l.at(0).toElement();
            if (e.attributes().contains("missing"))
            {
                // there is no such a page
                this->DeleteQuery();
                this->ui->lineEdit_3->setStyleSheet("color: red;");
                Huggle::Syslog::HuggleLogs->WarningLog(Huggle::Localizations::HuggleLocalizations->Localize("missing-page", ui->lineEdit_3->text()));
                this->tick->stop();
                this->edit = NULL;
                return;
            }
            if (e.attributes().contains("user"))
            {
                this->edit->User = new WikiUser(e.attribute("user"));
            }
            if (e.attributes().contains("revid"))
            {
                this->edit->RevID = e.attribute("revid").toInt();
            }
        }
        if (this->edit->User == NULL)
        {
            this->edit->User = new WikiUser();
        }
        QueryPool::HugglePool->PostProcessEdit(this->edit);
        this->QueryPhase = 2;
    }
}

void HuggleTool::FinishEdit()
{
    if (this->edit == NULL || !this->edit->IsPostProcessed())
        return;
    this->tick->stop();
    this->ui->pushButton->setEnabled(true);
    Core::HuggleCore->Main->ProcessEdit(this->edit, false, false, false, true);
}

void HuggleTool::DeleteQuery()
{
    if (this->query == NULL)
        return;
    this->query->DecRef();
    this->query = NULL;
}

void Huggle::HuggleTool::on_lineEdit_3_returnPressed()
{
    this->RenderEdit();
}

void Huggle::HuggleTool::on_lineEdit_2_returnPressed()
{
    if (!this->ui->pushButton->isEnabled() || !this->ui->lineEdit_2->text().length())
    {
        return;
    }
    this->ui->pushButton->setEnabled(false);
    this->ui->lineEdit_2->setStyleSheet("color: green;");
    // retrieve information about the user
    this->DeleteQuery();
    this->query = new ApiQuery(ActionQuery);
    this->QueryPhase = 3;
    this->query->Parameters = "list=usercontribs&ucuser=" + QUrl::toPercentEncoding(this->ui->lineEdit_2->text()) +
                              "&ucprop=flags%7Ccomment%7Ctimestamp%7Ctitle%7Cids%7Csize&uclimit=20";
    this->query->IncRef();
    this->query->Process();
    this->tick->start(200);
}
