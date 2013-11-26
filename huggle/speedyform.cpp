//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "speedyform.hpp"
#include "ui_speedyform.h"

using namespace Huggle;

SpeedyForm::SpeedyForm(QWidget *parent) : QDialog(parent), ui(new Ui::SpeedyForm)
{
    this->Page = NULL;
    this->User = NULL;
    this->ui->setupUi(this);
    int i=0;
    while (i < Configuration::HuggleConfiguration->LocalConfig_SpeedyTemplates.count())
    {
        QString item = Configuration::HuggleConfiguration->LocalConfig_SpeedyTemplates.at(i);
        // now we need to get first 2 items
        QStringList vals = item.split(";");
        if (vals.count() < 4)
        {
            Huggle::Syslog::HuggleLogs->DebugLog("Invalid csd: " + item);
            i++;
            continue;
        }
        this->ui->comboBox->addItem(vals.at(0) + ": " + vals.at(1));
        i++;
    }
}

void SpeedyForm::on_pushButton_clicked()
{
    this->ui->pushButton->setEnabled(false);
}

void SpeedyForm::on_pushButton_2_clicked()
{
    this->close();
}

SpeedyForm::~SpeedyForm()
{
    delete this->ui;
}

void SpeedyForm::Init(WikiUser *user, WikiPage *page)
{
    this->User = user;
    this->ui->label_2->setText(page->PageName);
    this->Page = page;
}
