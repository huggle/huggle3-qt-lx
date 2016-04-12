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
#include "exception.hpp"
#include "generic.hpp"
#include "localization.hpp"
#include "huggleweb.hpp"
#include "hugglequeue.hpp"
#include "mainwindow.hpp"
#include "syslog.hpp"
#include "ui_vandalnw.h"
#include "ircchattextbox.hpp"
#include "wikiedit.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"
#include <QUrl>
#include "libs/libirc/libircclient/network.h"
#include "libs/libirc/libircclient/parser.h"
#include "libs/libirc/libircclient/channel.h"
#include "libs/libirc/libirc/serveraddress.h"

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

QString VandalNw::GenerateWikiDiffLink(QString text, QString revid, WikiSite *site)
{
    return "<a href=\"huggle:///diff/" + site->Name + "/revid/" + revid + "\">" + text + "</a>";
}

VandalNw::VandalNw(QWidget *parent) : QDockWidget(parent), ui(new Ui::VandalNw)
{
    // Construct a server address from what we have
    libirc::ServerAddress server(hcfg->VandalNw_Server, false, 6667, hcfg->SystemConfig_Username);
    this->Irc = new libircclient::Network(server, "HAN");
    this->ui->setupUi(this);
    // such hack. much WOW :P
    this->ui->horizontalLayout_2->removeWidget(this->ui->plainTextEdit);
    delete this->ui->plainTextEdit;
    this->ui->plainTextEdit = new Huggle::IRCChatTextBox(this);
    this->ui->horizontalLayout_2->insertWidget(0, this->ui->plainTextEdit);
    /*
                ▄               ▄
                ▌▒█           ▄▀▒▌
                ▌▒▒▀▄       ▄▀▒▒▒▐
               ▐▄▀▒▒▀▀▀▀▄▄▄▀▒▒▒▒▒▐
             ▄▄▀▒▒▒▒▒▒▒▒▒▒▒█▒▒▄█▒▐
           ▄▀▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▀██▀▒▌
          ▐▒▒▒▄▄▄▒▒▒▒▒▒▒▒▒▒▒▒▒▀▄▒▒▌
          ▌▒▒▐▄█▀▒▒▒▒▄▀█▄▒▒▒▒▒▒▒█▒▐
         ▐▒▒▒▒▒▒▒▒▒▒▒▌██▀▒▒▒▒▒▒▒▒▀▄▌
         ▌▒▀▄██▄▒▒▒▒▒▒▒▒▒▒▒░░░░▒▒▒▒▌
         ▌▀▐▄█▄█▌▄▒▀▒▒▒▒▒▒░░░░░░▒▒▒▐
        ▐▒▀▐▀▐▀▒▒▄▄▒▄▒▒▒▒▒░░░░░░▒▒▒▒▌
        ▐▒▒▒▀▀▄▄▒▒▒▄▒▒▒▒▒▒░░░░░░▒▒▒▐
         ▌▒▒▒▒▒▒▀▀▀▒▒▒▒▒▒▒▒░░░░▒▒▒▒▌
         ▐▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▐
          ▀▄▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▄▒▒▒▒▌
            ▀▄▒▒▒▒▒▒▒▒▒▒▄▄▄▀▒▒▒▒▄▀
           ▐▀▒▀▄▄▄▄▄▄▀▀▀▒▒▒▒▒▄▄▀
          ▐▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▀▀

     */
    this->Prefix = QString(QChar(001)) + QString(QChar(001));
    this->JoinedMain = false;
    this->GetChannel();
    this->UsersModified = false;
    connect(((IRCChatTextBox*)this->ui->plainTextEdit), SIGNAL(Event_Link(QString)), this, SLOT(TextEdit_anchorClicked(QString)));
    connect(this->Irc, SIGNAL(Event_Connected()), this, SLOT(OnConnected()));
    connect(this->Irc, SIGNAL(Event_SelfJoin(libircclient::Channel*)), this, SLOT(OnIRCSelfJoin(libircclient::Channel*)));
    connect(this->Irc, SIGNAL(Event_SelfPart(libircclient::Parser*,libircclient::Channel*)), this, SLOT(OnIRCSelfPart(libircclient::Parser*,libircclient::Channel*)));
    connect(this->Irc, SIGNAL(Event_PRIVMSG(libircclient::Parser*)), this, SLOT(OnIRCChannelMessage(libircclient::Parser*)));
    connect(this->Irc, SIGNAL(Event_PerChannelQuit(libircclient::Parser*,libircclient::Channel*)), this, SLOT(OnIRCChannelQuit(libircclient::Parser*,libircclient::Channel*)));
    connect(this->Irc, SIGNAL(Event_Disconnected()), this, SLOT(OnDisconnected()));
    connect(this->Irc, SIGNAL(Event_Join(libircclient::Parser*,libircclient::User*,libircclient::Channel*)), this, SLOT(OnIRCUserJoin(libircclient::Parser*,libircclient::User*,libircclient::Channel*)));
    connect(this->Irc, SIGNAL(Event_MyInfo(libircclient::Parser*)), this, SLOT(OnIRCLoggedIn(libircclient::Parser*)));
    connect(this->Irc, SIGNAL(Event_Part(libircclient::Parser*,libircclient::Channel*)), this, SLOT(OnIRCUserPart(libircclient::Parser*,libircclient::Channel*)));
    connect(this->Irc, SIGNAL(Event_EndOfNames(libircclient::Parser*)), this, SLOT(OnIRCChannelNames(libircclient::Parser*)));
    this->Irc->SetDefaultUsername(Configuration::HuggleConfiguration->HuggleVersion);
    this->Irc->SetDefaultIdent("huggle");
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
    delete this->Irc;
}

void VandalNw::Connect()
{
    if (this->Irc->IsConnected())
    {
        Syslog::HuggleLogs->Log(_l("han-already-connected"));
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
    }
    this->UsersModified = true;
}

void VandalNw::Disconnect()
{
    this->Irc->Disconnect(Generic::IRCQuitDefaultMessage());
    this->JoinedMain = false;
    this->Insert(_l("han-disconnected"), HAN::MessageType_Info);
}

void VandalNw::Good(WikiEdit *Edit)
{
    if (Edit == nullptr)
    {
        throw new NullPointerException("WikiEdit *Edit", BOOST_CURRENT_FUNCTION);
    }
    if (!this->Site2Channel.contains(Edit->GetSite()))
    {
        throw new Exception("There is no channel for this site", BOOST_CURRENT_FUNCTION);
    }
    this->Irc->SendMessage(this->Prefix + "GOOD " + QString::number(Edit->RevID), this->Site2Channel[Edit->GetSite()]);
}

void VandalNw::Rollback(WikiEdit *Edit)
{
    if (Edit == nullptr)
    {
        throw new NullPointerException("WikiEdit *Edit", BOOST_CURRENT_FUNCTION);
    }
    if (!this->Site2Channel.contains(Edit->GetSite()))
    {
        throw new Exception("There is no channel for this site", BOOST_CURRENT_FUNCTION);
    }
    this->Irc->SendMessage(this->Prefix + "ROLLBACK " + QString::number(Edit->RevID), this->Site2Channel[Edit->GetSite()]);
}

void VandalNw::SuspiciousWikiEdit(WikiEdit *Edit)
{
    if (Edit == nullptr)
    {
        throw new NullPointerException("WikiEdit *Edit", BOOST_CURRENT_FUNCTION);
    }
    if (!this->Site2Channel.contains(Edit->GetSite()))
    {
        throw new Exception("There is no channel for this site", BOOST_CURRENT_FUNCTION);
    }
    this->Irc->SendMessage(this->Prefix + "SUSPICIOUS " + QString::number(Edit->RevID), this->Site2Channel[Edit->GetSite()]);
}

void VandalNw::WarningSent(WikiUser *user, byte_ht Level)
{
    if (user == nullptr)
    {
        throw new NullPointerException("WikiUser *user", BOOST_CURRENT_FUNCTION);
    }
    if (!this->Site2Channel.contains(user->GetSite()))
    {
        throw new Exception("There is no channel for this site", BOOST_CURRENT_FUNCTION);
    }
    this->Irc->SendMessage(this->Prefix + "WARN " + QString::number(Level) + " " + QUrl::toPercentEncoding(user->Username), this->Site2Channel[user->GetSite()]);
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
            this->ProcessSusp(edit, this->UnparsedSusp.at(item).User);
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
            this->ProcessGood(edit, this->UnparsedGood.at(item).User);
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
        QString sid = QString::number(score->RevID);
        bool bot_ = score->User.toLower().contains("bot");
        QString message = "<font color=green>" + score->User + " rescored edit <b>" + edit->Page->PageName + "</b> by <b>" +
                          edit->User->Username + "</b> (" + GenerateWikiDiffLink(sid, sid, edit->GetSite()) + ") by " +
                          QString::number(score->Score) + "</font>";
        if (bot_)
        {
            this->Insert(message, HAN::MessageType_Bot);
        } else
        {
            this->Insert(message, HAN::MessageType_User);
        }
        edit->Score += score->Score;
        if (!edit->MetaLabels.contains("Bot score"))
            edit->MetaLabels.insert("Bot score", QString::number(score->Score));
        delete score;
    }
}

void VandalNw::Message()
{
    if (this->Irc->IsConnected())
    {
        this->Irc->SendMessage(this->ui->lineEdit->text(), this->Site2Channel[Configuration::HuggleConfiguration->Project]);
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
    QString sid = QString::number(edit->RevID);
    this->Insert("<font color=blue>" + user + " saw a good edit on " + edit->GetSite()->Name + " to " + edit->Page->PageName + " by " + edit->User->Username
                     + " (" + GenerateWikiDiffLink(sid, sid, edit->GetSite()) + ")" + "</font>", HAN::MessageType_User);
    MainWindow::HuggleMain->Queue1->DeleteByRevID(edit->RevID, edit->GetSite());
}

void VandalNw::ProcessRollback(WikiEdit *edit, QString user)
{
    QString sid = QString::number(edit->RevID);
    this->Insert("<font color=orange>" + user + " did a rollback on " + edit->GetSite()->Name + " of " + edit->Page->PageName + " by " + edit->User->Username
                 + " (" + GenerateWikiDiffLink(sid, sid, edit->GetSite()) + ")" + "</font>", HAN::MessageType_User);
    edit->User->SetBadnessScore(edit->User->GetBadnessScore() + 200);
    if (Huggle::Configuration::HuggleConfiguration->UserConfig->DeleteEditsAfterRevert)
    {
        // we need to delete older edits that we know and that is somewhere in queue
        if (MainWindow::HuggleMain != nullptr && MainWindow::HuggleMain->Queue1 != nullptr)
                MainWindow::HuggleMain->Queue1->DeleteOlder(edit);
    }
    if (MainWindow::HuggleMain != nullptr && MainWindow::HuggleMain->Queue1 != nullptr)
        MainWindow::HuggleMain->Queue1->DeleteByRevID(edit->RevID, edit->GetSite());
}

void VandalNw::ProcessSusp(WikiEdit *edit, QString user)
{
    QString sid = QString::number(edit->RevID);
    this->Insert("<font color=red>" + user + " thinks that edit on " + edit->GetSite()->Name + " to " + edit->Page->PageName + " by "
                 + edit->User->Username + " (" + GenerateWikiDiffLink(sid, sid, edit->GetSite()) +
                 ") is likely a vandalism, but they didn't revert it </font>", HAN::MessageType_User);
    edit->Score += 600;
    MainWindow::HuggleMain->Queue1->SortItemByEdit(edit);
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
        QList<libircclient::User*> users;
        libircclient::Channel *channel_ = this->Irc->GetChannel(this->Site2Channel[Configuration::HuggleConfiguration->Project]);
        if (channel_ != nullptr)
        {
            this->setWindowTitle(QString("Network - " + this->Irc->GetNick() + " (" + QString::number(channel_->GetUserCount()) + ")"));
            users = channel_->GetUsers().values();
        }
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
                this->ui->tableWidget->setItem(0, 0, new QTableWidgetItem(users.at(0)->GetNick()));
                users.removeAt(0);
            }
            this->ui->tableWidget->resizeRowsToContents();
        }
    }
}

void VandalNw::Insert(QString text, HAN::MessageType type)
{
    if ((type == HAN::MessageType_Bot && !Configuration::HuggleConfiguration->UserConfig->HAN_DisplayBots)      ||
        (type == HAN::MessageType_User && !Configuration::HuggleConfiguration->UserConfig->HAN_DisplayUser)     ||
        (type == HAN::MessageType_UserTalk && !Configuration::HuggleConfiguration->UserConfig->HAN_DisplayUserTalk))
          return;
    if (type == HAN::MessageType_Info)
    {
        text = "<font color=gray>" + text + "</font>";
    }
    this->ui->plainTextEdit->appendHtml(text);
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

void VandalNw::on_lineEdit_returnPressed()
{
    this->Message();
}

void VandalNw::TextEdit_anchorClicked(QString link)
{
    QString scheme;
    if (link.contains("://"))
    {
        scheme = link.mid(0, link.indexOf("://"));
        link = link.mid(link.indexOf("://") + 3);
    }
    if (scheme == "huggle")
    {
        // ok this is internal huggle link let's get a site
        if (!link.contains("/"))
            return;
        QStringList elements = link.split("/");
        if (elements.size() < 4)
            return;
        if (elements[0] == "diff")
        {
            QString wiki = elements[1];
            QString type = elements[2];
            if (type == "revid")
            {
                // we need to display a revid on given wiki, let's first get the wiki
                WikiSite *site = nullptr;
                foreach(WikiSite *sp, hcfg->Projects)
                {
                    if (wiki == sp->Name)
                    {
                        site = sp;
                        break;
                    }
                }
                if (site == nullptr)
                {
                    HUGGLE_DEBUG1("There is no such a wiki: " + wiki);
                    return;
                }
                revid_ht id = elements[3].toLongLong();
                MainWindow::HuggleMain->DisplayRevid(id, site);
            }
        }
    }
}

void VandalNw::OnIRCUserJoin(libircclient::Parser *px, libircclient::User *user, libircclient::Channel *channel)
{
    (void)px;
    (void)user;
    (void)channel;
    this->UpdateHeader();
}

void VandalNw::OnIRCSelfJoin(libircclient::Channel *channel)
{
    this->UpdateHeader();
}

void VandalNw::OnIRCChannelNames(libircclient::Parser *px)
{
    this->UpdateHeader();
}

void VandalNw::OnIRCNetworkFailure(QString reason, int code)
{
    (void) code;
    this->Insert(reason, HAN::MessageType_Info);
}

void VandalNw::OnIRCUserPart(libircclient::Parser *px, libircclient::Channel *channel)
{
    (void)px;
    (void)channel;
    this->UpdateHeader();
}

void VandalNw::OnIRCSelfPart(libircclient::Parser *px, libircclient::Channel *channel)
{
    (void)px;
    (void)channel;
    this->UpdateHeader();
}

void VandalNw::OnIRCChannelMessage(libircclient::Parser *px)
{
    if (px->GetParameters().count() < 1)
        return;

    QString channel = px->GetParameters()[0];
    QString message = px->GetText();
    QString nick = px->GetSourceUserInfo()->GetNick();

    if (!this->Ch2Site.contains(channel))
    {
        Syslog::HuggleLogs->DebugLog("Ignoring message to channel " + channel + " as we don't know which site it belongs to");
        return;
    }

    if (message.contains(this->Irc->GetNick()))
    {
        // Show a notification in tray
        MainWindow::HuggleMain->TrayMessage("Huggle anti-vandalism network", message);
    }

    WikiSite *site = this->Ch2Site[channel];
    HAN::MessageType mt;
    if (!nick.toLower().contains("bot"))
    {
        mt = HAN::MessageType_User;
    } else
    {
        mt = HAN::MessageType_Bot;
    }
    if (message.startsWith(this->Prefix))
    {
        QString Command = message.mid(2);
        if (Command.contains(" "))
        {
            Command = Command.mid(0, Command.indexOf(" "));
            QString revid = message.mid(message.indexOf(" ") + 1);
            QString parameter = "";
            if (revid.contains(" "))
            {
                parameter = revid.mid(revid.indexOf(" ") + 1);
                revid = revid.mid(0, revid.indexOf(" "));
            }
            if (Command == "GOOD")
            {
                int RevID = revid.toInt();
                WikiEdit *edit = MainWindow::HuggleMain->Queue1->GetWikiEditByRevID(RevID, site);
                if (edit != nullptr)
                {
                    this->ProcessGood(edit, nick);
                } else
                {
                    while (this->UnparsedGood.count() > Configuration::HuggleConfiguration->SystemConfig_CacheHAN)
                    {
                        this->UnparsedGood.removeAt(0);
                    }
                    this->UnparsedGood.append(HAN::GenericItem(site, RevID, nick));
                }
            }
            if (Command == "ROLLBACK")
            {
                int RevID = revid.toInt();
                WikiEdit *edit = MainWindow::HuggleMain->Queue1->GetWikiEditByRevID(RevID, site);
                if (edit != nullptr)
                {
                    this->ProcessRollback(edit, nick);
                } else
                {
                    while (this->UnparsedRoll.count() > Configuration::HuggleConfiguration->SystemConfig_CacheHAN)
                    {
                        this->UnparsedRoll.removeAt(0);
                    }
                    this->UnparsedRoll.append(HAN::GenericItem(site, RevID, nick));
                }
            }
            if (Command == "SUSPICIOUS")
            {
                int RevID = revid.toInt();
                WikiEdit *edit = MainWindow::HuggleMain->Queue1->GetWikiEditByRevID(RevID, site);
                if (edit != nullptr)
                {
                    this->ProcessSusp(edit, nick);
                } else
                {
                    while (this->UnparsedSusp.count() > Configuration::HuggleConfiguration->SystemConfig_CacheHAN)
                    {
                        this->UnparsedSusp.removeAt(0);
                    }
                    this->UnparsedSusp.append(HAN::GenericItem(site, RevID, nick));
                }
            }
            if (Command == "SCORED")
            {
                revid_ht RevID = revid.toLongLong();
                long Score = parameter.toLong();
                if (Score != 0)
                {
                    WikiEdit *edit = MainWindow::HuggleMain->Queue1->GetWikiEditByRevID(RevID, site);
                    if (edit != nullptr)
                    {
                        this->Insert("<font color=green>" + nick  + " rescored edit <b>" + edit->Page->PageName + "</b> by <b>" + edit->User->Username +
                                     "</b> (" + GenerateWikiDiffLink(revid, revid, edit->GetSite()) + ") by " + QString::number(Score) + "</font>", mt);
                        edit->Score += Score;
                        if (!edit->MetaLabels.contains("Bot score"))
                            edit->MetaLabels.insert("Bot score", QString::number(Score));
                        MainWindow::HuggleMain->Queue1->SortItemByEdit(edit);
                    } else
                    {
                        while (this->UnparsedScores.count() > Configuration::HuggleConfiguration->SystemConfig_CacheHAN)
                        {
                            this->UnparsedScores.removeAt(0);
                        }
                        this->UnparsedScores.append(HAN::RescoreItem(site, RevID, Score, nick));
                    }
                }
            }
        }
    } else
    {
        QString message_ = message;
        if (!hcfg->UserConfig->HtmlAllowedInIrc)
            message_ = SafeHtml(message_);
        if (hcfg->SystemConfig_Multiple)
        {
            this->Insert(nick + " (" + site->Name + "): " + message_, HAN::MessageType_UserTalk);
        } else
        {
            this->Insert(nick + ": " + message_, HAN::MessageType_UserTalk);
        }
    }
}

void VandalNw::OnIRCChannelQuit(libircclient::Parser *px, libircclient::Channel *channel)
{
    this->UpdateHeader();
}

void VandalNw::OnIRCLoggedIn(libircclient::Parser *px)
{
    this->JoinedMain = true;
    this->Insert("You are now connected to huggle antivandalism network", HAN::MessageType_Info);
    foreach(QString channel, this->Site2Channel.values())
    {
        if (channel.startsWith("#"))
            this->Irc->TransferRaw("JOIN " + channel);
    }
}

void VandalNw::OnConnected()
{
    this->UsersModified = true;
}

void VandalNw::OnDisconnected()
{
    /// \todo LOCALIZE ME
    this->Insert("Lost connection to antivandalism network", HAN::MessageType_Info);
}
