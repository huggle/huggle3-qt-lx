//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "protectpage.h"
#include "ui_protectpage.h"

using namespace Huggle;

ProtectPage::ProtectPage(QWidget *parent) : QDialog(parent), ui(new Ui::ProtectPage)
{
    ui->setupUi(this);
    this->ptkk = NULL;
    this->ptkq = NULL;
    this->ptpge = NULL;
    this->ptpt = NULL;
    this->ui->comboBox_3->addItem("Everyone (no protection)");
    this->ui->comboBox_3->addItem("Autoconfirmed (semi)");
    this->ui->comboBox_3->addItem("Sysops (full)");
    this->ui->comboBox_3->setCurrentIndex(2);
    this->protecttoken = "";
    this->tt = NULL;
    ui->comboBox->addItem(Configuration::LocalConfig_ProtectReason);
}

ProtectPage::~ProtectPage()
{
    delete ui;
    delete ptpge;
    delete tt;
}

void ProtectPage::setPageToProtect(WikiPage *Page)
{
    this->ptpge = Page;
}

void ProtectPage::getTokenToProtect()
{
    this->ptkq = new ApiQuery();
    ptkq->SetAction(ActionQuery);
    ptkq->Parameters = "prop=info&intoken=protect&titles=" + QUrl::toPercentEncoding(this->ptpge->PageName);
    ptkq->Target = "Fetching token to protect" + this->ptpge->PageName;
    ptkq->RegisterConsumer("ProtectPage::getTokenToProtect()");
    Core::AppendQuery(ptkq);
    ptkq->Process();

    this->tt = new QTimer(this);
    connect(this->tt, SIGNAL(timeout()), this, SLOT(onTick()));
    this->PtQueryPhase = 0;
    this->tt->start(200);
}

void ProtectPage::onTick()
{
    switch(this->PtQueryPhase)
    {
    case 0:
        this->checkTokenToProtect();
        return;
    case 1:
        this->Protect();
        return;
    }
    this->tt->stop();
}

void ProtectPage::checkTokenToProtect()
{
    if(ptkq == NULL)
    {
        return;
    }
    if(!ptkq->Processed())
    {
        return;
    }
    if(ptkq->Result->Failed)
    {
        Failed("ERROR: Token cannot be retrieved. The reason was: " + ptkq->Result->ErrorMessage);
        return;
    }
    QDomDocument r;
    r.setContent(ptkq->Result->Data);
    QDomNodeList l = r.elementsByTagName("page");
    if (l.count() == 0)
    {
        Core::DebugLog(this->ptkq->Result->Data);
        Failed("No page info was available (are you an admin?)");
        return;
    }
    QDomElement element = l.at(0).toElement();
    if (!element.attributes().contains("protecttoken"))
    {
        Failed("No token");
        return;
    }
    this->protecttoken = element.attribute("protecttoken");
    this->PtQueryPhase++;
    this->ptkq->UnregisterConsumer("ProtectPage::getTokenToProtect");
    this->ptkq = NULL;
    Core::DebugLog("Protection token for " + this->ptpge->PageName + ": " + this->protecttoken);
    this->ptpt = new ApiQuery();
    ptpt->SetAction(ActionProtect);
    QString protection = "edit=sysop|move=sysop";
    switch (this->ui->comboBox_3->currentIndex())
    {
    case 0:
        protection = "edit=all|move=all";
        break;
    case 1:
        protection = "edit=autoconfirmed|move=autoconfirmed";
        break;
    }
    ptpt->Parameters = "title=" + QUrl::toPercentEncoding(this->ptpge->PageName)
            + "&reason=" + QUrl::toPercentEncoding(Configuration::LocalConfig_ProtectReason)
            + "&expiry=" + QUrl::toPercentEncoding(ui->comboBox_2->currentText())
            + "&protections=" + QUrl::toPercentEncoding(protection)
            + "&token=" + QUrl::toPercentEncoding(protecttoken);
    ptpt->Target = "Protecting " + this->ptpge->PageName;
    ptpt->RegisterConsumer("ProtectPage");
    Core::AppendQuery(ptpt);
    ptpt->Process();
}

void ProtectPage::on_pushButton_clicked()
{
    this->hide();
}

void ProtectPage::on_pushButton_2_clicked()
{
    this->getTokenToProtect();
    this->ui->pushButton_2->setEnabled(false);
}

void ProtectPage::Failed(QString reason)
{
    QMessageBox *_pmb = new QMessageBox();
    _pmb->setWindowTitle("Unable to protect page");
    _pmb->setText("Unable to protect the page because " + reason);
    _pmb->exec();
    delete _pmb;
    this->tt->stop();
    delete this->tt;
    this->tt = NULL;
    ui->pushButton->setEnabled(true);
    if (this->ptkq != NULL)
    {
        this->ptkq->UnregisterConsumer("ProtectPage::getTokenToProtect");
    }
    if (this->ptpt != NULL)
    {
        this->ptpt->UnregisterConsumer("ProtectPage");
    }
    this->ptpt = NULL;
    this->ptkq = NULL;
}

void ProtectPage::Protect()
{
    if (!this->ptpt->Processed())
    {
        return;
    }
    if (ptpt == NULL)
    {
        return;
    }
    if (ptpt->Result->Failed)
    {
        Failed("The API query failed. Reason supplied was: " + ptpt->Result->ErrorMessage);
        return;
    }
    ui->pushButton_2->setText("Page has been protected");
    Core::DebugLog("The page " + ptpge->PageName + " has successfully been protected");
    this->ptpt->UnregisterConsumer("ProtectPage");
    this->tt->stop();
    ptpt = NULL;
}

