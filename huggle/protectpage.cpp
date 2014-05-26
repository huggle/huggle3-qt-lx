//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "protectpage.hpp"
#include <QMessageBox>
#include "configuration.hpp"
#include "querypool.hpp"
#include "ui_protectpage.h"

using namespace Huggle;

ProtectPage::ProtectPage(QWidget *parent) : QDialog(parent), ui(new Ui::ProtectPage)
{
    this->ui->setupUi(this);
    this->qToken = nullptr;
    this->PageToProtect = nullptr;
    this->qProtection = nullptr;
    this->ui->comboBox_3->addItem(Localizations::HuggleLocalizations->Localize("protect-none"));
    this->ui->comboBox_3->addItem(Localizations::HuggleLocalizations->Localize("protect-semiprotection"));
    this->ui->comboBox_3->addItem(Localizations::HuggleLocalizations->Localize("protect-fullprotection"));
    this->ui->comboBox_3->setCurrentIndex(2);
    this->ProtectToken = "";
    this->tt = nullptr;
    this->ui->comboBox->addItem(Configuration::HuggleConfiguration->ProjectConfig_ProtectReason);
}

ProtectPage::~ProtectPage()
{
    this->DelRefs();
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
    this->qToken = new ApiQuery();
    this->qToken->SetAction(ActionQuery);
    this->qToken->Parameters = "prop=info&intoken=protect&titles=" + QUrl::toPercentEncoding(this->PageToProtect->PageName);
    this->qToken->Target = Localizations::HuggleLocalizations->Localize("protection-ft");
    this->qToken->IncRef();
    QueryPool::HugglePool->AppendQuery(qToken);
    this->qToken->Process();
    this->tt = new QTimer(this);
    connect(this->tt, SIGNAL(timeout()), this, SLOT(onTick()));
    this->PtQueryPhase = 0;
    this->tt->start(200);
}

void ProtectPage::DelRefs()
{
    GC_DECREF(this->qToken);
    GC_DECREF(this->qProtection);
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
    if (this->qToken == nullptr || !this->qToken->IsProcessed())
        return;
    if (this->qToken->Result->Failed)
    {
        /// \todo LOCALIZE ME
        this->Failed("ERROR: Token cannot be retrieved. The reason was: " + this->qToken->Result->ErrorMessage);
        return;
    }
    QDomDocument r;
    r.setContent(qToken->Result->Data);
    QDomNodeList l = r.elementsByTagName("page");
    if (l.count() == 0)
    {
        Huggle::Syslog::HuggleLogs->DebugLog(this->qToken->Result->Data);
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
    this->qToken->DecRef();
    this->qToken = nullptr;
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
            + "&reason=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->ProjectConfig_ProtectReason)
            + "&expiry=" + QUrl::toPercentEncoding(this->ui->comboBox_2->currentText())
            + "&protections=" + QUrl::toPercentEncoding(protection)
            + "&token=" + QUrl::toPercentEncoding(this->ProtectToken);
    this->qProtection->Target = "Protecting " + this->PageToProtect->PageName;
    this->qProtection->IncRef();
    QueryPool::HugglePool->AppendQuery(this->qProtection);
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
    this->DelRefs();
    this->tt = nullptr;
    ui->pushButton->setEnabled(true);
}

void ProtectPage::Protect()
{
    if (!this->qProtection->IsProcessed() || this->qProtection == nullptr)
    {
        return;
    }
    if (this->qProtection->Result->Failed)
    {
        Failed("The API query failed. Reason supplied was: " + qProtection->Result->ErrorMessage);
        return;
    }
    this->ui->pushButton_2->setText("Page has been protected");
    Huggle::Syslog::HuggleLogs->DebugLog("The page " + PageToProtect->PageName + " has successfully been protected");
    this->qProtection->DecRef();
    this->tt->stop();
    this->qProtection = nullptr;
}

