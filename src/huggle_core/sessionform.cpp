//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "sessionform.hpp"
#include "configuration.hpp"
#include "core.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "hugglefeed.hpp"
#include "ui_sessionform.h"
#include "wikisite.hpp"

using namespace Huggle;
SessionForm::SessionForm(QWidget *parent) : HW("sessionform", this, parent), ui(new Ui::SessionForm)
{
    this->ui->setupUi(this);
    foreach (WikiSite *site, Configuration::HuggleConfiguration->Projects)
        this->ui->comboBox->addItem(site->Name);
    this->ui->comboBox->setEnabled(Configuration::HuggleConfiguration->SystemConfig_Multiple);
    this->ui->comboBox->setCurrentIndex(0);
    this->Reload(0);
    this->RestoreWindow();
}

SessionForm::~SessionForm()
{
    delete this->ui;
}

void Huggle::SessionForm::on_pushButton_clicked()
{
    this->close();
}

void Huggle::SessionForm::on_comboBox_currentIndexChanged(int index)
{
    this->Reload(index);
}

void SessionForm::Reload(int x)
{
    /// \todo TRANSLATE ME
    WikiSite *site = Configuration::HuggleConfiguration->Projects.at(x);
    this->ui->label_2->setText("You are logged in as " + Configuration::HuggleConfiguration->SystemConfig_Username + "\n" +
                               "SSL: " + Generic::Bool2String(Configuration::HuggleConfiguration->SystemConfig_UsingSSL) + "\n" +
                               "RC feed: " + site->Provider->ToString() + "\n" +
                               "MediaWiki: " + site->MediawikiVersion.ToString());
    this->ui->listWidget->clear();
    foreach (QString p, site->GetProjectConfig()->Rights)
        this->ui->listWidget->addItem(p);
}
