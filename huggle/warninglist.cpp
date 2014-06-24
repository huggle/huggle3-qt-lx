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
#include "warnings.hpp"
#include "ui_warninglist.h"
#include "wikiuser.hpp"

using namespace Huggle;

WarningList::WarningList(WikiEdit *edit, QWidget *parent) : QDialog(parent), ui(new Ui::WarningList)
{
    this->ui->setupUi(this);
    this->wikiEdit = edit;
    if (edit->User == nullptr)
    {
        // unlikely to happen
        throw new Huggle::Exception("null user", "WarningList::WarningList(WikiEdit *edit, QWidget *parent) : QDialog(parent), ui(new Ui::WarningList)");
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
    QString wt = HuggleParser::GetKeyOfWarningTypeFromWarningName(this->ui->comboBox->currentText());
    if (wt.isEmpty())
    {
        QMessageBox mb;
        mb.setWindowTitle("No warning");
        mb.setText("There is no data for this type of warning, unable to deliver a warning to user!");
        mb.exec();
        return;
    }
    bool Report_ = false;
    PendingWarning *ptr_Warning_ = Warnings::WarnUser(wt, nullptr, this->wikiEdit, &Report_);
    if (Report_)
    {
        QMessageBox::StandardButton q = QMessageBox::question(nullptr, "Warning" , "This user has already received a final warning"\
                                                              ", so I will not send any more warnings to them, do you want to"\
                                                              " report them instead?", QMessageBox::Yes|QMessageBox::No);
        if (q == QMessageBox::Yes)
        {
            Core::HuggleCore->Main->DisplayReportUserWindow(this->wikiEdit->User);
        }
    }
    if (ptr_Warning_ != nullptr)
        PendingWarning::PendingWarnings.append(ptr_Warning_);
    this->wikiEdit.Delete();
    this->hide();
}
