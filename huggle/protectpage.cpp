//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "protectpage.hpp"
#include "configuration.hpp"
#include "generic.hpp"
#include "history.hpp"
#include "historyitem.hpp"
#include "mainwindow.hpp"
#include "localization.hpp"
#include "querypool.hpp"
#include "syslog.hpp"
#include "ui_protectpage.h"
#include "wikisite.hpp"

using namespace Huggle;

ProtectPage::ProtectPage(QWidget *parent) : QDialog(parent), ui(new Ui::ProtectPage)
{
    this->ui->setupUi(this);
    this->PageToProtect = nullptr;
    this->ui->comboBox_3->addItem(_l("protect-none"));
    this->ui->comboBox_3->addItem(_l("protect-semiprotection"));
    this->ui->comboBox_3->addItem(_l("protect-fullprotection"));
    this->ui->comboBox_3->setCurrentIndex(2);
    this->tt = nullptr;
    this->ui->comboBox->addItem(Configuration::HuggleConfiguration->ProjectConfig->ProtectReason);
}

ProtectPage::~ProtectPage()
{
    delete this->ui;
    delete this->PageToProtect;
    delete this->tt;
}

void ProtectPage::setPageToProtect(WikiPage *Page)
{
    this->PageToProtect = new WikiPage(Page);
}

void ProtectPage::onTick()
{
    this->Protect();
}

void ProtectPage::on_pushButton_clicked()
{
    this->hide();
}

void ProtectPage::on_pushButton_2_clicked()
{
    this->ui->pushButton_2->setEnabled(false);
    this->qProtection = new ApiQuery(ActionProtect, this->PageToProtect->GetSite());
    this->qProtection->UsingPOST = true;
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
    // reason why protect
    QString reason = Configuration::GenerateSuffix(this->ui->comboBox->currentText(), this->PageToProtect->GetSite()->GetProjectConfig());
    this->qProtection->Parameters = "title=" + QUrl::toPercentEncoding(this->PageToProtect->PageName)
            + "&reason=" + QUrl::toPercentEncoding(reason)
            + "&expiry=" + QUrl::toPercentEncoding(this->ui->comboBox_2->currentText())
            + "&protections=" + QUrl::toPercentEncoding(protection)
            + "&token=" + QUrl::toPercentEncoding(this->PageToProtect->GetSite()->GetProjectConfig()->Token_Csrf);
    this->qProtection->Target = "Protecting " + this->PageToProtect->PageName;
    QueryPool::HugglePool->AppendQuery(this->qProtection);
    this->qProtection->Process();
    this->tt = new QTimer(this);
    connect(this->tt, SIGNAL(timeout()), this, SLOT(onTick()));
    this->tt->start(HUGGLE_TIMER);
}

void ProtectPage::Failed(QString reason)
{
    Generic::MessageBox(_l("protect-message-title-fail"), _l("protect-error", reason), MessageBoxStyleWarning, true);
    this->tt->stop();
    delete this->tt;
    this->qProtection.Delete();
    this->tt = nullptr;
    this->ui->pushButton->setEnabled(true);
}

void ProtectPage::Protect()
{
    if (!this->qProtection->IsProcessed() || this->qProtection == nullptr)
    {
        return;
    }
    if (this->qProtection->Result->IsFailed())
    {
        Failed("The API query failed. Reason supplied was: " + qProtection->Result->ErrorMessage);
        return;
    }
    Huggle::Syslog::HuggleLogs->DebugLog("The page " + PageToProtect->PageName + " has successfully been protected");
    HistoryItem *item = new HistoryItem();
    item->Type = HistoryProtect;
    item->Target = this->PageToProtect->PageName;
    item->Result = _l("successful");
    item->IsRevertable = false;
    item->NewPage = false;
    // insert to history
    MainWindow::HuggleMain->_History->Prepend(item);
    this->tt->stop();
    this->qProtection = nullptr;
    this->close();
}

