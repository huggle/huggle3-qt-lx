//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "vandalnw.hpp"
#include "ui_vandalnw.h"

using namespace Huggle;

VandalNw::VandalNw(QWidget *parent) : QDockWidget(parent), ui(new Ui::VandalNw)
{
    this->Irc = new IRC::NetworkIrc(Configuration::HuggleConfiguration->VandalNw_Server, Configuration::HuggleConfiguration->UserName);
    this->ui->setupUi(this);
    this->pref = QString(QChar(001)) + QString(QChar(001));
    this->tm = new QTimer(this);
    this->Text = "";
    this->JoinedMain = false;
    connect(tm, SIGNAL(timeout()), this, SLOT(onTick()));
    this->tm->start(200);
    this->Irc->UserName = Configuration::HuggleConfiguration->HuggleVersion;
}

VandalNw::~VandalNw()
{
    delete this->ui;
    delete this->tm;
    delete this->Irc;
}

void VandalNw::Connect()
{
    if (Configuration::HuggleConfiguration->Restricted || !Configuration::HuggleConfiguration->VandalNw_Login)
    {
        /// \todo LOCALIZE ME
        Huggle::Syslog::HuggleLogs->Log("Vandalism network isn't allowed in options");
        return;
    } else
    {
        /// \todo LOCALIZE ME
        this->Insert("Connecting to huggle anti vandalism network");
        this->Irc->Connect();
    }
}

void VandalNw::Disconnect()
{
    this->Irc->Disconnect();
    /// \todo LOCALIZE ME
    this->Insert("You are disconnected from HAN");
}

void VandalNw::Good(WikiEdit *Edit)
{
    if (Edit == NULL)
    {
        throw new Exception("WikiEdit *Edit was NULL", "void VandalNw::Good(WikiEdit *Edit)");
    }
    this->Irc->Send(this->GetChannel(), this->pref + "GOOD " + QString::number(Edit->RevID));
}

void VandalNw::Rollback(WikiEdit *Edit)
{
    if (Edit == NULL)
    {
        throw new Exception("WikiEdit *Edit was NULL", "void VandalNw::Rollback(WikiEdit *Edit)");
    }
    this->Irc->Send(this->GetChannel(), this->pref + "ROLLBACK " + QString::number(Edit->RevID));
}

void VandalNw::SuspiciousWikiEdit(WikiEdit *Edit)
{
    if (Edit == NULL)
    {
        throw new Exception("WikiEdit *Edit was NULL", "void VandalNw::Rollback(WikiEdit *Edit)");
    }
    this->Irc->Send(this->GetChannel(), this->pref + "SUSPICIOUS " + QString::number(Edit->RevID));
}

void VandalNw::WarningSent(WikiUser *user, int Level)
{
    if (user == NULL)
    {
        throw new Exception("WikiUser *user was NULL", "void VandalNw::WarningSent(WikiUser *user, int Level)");
    }
    this->Irc->Send(this->GetChannel(), this->pref + "WARN " + QString::number(Level) + " " + QUrl::toPercentEncoding(user->Username));
}

QString VandalNw::GetChannel()
{
    return Configuration::HuggleConfiguration->Project->IRCChannel + ".huggle";
}

void VandalNw::Rescore(WikiEdit *edit)
{
    if (this->UnparsedScores.count() == 0)
    {
        return;
    }
    int item = 0;
    RescoreItem *score = NULL;
    while (item < this->UnparsedScores.count())
    {
        if (this->UnparsedScores.at(item).RevID == edit->RevID)
        {
            score = new RescoreItem(this->UnparsedScores.at(item));
            this->UnparsedScores.removeAt(item);
            break;
        }
        item++;
    }
    if (score != NULL)
    {
        this->Insert("<font color=green>" + score->User + " rescored edit <b>" + edit->Page->PageName + "</b> by <b>" +
                     edit->User->Username + "</b> (" + QString::number(score->RevID) + ") by " +
                     QString::number(score->Score) + "</font>");
        edit->Score += score->Score;
        delete score;
    }
}

void VandalNw::onTick()
{
    if (!this->Irc->IsConnecting() && !this->Irc->IsConnected())
    {
        /// \todo LOCALIZE ME
        this->Insert("Lost connection to antivandalism network");
        this->tm->stop();
        return;
    }
    if (!this->Irc->IsConnected())
    {
        // we are now connecting to irc so we need to wait
        return;
    }
    if (!this->JoinedMain && this->Irc->IsConnected())
    {
        this->JoinedMain = true;
        /// \todo LOCALIZE ME
        this->Insert("You are now connected to huggle antivandalism network");
        this->Irc->Join(this->GetChannel());
    }
    Huggle::IRC::Message *m = this->Irc->GetMessage();
    if (m != NULL)
    {
        if (m->Text.startsWith(pref))
        {
            QString Command = m->Text.mid(2);
            if (Command.contains(" "))
            {
                Command = Command.mid(0, Command.indexOf(" "));
                QString revid = m->Text.mid(m->Text.indexOf(" ") + 1);
                QString parameter = "";
                if (revid.contains(" "))
                {
                    parameter = revid.mid(revid.indexOf(" ") + 1);
                    revid = revid.mid(0, revid.indexOf(" "));
                }
                if (Command == "GOOD")
                {
                    int RevID = revid.toInt();
                    WikiEdit *edit = Core::HuggleCore->Main->Queue1->GetWikiEditByRevID(RevID);
                    if (edit != NULL)
                    {
                        edit->User->setBadnessScore(edit->User->getBadnessScore() - 200);
                        this->Insert("<font color=blue>" + m->user.Nick + " seen a good edit to " + edit->Page->PageName +
                                     " by " + edit->User->Username + " (" + revid + ")" + "</font>");
                        Core::HuggleCore->Main->Queue1->DeleteByRevID(RevID);
                    }
                }
                if (Command == "ROLLBACK")
                {
                    int RevID = revid.toInt();
                    WikiEdit *edit = Core::HuggleCore->Main->Queue1->GetWikiEditByRevID(RevID);
                    if (edit != NULL)
                    {
                        this->Insert("<font color=orange>" + m->user.Nick + " did a rollback of " + edit->Page->PageName + " by " + edit->User->Username + " (" + revid + ")" + "</font>");
                        edit->User->setBadnessScore(edit->User->getBadnessScore() + 200);
                        Core::HuggleCore->Main->Queue1->DeleteByRevID(RevID);
                    }
                }
                if (Command == "SUSPICIOUS")
                {
                    int RevID = revid.toInt();
                    WikiEdit *edit = Core::HuggleCore->Main->Queue1->GetWikiEditByRevID(RevID);
                    if (edit != NULL)
                    {
                        this->Insert("<font color=red>" + m->user.Nick + " thinks that edit to " + edit->Page->PageName +
                                     " by " + edit->User->Username + " (" + revid +
                                     ") is likely a vandalism, but they didn't revert it </font>");
                        edit->Score += 600;
                        Core::HuggleCore->Main->Queue1->SortItemByEdit(edit);
                    }
                }
                if (Command == "SCORED")
                {
                    int RevID = revid.toInt();
                    int Score = parameter.toInt();
                    if (Score != 0)
                    {
                        WikiEdit *edit = Core::HuggleCore->Main->Queue1->GetWikiEditByRevID(RevID);
                        if (edit != NULL)
                        {
                            this->Insert("<font color=green>" + m->user.Nick + " rescored edit <b>" +
                                  edit->Page->PageName + "</b> by <b>" + edit->User->Username + "</b> (" + revid + ") by " +
                                         QString::number(Score) + "</font>");
                            edit->Score += Score;
                            Core::HuggleCore->Main->Queue1->SortItemByEdit(edit);
                        } else
                        {
                            while (this->UnparsedScores.count() > 200)
                            {
                                this->UnparsedScores.removeAt(0);
                            }
                            this->UnparsedScores.append(RescoreItem(RevID, Score, m->user.Nick));
                        }
                    }
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
    this->Text.prepend(text + "<br>");
    this->ui->textEdit->setHtml(this->Text);
}

void Huggle::VandalNw::on_pushButton_clicked()
{
    if (this->Irc->IsConnected())
    {
        this->Irc->Send(this->GetChannel(), this->ui->lineEdit->text());
        this->Insert(Configuration::HuggleConfiguration->UserName + ": " + ui->lineEdit->text());
    }
    this->ui->lineEdit->setText("");
}

RescoreItem::RescoreItem(int _revID, int _score, QString _user)
{
    this->RevID = _revID;
    this->User = _user;
    this->Score = _score;
}

RescoreItem::RescoreItem(const RescoreItem &item)
{
    this->User = item.User;
    this->Score = item.Score;
    this->RevID = item.RevID;
}

RescoreItem::RescoreItem(RescoreItem *item)
{
    this->User = item->User;
    this->RevID = item->RevID;
    this->Score = item->Score;
}
