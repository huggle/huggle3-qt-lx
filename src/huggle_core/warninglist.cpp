//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "warninglist.hpp"
#include <QMessageBox>
#include "configuration.hpp"
#include "core.hpp"
#include "exception.hpp"
#include "huggleparser.hpp"
#include "mainwindow.hpp"
#include "localization.hpp"
#include "reportuser.hpp"
#include "warnings.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"
#include "ui_warninglist.h"

using namespace Huggle;

WarningList::WarningList(WikiEdit *edit, QWidget *parent) : QDialog(parent), ui(new Ui::WarningList)
{
    this->ui->setupUi(this);
    this->wikiEdit = edit;
    if (edit->User == nullptr)
    {
        // unlikely to happen
        throw new Huggle::NullPointerException("WikiEdit *edit->User", BOOST_CURRENT_FUNCTION);
    }
    this->setWindowTitle(_l("warning-title", edit->User->Username));
    this->ui->pushButton->setText(_l(this->ui->pushButton->text()));
    // insert all possible warnings now
    if (Configuration::HuggleConfiguration->ProjectConfig->WarningTypes.count() > 0)
    {
        int r=0;
        while (r<Configuration::HuggleConfiguration->ProjectConfig->WarningTypes.count())
        {
            this->ui->comboBox->addItem(HuggleParser::GetValueFromKey(Configuration::HuggleConfiguration->ProjectConfig->WarningTypes.at(r)));
            r++;
        }
        this->ui->comboBox->setCurrentIndex(0);
    }
}

WarningList::~WarningList()
{
    this->wikiEdit.Delete();
    delete this->ui;
}

void WarningList::on_pushButton_clicked()
{
    if (this->wikiEdit == nullptr)
    {
        // ERROR :'(
        throw new Huggle::NullPointerException("local WikiEdit wikiEdit", BOOST_CURRENT_FUNCTION);
    }
    QString wt = HuggleParser::GetKeyOfWarningTypeFromWarningName(this->ui->comboBox->currentText(), this->wikiEdit->GetSite()->GetProjectConfig());
    if (wt.isEmpty())
    {
        QMessageBox mb;
        mb.setWindowTitle(_l("warninglist-no-warning-title"));
        mb.setText(_l("warninglist-no-warning-text"));
        mb.exec();
        return;
    }
    bool Report_ = false;
    PendingWarning *ptr_Warning_ = Warnings::WarnUser(wt, nullptr, this->wikiEdit, &Report_);
    if (Report_)
    {
        if ((hcfg->UserConfig->AutomaticReports && this->wikiEdit->GetSite()->GetProjectConfig()->ReportMode != ReportType_StrictManual) || this->wikiEdit->GetSite()->GetProjectConfig()->ReportMode == ReportType_StrictAuto)
        {
            ReportUser::SilentReport(this->wikiEdit->User);
        } else
        {
            QMessageBox::StandardButton q = QMessageBox::question(nullptr, _l("warning") , _l("warninglist-report-text"), QMessageBox::Yes|QMessageBox::No);
            if (q == QMessageBox::Yes)
            {
                MainWindow::HuggleMain->DisplayReportUserWindow(this->wikiEdit->User);
            }
        }
    }
    if (ptr_Warning_ != nullptr)
        PendingWarning::PendingWarnings.append(ptr_Warning_);
    this->wikiEdit.Delete();
    this->hide();
}
