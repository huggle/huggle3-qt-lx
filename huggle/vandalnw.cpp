//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "vandalnw.h"
#include "ui_vandalnw.h"

using namespace Huggle;

VandalNw::VandalNw(QWidget *parent) : QDockWidget(parent), ui(new Ui::VandalNw)
{
    this->Irc = new IRC::NetworkIrc(Configuration::VandalNw_Server, Configuration::UserName);
    this->ui->setupUi(this);
    this->tm = new QTimer(this);
    this->JoinedMain = false;
    connect(tm, SIGNAL(timeout()), this, SLOT(onTick()));
    this->tm->start(200);
}

VandalNw::~VandalNw()
{
    delete this->ui;
    delete this->tm;
    delete this->Irc;
}

void VandalNw::Connect()
{
    this->Irc->Connect();
}

void VandalNw::onTick()
{
    if (!this->JoinedMain && this->Irc->IsConnected())
    {
        this->JoinedMain = true;
        this->Insert("You are now connected to huggle antivandalism network");
        this->Irc->Join(Configuration::Project.IRCChannel + ".huggle");
    }
}

void VandalNw::Insert(QString text)
{
    QString t = ui->textEdit->toPlainText();
    t.prepend(text + "\n");

    ui->textEdit->setPlainText(t);
}
