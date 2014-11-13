//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "vandalnw.hpp"
#include "core.hpp"
#include "configuration.hpp"
#include "localization.hpp"
#include "huggleweb.hpp"
#include "hugglequeue.hpp"
#include "mainwindow.hpp"
#include "networkirc.hpp"
#include "syslog.hpp"
#include "ui_vandalnw.h"
#include "wikiedit.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"

using namespace Huggle;

QString VandalNw::SafeHtml(QString text)
{
    QString result;
    QStringList AllowedTags;
    AllowedTags << "b" << "big" << "font" << "i" << "s" << "u";
    // we split the text by starting element
    QStringList Part = text.split('<');
    bool FirstPart = true;
    foreach (QString part, Part)
    {
        if (FirstPart)
            FirstPart = false;
        else
            part = "<" + part;
        // if there is no closing we don't want to render this tag
        if (!part.contains(">"))
        {
            result += HuggleWeb::Encode(part);
            continue;
        }
        // now we need to find what tag it is
        QString tag = part.mid(1, part.indexOf(">") - 1);
        if (tag.startsWith("/"))
            tag = tag.mid(1);
        if (tag.contains(" "))
        {
            // this is some composite tag
            tag = tag.mid(0, tag.indexOf(" "));
        }
        if (!AllowedTags.contains(tag))
            part = HuggleWeb::Encode(part);
        result += part;
    }

    return result;
}

VandalNw::VandalNw(QWidget *parent) : QDockWidget(parent), ui(new Ui::VandalNw)
{
    this->Irc = new IRC::NetworkIrc(Configuration::HuggleConfiguration->VandalNw_Server,
                                    Configuration::HuggleConfiguration->SystemConfig_Username);
    this->ui->setupUi(this);
    this->Prefix = QString(QChar(001)) + QString(QChar(001));
    this->tm = new QTimer(this);
    this->Text = "";
    this->JoinedMain = false;
    this->GetChannel();
    this->UsersModified = false;
    connect(tm, SIGNAL(timeout()), this, SLOT(onTick()));
    this->Irc->UserName = Configuration::HuggleConfiguration->HuggleVersion;
    this->ui->tableWidget->setColumnCount(1);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui->tableWidget->setShowGrid(false);
}

VandalNw::~VandalNw()
{
    delete this->ui;
    delete this->tm;
    delete this->Irc;
}

void VandalNw::Connect()
{
    if (this->Irc->IsConnected())
    {
        Syslog::HuggleLogs->Log("Not connecting to HAN because it's already connected");
        return;
    }
    if (this->Irc->IsConnecting())
    {
        Syslog::HuggleLogs->Log("Please wait, I am already connecting to HAN");
        return;
    }
    if (Configuration::HuggleConfiguration->Restricted || !Configuration::HuggleConfiguration->VandalNw_Login)
    {
        Huggle::Syslog::HuggleLogs->Log(_l("han-not"));
        return;
    } else
    {
        this->Insert(_l("han-connecting"), HAN::MessageType_Info);
        this->Irc->Connect();
        this->tm->start(HUGGLE_TIMER);
    }
    this->UsersModified = true;
}

void VandalNw::Disconnect()
{
    this->Irc->Disconnect();
    this->JoinedMain = false;
    /// \todo LOCALIZE ME
    this->Insert("You are disconnected from HAN", HAN::MessageType_Info);
}

void VandalNw::Good(WikiEdit *Edit)
{
    if (Edit == nullptr)
    {
        throw new Exception("WikiEdit *Edit was NULL", BOOST_CURRENT_FUNCTION);
    }
    if (!this->Site2Channel.contains(Edit->GetSite()))
    {
        throw new Exception("There is no channel for this site", BOOST_CURRENT_FUNCTION);
    }
    this->Irc->Send(this->Site2Channel[Edit->GetSite()], this->Prefix + "GOOD " + QString::number(Edit->RevID));
}

void VandalNw::Rollback(WikiEdit *Edit)
{
    if (Edit == nullptr)
    {
        throw new Exception("WikiEdit *Edit was NULL", BOOST_CURRENT_FUNCTION);
    }
    if (!this->Site2Channel.contains(Edit->GetSite()))
    {
        throw new Exception("There is no channel for this site", BOOST_CURRENT_FUNCTION);
    }
    this->Irc->Send(this->Site2Channel[Edit->GetSite()], this->Prefix + "ROLLBACK " + QString::number(Edit->RevID));
}

void VandalNw::SuspiciousWikiEdit(WikiEdit *Edit)
{
    if (Edit == nullptr)
    {
        throw new Exception("WikiEdit *Edit was NULL", BOOST_CURRENT_FUNCTION);
    }
    if (!this->Site2Channel.contains(Edit->GetSite()))
    {
        throw new Exception("There is no channel for this site", BOOST_CURRENT_FUNCTION);
    }
    this->Irc->Send(this->Site2Channel[Edit->GetSite()], this->Prefix + "SUSPICIOUS " + QString::number(Edit->RevID));
}

void VandalNw::WarningSent(WikiUser *user, byte_ht Level)
{
    if (user == nullptr)
    {
        throw new Exception("WikiUser *user was NULL", BOOST_CURRENT_FUNCTION);
    }
    if (!this->Site2Channel.contains(user->GetSite()))
    {
        throw new Exception("There is no channel for this site", BOOST_CURRENT_FUNCTION);
    }
    this->Irc->Send(this->Site2Channel[user->GetSite()], this->Prefix + "WARN " + QString::number(Level)
                    + " " + QUrl::toPercentEncoding(user->Username));
}

void VandalNw::GetChannel()
{
    this->Site2Channel.clear();
    foreach (WikiSite *ch, Configuration::HuggleConfiguration->Projects)
    {
        QString name;
        if (!ch->HANChannel.isEmpty())
        {
            name = ch->HANChannel;
        } else
        {
            name = Configuration::HuggleConfiguration->HANMask;
            name.replace("$feed", ch->IRCChannel);
            name.replace("$wiki", ch->Name);
        }
        // now we need to insert both site and channel to tables
        this->Ch2Site.insert(name, ch);
        this->Site2Channel.insert(ch, name);
    }
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
    HAN::RescoreItem *score = nullptr;
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
    if (score != nullptr)
    {
        bool bot_ = score->User.toLower().contains("bot");
        QString message = "<font color=green>" + score->User + " rescored edit <b>" + edit->Page->PageName + "</b> by <b>" +
                          edit->User->Username + "</b> (" + QString::number(score->RevID) + ") by " +
                          QString::number(score->Score) + "</font>";
        if (bot_)
        {
            this->Insert(message, HAN::MessageType_Bot);
        } else
        {
            this->Insert(message, HAN::MessageType_User);
        }
        edit->Score += score->Score;
        delete score;
    }
}

void VandalNw::Message()
{
    if (this->Irc->IsConnected())
    {
        this->Irc->Send(this->Site2Channel[Configuration::HuggleConfiguration->Project], this->ui->lineEdit->text());
        QString text = ui->lineEdit->text();
        if (!hcfg->UserConfig->HtmlAllowedInIrc)
            text = SafeHtml(text);
        this->Insert(Configuration::HuggleConfiguration->SystemConfig_Username + ": " + text,
                     HAN::MessageType_UserTalk);
    }
    this->ui->lineEdit->setText("");
}

void VandalNw::ProcessGood(WikiEdit *edit, QString user)
{
    edit->User->SetBadnessScore(edit->User->GetBadnessScore() - 200);
    this->Insert("<font color=blue>" + user + " saw a good edit on " + edit->GetSite()->Name + " to " + edit->Page->PageName + " by " + edit->User->Username
                     + " (" + QString::number(edit->RevID) + ")" + "</font>", HAN::MessageType_User);
    Core::HuggleCore->Main->Queue1->DeleteByRevID(edit->RevID, edit->GetSite());
}

void VandalNw::ProcessRollback(WikiEdit *edit, QString user)
{
    this->Insert("<font color=orange>" + user + " did a rollback on " + edit->GetSite()->Name + " of " + edit->Page->PageName + " by " + edit->User->Username
                 + " (" + QString::number(edit->RevID) + ")" + "</font>", HAN::MessageType_User);
    edit->User->SetBadnessScore(edit->User->GetBadnessScore() + 200);
    if (Huggle::Configuration::HuggleConfiguration->UserConfig->DeleteEditsAfterRevert)
    {
        // we need to delete older edits that we know and that is somewhere in queue
        if (Core::HuggleCore->Main != nullptr)
        {
            if (Core::HuggleCore->Main->Queue1 != nullptr)
            {
                Core::HuggleCore->Main->Queue1->DeleteOlder(edit);
            }
        }
    }
    Core::HuggleCore->Main->Queue1->DeleteByRevID(edit->RevID, edit->GetSite());
}

void VandalNw::ProcessSusp(WikiEdit *edit, QString user)
{
    this->Insert("<font color=red>" + user + " thinks that edit on " + edit->GetSite()->Name + " to " + edit->Page->PageName + " by "
                 + edit->User->Username + " (" + QString::number(edit->RevID) +
                 ") is likely a vandalism, but they didn't revert it </font>", HAN::MessageType_User);
    edit->Score += 600;
    Core::HuggleCore->Main->Queue1->SortItemByEdit(edit);
}

void VandalNw::UpdateHeader()
{
    if (!this->UsersModified)
    {
        return;
    }
    if (!this->Irc->IsConnected())
    {
        this->setWindowTitle("Network");
        this->UsersModified = false;
    } else
    {
        QList<IRC::User> users;
        this->Irc->ChannelsLock->lock();
        if (this->Irc->Channels.contains(this->Site2Channel[Configuration::HuggleConfiguration->Project]))
        {
            Huggle::IRC::Channel *channel_ = this->Irc->Channels[this->Site2Channel[Configuration::HuggleConfiguration->Project]];
            if (channel_->UsersChanged())
            {
                this->setWindowTitle(QString("Network (" + QString::number(channel_->Users.count()) + ")"));
                // users
                users = channel_->Users.values();
            }
        }
        this->Irc->ChannelsLock->unlock();
        if (users.count() > 0)
        {
            // remove all items from list
            while (this->ui->tableWidget->rowCount() > 0)
            {
                this->ui->tableWidget->removeRow(0);
            }
            while (users.count() > 0)
            {
                this->ui->tableWidget->insertRow(0);
                this->ui->tableWidget->setItem(0, 0, new QTableWidgetItem(users.at(0).Nick));
                users.removeAt(0);
            }
            this->ui->tableWidget->resizeRowsToContents();
        }
    }
}

void VandalNw::onTick()
{
    if (!this->Irc->IsConnecting() && !this->Irc->IsConnected())
    {
        /// \todo LOCALIZE ME
        this->Insert("Lost connection to antivandalism network", HAN::MessageType_Info);
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
        this->UsersModified = true;
        this->JoinedMain = true;
        /// \todo LOCALIZE ME
        this->Insert("You are now connected to huggle antivandalism network", HAN::MessageType_Info);
        foreach (QString channel, this->Site2Channel.values())
            this->Irc->Join(channel);
    }
    Huggle::IRC::Message *m = this->Irc->GetMessage();
    if (m != nullptr)
    {
        if (!this->Ch2Site.contains(m->Channel))
        {
            Syslog::HuggleLogs->DebugLog("Ignoring message to channel " + m->Channel + " as we don't know which site it belongs to");
            return;
        }
        WikiSite *site = this->Ch2Site[m->Channel];
        HAN::MessageType mt;
        if (!m->user.Nick.toLower().contains("bot"))
        {
            mt = HAN::MessageType_User;
        } else
        {
            mt = HAN::MessageType_Bot;
        }
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
                    WikiEdit *edit = Core::HuggleCore->Main->Queue1->GetWikiEditByRevID(RevID, site);
                    if (edit != nullptr)
                    {
                        this->ProcessGood(edit, m->user.Nick);
                    } else
                    {
                        while (this->UnparsedGood.count() > Configuration::HuggleConfiguration->SystemConfig_CacheHAN)
                        {
                            this->UnparsedGood.removeAt(0);
                        }
                        this->UnparsedGood.append(HAN::GenericItem(site, RevID, m->user.Nick));
                    }
                }
                if (Command == "ROLLBACK")
                {
                    int RevID = revid.toInt();
                    WikiEdit *edit = Core::HuggleCore->Main->Queue1->GetWikiEditByRevID(RevID, site);
                    if (edit != nullptr)
                    {
                        this->ProcessRollback(edit, m->user.Nick);
                    } else
                    {
                        while (this->UnparsedRoll.count() > Configuration::HuggleConfiguration->SystemConfig_CacheHAN)
                        {
                            this->UnparsedRoll.removeAt(0);
                        }
                        this->UnparsedRoll.append(HAN::GenericItem(site, RevID, m->user.Nick));
                    }
                }
                if (Command == "SUSPICIOUS")
                {
                    int RevID = revid.toInt();
                    WikiEdit *edit = Core::HuggleCore->Main->Queue1->GetWikiEditByRevID(RevID, site);
                    if (edit != nullptr)
                    {
                        this->ProcessSusp(edit, m->user.Nick);
                    } else
                    {
                        while (this->UnparsedSusp.count() > Configuration::HuggleConfiguration->SystemConfig_CacheHAN)
                        {
                            this->UnparsedSusp.removeAt(0);
                        }
                        this->UnparsedSusp.append(HAN::GenericItem(site, RevID, m->user.Nick));
                    }
                }
                if (Command == "SCORED")
                {
                    int RevID = revid.toInt();
                    int Score = parameter.toInt();
                    if (Score != 0)
                    {
                        WikiEdit *edit = Core::HuggleCore->Main->Queue1->GetWikiEditByRevID(RevID, site);
                        if (edit != nullptr)
                        {
                            this->Insert("<font color=green>" + m->user.Nick + " rescored edit <b>" +
                                         edit->Page->PageName + "</b> by <b>" + edit->User->Username +
                                         "</b> (" + revid + ") by " + QString::number(Score) + "</font>", mt);
                            edit->Score += Score;
                            Core::HuggleCore->Main->Queue1->SortItemByEdit(edit);
                        } else
                        {
                            while (this->UnparsedScores.count() > Configuration::HuggleConfiguration->SystemConfig_CacheHAN)
                            {
                                this->UnparsedScores.removeAt(0);
                            }
                            this->UnparsedScores.append(HAN::RescoreItem(site, RevID, Score, m->user.Nick));
                        }
                    }
                }
            }
        } else
        {
            QString message_ = m->Text;
            if (!hcfg->UserConfig->HtmlAllowedInIrc)
                message_ = SafeHtml(message_);
            if (hcfg->Multiple)
            {
                this->Insert(m->user.Nick + " (" + site->Name + "): " + message_, HAN::MessageType_UserTalk);
            } else
            {
                this->Insert(m->user.Nick + ": " + message_, HAN::MessageType_UserTalk);
            }
        }
        delete m;
    }
    this->UpdateHeader();
}

void VandalNw::Insert(QString text, HAN::MessageType type)
{
    if ((type == HAN::MessageType_Bot && !Configuration::HuggleConfiguration->UserConfig->HAN_DisplayBots)      ||
        (type == HAN::MessageType_User && !Configuration::HuggleConfiguration->UserConfig->HAN_DisplayUser)     ||
        (type == HAN::MessageType_UserTalk && !Configuration::HuggleConfiguration->UserConfig->HAN_DisplayUserTalk))
          return;
    if (type == HAN::MessageType_Info)
    {
        this->Text.prepend("<font color=gray>" + text + "</font><br>");
    } else
    {
        this->Text.prepend(text + "<br>");
    }
    this->ui->textEdit->setHtml(this->Text);
}

void Huggle::VandalNw::on_pushButton_clicked()
{
    this->Message();
}

HAN::RescoreItem::RescoreItem(WikiSite *site, int _revID, int _score, QString _user) : GenericItem(site, _revID, _user)
{
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

HAN::GenericItem::GenericItem(WikiSite *site)
{
    this->Site = site;
    this->User = "none";
    this->RevID = WIKI_UNKNOWN_REVID;
}

HAN::GenericItem::GenericItem(WikiSite *site, int _revID, QString _user)
{
    this->Site = site;
    this->RevID = _revID;
    this->User = _user;
}

HAN::GenericItem::GenericItem(const HAN::GenericItem &i)
{
    this->Site = i.Site;
    this->RevID = i.RevID;
    this->User = i.User;
}

HAN::GenericItem::GenericItem(HAN::GenericItem *i)
{
    this->Site = i->Site;
    this->RevID = i->RevID;
    this->User = i->User;
}

void Huggle::VandalNw::on_lineEdit_returnPressed()
{
    this->Message();
}
