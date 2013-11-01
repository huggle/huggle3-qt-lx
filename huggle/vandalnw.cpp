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
    this->pref = QString(QChar(001)) + QString(QChar(001));
    this->tm = new QTimer(this);
    this->JoinedMain = false;
    connect(tm, SIGNAL(timeout()), this, SLOT(onTick()));
    this->tm->start(200);
    this->Irc->UserName = Configuration::HuggleVersion;
}

VandalNw::~VandalNw()
{
    delete this->ui;
    delete this->tm;
    delete this->Irc;
}

void VandalNw::Connect()
{
    if (!Configuration::VandalNw_Login)
    {
        Core::Log("Vandalism network isn't allowed in options");
        return;
    } else
    {
        this->Insert("Connecting to huggle anti vandalism network");
        this->Irc->Connect();
    }
}

void VandalNw::Disconnect()
{
    this->Irc->Disconnect();
    this->Insert("You are disconnected from HAN");
}

void VandalNw::Good(WikiEdit *Edit)
{
    this->Irc->Send(this->GetChannel(), this->pref + "GOOD " + QString::number(Edit->RevID));
}

void VandalNw::Rollback(WikiEdit *Edit)
{
    this->Irc->Send(this->GetChannel(), this->pref + "ROLLBACK " + QString::number(Edit->RevID));
}

void VandalNw::SuspiciousWikiEdit(WikiEdit *Edit)
{
    this->Irc->Send(this->GetChannel(), this->pref + "SUSPICIOUS " + QString::number(Edit->RevID));
}

void VandalNw::WarningSent(WikiUser *user, int Level)
{
    this->Irc->Send(this->GetChannel(), this->pref + "WARN " + QString::number(Level) + " " + QUrl::toPercentEncoding(user->Username));
}

QString VandalNw::GetChannel()
{
    return Configuration::Project.IRCChannel + ".huggle";
}

void VandalNw::onTick()
{
    if (!this->Irc->IsConnected())
    {
        this->Insert("Lost connection to antivandalism network");
        this->tm->stop();
        return;
    }
    if (!this->JoinedMain && this->Irc->IsConnected())
    {
        this->JoinedMain = true;
        this->Insert("You are now connected to huggle antivandalism network");
        this->Irc->Join(this->GetChannel());
    }
    Huggle::IRC::Message *m = this->Irc->GetMessage();
    if (m != NULL)
    {
        if (m->Text.startsWith(pref))
        {
            QString Command = m->Text.mid(2);
            if (m->Text.contains(" "))
            {
                Command = Command.mid(0, Command.indexOf(" "));
                QString revid = m->Text.mid(m->Text.indexOf(" ") + 1);
                if (Command == "GOOD")
                {
                    int RevID = revid.toInt();
                    Core::Main->Queue1->DeleteByRevID(RevID);
                    this->Insert(m->user.Nick + " seen a good edit " + revid);
                }
                if (Command == "ROLLBACK")
                {
                    int RevID = revid.toInt();
                    Core::Main->Queue1->DeleteByRevID(RevID);
                    this->Insert(m->user.Nick + " did a rollback of " + revid);
                }
            }

        } else
        {
            this->Insert(m->user.Nick + ": " + m->Text);
        }
        delete m;
    }
}

void VandalNw::Insert(QString text)
{
    QString t = ui->textEdit->toPlainText();
    t.prepend(text + "\n");

    ui->textEdit->setPlainText(t);
}

void Huggle::VandalNw::on_pushButton_clicked()
{
    if (this->Irc->IsConnected())
    {
        this->Irc->Send(this->GetChannel(), this->ui->lineEdit->text());
        this->Insert(Configuration::UserName + ": " + ui->lineEdit->text());
    }
    ui->lineEdit->setText("");
}
