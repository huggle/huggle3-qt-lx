//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "whitelistform.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "ui_whitelistform.h"
#include "wikisite.hpp"

using namespace Huggle;

WhitelistForm::WhitelistForm(QWidget *parent) : QDialog(parent), ui(new Ui::WhitelistForm)
{
    this->ui->setupUi(this);
    this->timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(OnTick()));
    foreach (WikiSite *wiki, Configuration::HuggleConfiguration->Projects)
    {
        // register every project we use
        this->ui->comboBox->addItem(wiki->Name);
    }
    if (!Configuration::HuggleConfiguration->Multiple)
        this->ui->comboBox->setEnabled(false);
    this->ui->comboBox->setCurrentIndex(0);
    this->Reload(0);
}

WhitelistForm::~WhitelistForm()
{
    delete this->timer;
    delete this->ui;
}

void WhitelistForm::OnTick()
{
    if (this->Whitelist.count() == 0)
    {
        this->timer->stop();
    }
    int i = 0;
    while (this->Whitelist.count() > 0 && i < 200)
    {
        QString xx = this->Whitelist.at(0);
        xx = xx.replace(QRegExp("[^\\w\\s]"), "");
        this->ui->listWidget->addItem(xx);
        this->Whitelist.removeAt(0);
        i++;
    }
}

void WhitelistForm::on_pushButton_clicked()
{
    this->close();
}

void Huggle::WhitelistForm::on_comboBox_currentIndexChanged(int index)
{
    this->Reload(index);
}

void WhitelistForm::Reload(int pn)
{
    WikiSite *site;
    this->ui->listWidget->clear();
    this->Whitelist.clear();
    if (Configuration::HuggleConfiguration->Multiple)
    {
        if (Configuration::HuggleConfiguration->Projects.count() <= pn)
        {
            throw new Huggle::Exception("There is more projects in a list than in global list",
                                        "void WhitelistForm::Reload(int pn)");
        }
        site = Configuration::HuggleConfiguration->Projects.at(pn);
    } else
    {
        site = Configuration::HuggleConfiguration->Project;
    }
    site->GetProjectConfig()->WhiteList.sort();
    this->Whitelist += site->GetProjectConfig()->WhiteList;
    this->timer->start(HUGGLE_TIMER);
}
