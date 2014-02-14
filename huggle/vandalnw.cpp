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
    this->Prefix = QString(QChar(001)) + QString(QChar(001));
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
        Huggle::Syslog::HuggleLogs->Log(Localizations::HuggleLocalizations->Localize("han-not"));
        return;
    } else
    {
        this->Insert(Localizations::HuggleLocalizations->Localize("han-connecting"));
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
    this->Irc->Send(this->GetChannel(), this->Prefix + "GOOD " + QString::number(Edit->RevID));
}

void VandalNw::Rollback(WikiEdit *Edit)
{
    if (Edit == NULL)
    {
        throw new Exception("WikiEdit *Edit was NULL", "void VandalNw::Rollback(WikiEdit *Edit)");
    }
    this->Irc->Send(this->GetChannel(), this->Prefix + "ROLLBACK " + QString::number(Edit->RevID));
}

void VandalNw::SuspiciousWikiEdit(WikiEdit *Edit)
{
    if (Edit == NULL)
    {
        throw new Exception("WikiEdit *Edit was NULL", "void VandalNw::Rollback(WikiEdit *Edit)");
    }
    this->Irc->Send(this->GetChannel(), this->Prefix + "SUSPICIOUS " + QString::number(Edit->RevID));
}

void VandalNw::WarningSent(WikiUser *user, int Level)
{
    if (user == NULL)
    {
        throw new Exception("WikiUser *user was NULL", "void VandalNw::WarningSent(WikiUser *user, int Level)");
    }
    this->Irc->Send(this->GetChannel(), this->Prefix + "WARN " + QString::number(Level) + " " + QUrl::toPercentEncoding(user->Username));
}

QString VandalNw::GetChannel()
{
    return Configuration::HuggleConfiguration->Project->IRCChannel + ".huggle";
}

bool VandalNw::IsParsed(WikiEdit *edit)
{
    int item = 0;
    while (item < this->UnparsedRoll.count())
    {
        if (this->UnparsedRoll.at(item).RevID == edit->RevID)
        {
            this->ProcessRollback(edit, this->UnparsedRoll.at(item).User);
            this->UnparsedRoll.removeAt(item);
            return true;
        }
        item++;
    }
    item = 0;
    while (item < this->UnparsedSusp.count())
    {
        if (this->UnparsedSusp.at(item).RevID == edit->RevID)
        {
            this->ProcessSusp(edit, this->UnparsedRoll.at(item).User);
            this->UnparsedSusp.removeAt(item);
            return true;
        }
        item++;
    }
    item = 0;
    while (item < this->UnparsedGood.count())
    {
        if (this->UnparsedGood.at(item).RevID == edit->RevID)
        {
            this->ProcessGood(edit, this->UnparsedRoll.at(item).User);
            this->UnparsedGood.removeAt(item);
            return true;
        }
        item++;
    }
    return false;
}

void VandalNw::Rescore(WikiEdit *edit)
{
    if (this->UnparsedScores.count() == 0)
    {
        return;
    }
    int item = 0;
    HAN::RescoreItem *score = NULL;
    while (item < this->UnparsedScores.count())
    {
        if (this->UnparsedScores.at(item).RevID == edit->RevID)
        {
            score = new HAN::RescoreItem(this->UnparsedScores.at(item));
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

void VandalNw::Message()
{
    if (this->Irc->IsConnected())
    {
        this->Irc->Send(this->GetChannel(), this->ui->lineEdit->text());
        this->Insert(Configuration::HuggleConfiguration->UserName + ": " + ui->lineEdit->text());
    }
    this->ui->lineEdit->setText("");
}

void VandalNw::ProcessGood(WikiEdit *edit, QString user)
{
    edit->User->SetBadnessScore(edit->User->GetBadnessScore() - 200);
    this->Insert("<font color=blue>" + user + " seen a good edit to " + edit->Page->PageName +
                 " by " + edit->User->Username + " (" + QString::number(edit->RevID) + ")" + "</font>");
    Core::HuggleCore->Main->Queue1->DeleteByRevID(edit->RevID);
}

void VandalNw::ProcessRollback(WikiEdit *edit, QString user)
{
    this->Insert("<font color=orange>" + user + " did a rollback of " + edit->Page->PageName + " by " +
           edit->User->Username + " (" + QString::number(edit->RevID) + ")" + "</font>");
    edit->User->SetBadnessScore(edit->User->GetBadnessScore() + 200);
    if (Huggle::Configuration::HuggleConfiguration->UserConfig_DeleteEditsAfterRevert)
    {
        // we need to delete older edits that we know and that is somewhere in queue
        if (Core::HuggleCore->Main != NULL)
        {
            if (Core::HuggleCore->Main->Queue1 != NULL)
            {
                Core::HuggleCore->Main->Queue1->DeleteOlder(edit);
            }
        }
    }
    Core::HuggleCore->Main->Queue1->DeleteByRevID(edit->RevID);
}

void VandalNw::ProcessSusp(WikiEdit *edit, QString user)
{
    this->Insert("<font color=red>" + user + " thinks that edit to " + edit->Page->PageName +
                 " by " + edit->User->Username + " (" + QString::number(edit->RevID) +
                 ") is likely a vandalism, but they didn't revert it </font>");
    edit->Score += 600;
    Core::HuggleCore->Main->Queue1->SortItemByEdit(edit);
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
        if (m->Text.startsWith(Prefix))
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
                        this->ProcessGood(edit, m->user.Nick);
                    } else
                    {
                        while (this->UnparsedGood.count() > Configuration::HuggleConfiguration->SystemConfig_CacheHAN)
                        {
                            this->UnparsedGood.removeAt(0);
                        }
                        this->UnparsedGood.append(HAN::GenericItem(RevID, m->user.Nick));
                    }
                }
                if (Command == "ROLLBACK")
                {
                    int RevID = revid.toInt();
                    WikiEdit *edit = Core::HuggleCore->Main->Queue1->GetWikiEditByRevID(RevID);
                    if (edit != NULL)
                    {
                        this->ProcessRollback(edit, m->user.Nick);
                    } else
                    {
                        while (this->UnparsedRoll.count() > Configuration::HuggleConfiguration->SystemConfig_CacheHAN)
                        {
                            this->UnparsedRoll.removeAt(0);
                        }
                        this->UnparsedRoll.append(HAN::GenericItem(RevID, m->user.Nick));
                    }
                }
                if (Command == "SUSPICIOUS")
                {
                    int RevID = revid.toInt();
                    WikiEdit *edit = Core::HuggleCore->Main->Queue1->GetWikiEditByRevID(RevID);
                    if (edit != NULL)
                    {
                        this->ProcessSusp(edit, m->user.Nick);
                    } else
                    {
                        while (this->UnparsedSusp.count() > Configuration::HuggleConfiguration->SystemConfig_CacheHAN)
                        {
                            this->UnparsedSusp.removeAt(0);
                        }
                        this->UnparsedSusp.append(HAN::GenericItem(RevID, m->user.Nick));
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
                            while (this->UnparsedScores.count() > Configuration::HuggleConfiguration->SystemConfig_CacheHAN)
                            {
                                this->UnparsedScores.removeAt(0);
                            }
                            this->UnparsedScores.append(HAN::RescoreItem(RevID, Score, m->user.Nick));
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
    this->Message();
}

HAN::RescoreItem::RescoreItem(int _revID, int _score, QString _user)
{
    this->RevID = _revID;
    this->User = _user;
    this->Score = _score;
}

HAN::RescoreItem::RescoreItem(const RescoreItem &item) : GenericItem(item)
{
    this->Score = item.Score;
}

HAN::RescoreItem::RescoreItem(RescoreItem *item) : GenericItem(item)
{
    this->Score = item->Score;
}

HAN::GenericItem::GenericItem()
{
    this->RevID = -1;
    this->User = "none";
}

HAN::GenericItem::GenericItem(int _revID, QString _user)
{
    this->RevID = _revID;
    this->User = _user;
}

HAN::GenericItem::GenericItem(const HAN::GenericItem &i)
{
    this->RevID = i.RevID;
    this->User = i.User;
}

HAN::GenericItem::GenericItem(HAN::GenericItem *i)
{
    this->RevID = i->RevID;
    this->User = i->User;
}

void Huggle::VandalNw::on_lineEdit_returnPressed()
{
    this->Message();
}
