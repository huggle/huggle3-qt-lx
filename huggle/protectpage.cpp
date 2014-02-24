//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "protectpage.hpp"
#include "ui_protectpage.h"

using namespace Huggle;

ProtectPage::ProtectPage(QWidget *parent) : QDialog(parent), ui(new Ui::ProtectPage)
{
    this->ui->setupUi(this);
    this->qToken2 = NULL;
    this->qToken1 = NULL;
    this->PageToProtect = NULL;
    this->qProtection = NULL;
    this->ui->comboBox_3->addItem(Localizations::HuggleLocalizations->Localize("protect-none"));
    this->ui->comboBox_3->addItem(Localizations::HuggleLocalizations->Localize("protect-semiprotection"));
    this->ui->comboBox_3->addItem(Localizations::HuggleLocalizations->Localize("protect-fullprotection"));
    this->ui->comboBox_3->setCurrentIndex(2);
    this->ProtectToken = "";
    this->tt = NULL;
    this->ui->comboBox->addItem(Configuration::HuggleConfiguration->LocalConfig_ProtectReason);
}

ProtectPage::~ProtectPage()
{
    delete this->ui;
    delete this->PageToProtect;
    delete this->tt;
}

void ProtectPage::setPageToProtect(WikiPage *Page)
{
    this->PageToProtect = Page;
}

void ProtectPage::getTokenToProtect()
{
    this->qToken1 = new ApiQuery();
    this->qToken1->SetAction(ActionQuery);
    this->qToken1->Parameters = "prop=info&intoken=protect&titles=" + QUrl::toPercentEncoding(this->PageToProtect->PageName);
    this->qToken1->Target = Localizations::HuggleLocalizations->Localize("protection-ft");
    this->qToken1->RegisterConsumer(HUGGLECONSUMER_PROTECTPAGE);
    Core::HuggleCore->AppendQuery(qToken1);
    this->qToken1->Process();
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
    if (this->qToken1 == NULL)
    {
        return;
    }
    if (!this->qToken1->IsProcessed())
    {
        return;
    }
    if (this->qToken1->Result->Failed)
    {
        /// \todo LOCALIZE ME
        this->Failed("ERROR: Token cannot be retrieved. The reason was: " + this->qToken1->Result->ErrorMessage);
        return;
    }
    QDomDocument r;
    r.setContent(qToken1->Result->Data);
    QDomNodeList l = r.elementsByTagName("page");
    if (l.count() == 0)
    {
        Huggle::Syslog::HuggleLogs->DebugLog(this->qToken1->Result->Data);
        /// \todo LOCALIZE ME
        this->Failed("No page info was available (are you an admin?)");
        return;
    }
    QDomElement element = l.at(0).toElement();
    if (!element.attributes().contains("protecttoken"))
    {
        this->Failed("No token");
        return;
    }
    this->ProtectToken = element.attribute("protecttoken");
    this->PtQueryPhase++;
    this->qToken1->UnregisterConsumer(HUGGLECONSUMER_PROTECTPAGE);
    this->qToken1 = NULL;
    Huggle::Syslog::HuggleLogs->DebugLog("Protection token for " + this->PageToProtect->PageName + ": " + this->ProtectToken);
    this->qProtection = new ApiQuery();
    this->qProtection->SetAction(ActionProtect);
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
    this->qProtection->Parameters = "title=" + QUrl::toPercentEncoding(this->PageToProtect->PageName)
            + "&reason=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->LocalConfig_ProtectReason)
            + "&expiry=" + QUrl::toPercentEncoding(this->ui->comboBox_2->currentText())
            + "&protections=" + QUrl::toPercentEncoding(protection)
            + "&token=" + QUrl::toPercentEncoding(this->ProtectToken);
    this->qProtection->Target = "Protecting " + this->PageToProtect->PageName;
    this->qProtection->RegisterConsumer(HUGGLECONSUMER_PROTECTPAGE);
    Core::HuggleCore->AppendQuery(this->qProtection);
    this->qProtection->Process();
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
    /// \todo LOCALIZE ME
    _pmb->setWindowTitle("Unable to protect page");
    /// \todo LOCALIZE ME
    _pmb->setText("Unable to protect the page because " + reason);
    _pmb->exec();
    delete _pmb;
    this->tt->stop();
    delete this->tt;
    this->tt = NULL;
    ui->pushButton->setEnabled(true);
    if (this->qToken1 != NULL)
    {
        this->qToken1->UnregisterConsumer(HUGGLECONSUMER_PROTECTPAGE);
    }
    if (this->qProtection != NULL)
    {
        this->qProtection->UnregisterConsumer(HUGGLECONSUMER_PROTECTPAGE);
    }
    this->qProtection = NULL;
    this->qToken1 = NULL;
}

void ProtectPage::Protect()
{
    if (!this->qProtection->IsProcessed())
    {
        return;
    }
    if (this->qProtection == NULL)
    {
        return;
    }
    if (this->qProtection->Result->Failed)
    {
        /// \todo LOCALIZE ME
        Failed("The API query failed. Reason supplied was: " + qProtection->Result->ErrorMessage);
        return;
    }
    this->ui->pushButton_2->setText("Page has been protected");
    Huggle::Syslog::HuggleLogs->DebugLog("The page " + PageToProtect->PageName + " has successfully been protected");
    this->qProtection->UnregisterConsumer(HUGGLECONSUMER_PROTECTPAGE);
    this->tt->stop();
    this->qProtection = NULL;
}

