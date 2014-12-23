//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "message.hpp"
#include <QtXml>
#include "collectable.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "history.hpp"
#include "localization.hpp"
#include "mainwindow.hpp"
#include "generic.hpp"
#include "querypool.hpp"
#include "syslog.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"

using namespace Huggle;

Message::Message(WikiUser *target, QString MessageText, QString MessageSummary)
{
    // we copy the user here so that we can let the caller delete it
    this->User = new WikiUser(target);
    this->Text = MessageText;
    this->Summary = MessageSummary;
    this->Suffix = true;
    this->SectionKeep = false;
    this->_Status = Huggle::MessageStatus_None;
    this->PreviousTalkPageRetrieved = false;
    this->Page = "";
    this->BaseTimestamp = "";
    this->CreateOnly = false;
    this->StartTimestamp = "";
    this->RequireFresh = false;
    this->Error = MessageError_NoError;
    this->ErrorText = "";
    this->Title = "Message from " + Configuration::HuggleConfiguration->SystemConfig_Username;
}

Message::~Message()
{
    delete this->User;
}

void Message::Send()
{
    if (!this->User)
        throw new Huggle::NullPointerException("local WikiUser User", BOOST_CURRENT_FUNCTION);

    if (this->HasValidEditToken())
    {
        this->PreflightCheck();
        return;
    }
    // there is no token to use
    this->Fail("No token");
}

void Message::Fail(QString reason)
{
    QStringList parameters;
    parameters << this->User->Username << reason;
    Huggle::Syslog::HuggleLogs->ErrorLog(_l("message-er", parameters));
    this->_Status = Huggle::MessageStatus_Failed;
    this->Error = Huggle::MessageError_Unknown;
    this->ErrorText = reason;
    this->query.Delete();
}

bool Message::IsFinished()
{
    if (this->Done())
    {
        return true;
    }
    if (this->Dependency != nullptr)
    {
        if (!this->Dependency->IsProcessed())
        {
            return false;
        } else if (this->Dependency->IsFailed())
        {
            // we can't continue because the dependency is fucked
            this->Dependency.Delete();
            this->_Status = Huggle::MessageStatus_Failed;
            this->Error = MessageError_Dependency;
            this->query.Delete();
            return true;
        }
    }
    this->Finish();
    return this->Done();
}

bool Message::IsFailed()
{
    return this->_Status == Huggle::MessageStatus_Failed;
}

bool Message::HasValidEditToken()
{
    return (!this->User->GetSite()->GetProjectConfig()->Token_Csrf.isEmpty());
}

bool Message::IsSending()
{
    return (this->_Status == Huggle::MessageStatus_SendingMessage);
}

bool Message::Done()
{
    return (this->_Status == Huggle::MessageStatus_Done || this->_Status == Huggle::MessageStatus_Failed);
}

void Message::Finish()
{
    if (this->Done())
    {
        // we really need to quit now because query is null
        return;
    }
    // Check if we have a valid token
    if (!this->HasValidEditToken())
    {
        this->Fail("No token!");
        return;
    }
    if (this->query == nullptr)
    {
        // we should never reach this code
        return;
    }
    if (this->_Status == Huggle::MessageStatus_RetrievingTalkPage && !this->PreviousTalkPageRetrieved)
    {
        // we need to finish retrieving of previous talk page
        if (!this->query->IsProcessed())
        {
            return;
        }
        if (this->query->IsFailed())
        {
            this->Fail(_l("message-fail-retrieve-talk"));
            return;
        }
        this->ProcessTalk();
        this->query.Delete();
        // we should be able to finish sending now
        this->ProcessSend();
        return;
    }

    if (this->_Status == Huggle::MessageStatus_SendingMessage)
    {
        // we need to check the query
        if (!this->query->IsProcessed())
        {
            return;
        }
        if (this->query->IsFailed())
        {
            this->Fail(_l("message-fail"));
            return;
        }
        bool sent = false;
        QDomDocument dResult_;
        dResult_.setContent(this->query->Result->Data);
        QDomNodeList e = dResult_.elementsByTagName("error");
        if (e.count() > 0)
        {
            QDomElement element = e.at(0).toElement();
            if (element.attributes().contains("code"))
            {
                QString ec = element.attribute("code");
                if (ec == "editconflict")
                {
                    // someone edit the page meanwhile which means that our token has expired
                    this->Fail(_l("edit-conflict"));
                    Huggle::Syslog::HuggleLogs->DebugLog("EC while delivering message to " + this->User->Username);
                    this->Error = MessageError_Obsolete;
                } else if (ec == "articleexists")
                {
                    this->Fail(_l("edit-conflict"));
                    this->Error = MessageError_ArticleExist;
                } else
                {
                    this->Fail(_l("error-unknown-code",ec));
                    this->Error = MessageError_Unknown;
                }
                return;
            }
        }
        QDomNodeList editlist = dResult_.elementsByTagName("edit");
        if (editlist.count() > 0)
        {
            QDomElement element = editlist.at(0).toElement();
            if (element.attributes().contains("result"))
            {
                if (element.attribute("result") == "Success")
                {
                    Huggle::Syslog::HuggleLogs->Log(_l("message-done", this->User->Username, this->User->GetSite()->Name));
                    sent = true;
                    HistoryItem *item = new HistoryItem();
                    item->Result = _l("successful");
                    item->NewPage = this->CreateOnly;
                    item->Site = this->User->GetSite();
                    item->Type = HistoryMessage;
                    item->Target = User->Username;
                    if (this->Dependency != nullptr && this->Dependency->HI != nullptr)
                    {
                        this->Dependency->HI->UndoDependency = item;
                        item->ReferencedBy = this->Dependency->HI;
                    }
                    if (MainWindow::HuggleMain != nullptr)
                    {
                        MainWindow::HuggleMain->_History->Prepend(item);
                    }
                }
            }
        }
        if (!sent)
        {
            Huggle::Syslog::HuggleLogs->Log(_l("message-error", this->User->Username));
            Huggle::Syslog::HuggleLogs->DebugLog(this->query->Result->Data);
        }
        this->Dependency.Delete();
        this->_Status = Huggle::MessageStatus_Done;
        this->query = nullptr;
    }
}

void Message::PreflightCheck()
{
    // check if we can directly append the data or if we need to fetch the previous page content before we do that
    if ((!this->CreateInNewSection || this->SectionKeep) && !this->PreviousTalkPageRetrieved)
    {
        this->_Status = MessageStatus_RetrievingTalkPage;
        // we need to retrieve the talk page
        this->query = Generic::RetrieveWikiPageContents(this->User->GetTalk(), this->User->GetSite());
        // inform user what is going on
        QueryPool::HugglePool->AppendQuery(this->query);
        /// \todo LOCALIZE ME
        this->query->Target = "Reading TP of " + this->User->Username;
        this->query->Process();
    } else
    {
        this->PreviousTalkPageRetrieved = true;
        this->ProcessSend();
    }
}

void Message::ProcessSend()
{
    this->_Status = MessageStatus_SendingMessage;
    if (this->RequireFresh && Configuration::HuggleConfiguration->UserConfig->TalkPageFreshness != 0)
    {
        if (!this->CreateOnly && (this->User->TalkPage_RetrievalTime().addSecs(hcfg->UserConfig->TalkPageFreshness) < QDateTime::currentDateTime()))
        {
            this->Error = Huggle::MessageError_Expired;
            this->_Status = Huggle::MessageStatus_Failed;
            return;
        } else if (!this->CreateOnly && Configuration::HuggleConfiguration->Verbosity >= 2)
        {
            HUGGLE_DEBUG("Message to " + this->User->Username + " old " + QString::number(this->User->TalkPage_RetrievalTime()
                           .addSecs(Configuration::HuggleConfiguration->UserConfig->TalkPageFreshness)
                           .secsTo(QDateTime::currentDateTime())), 2);
        }
    }
    this->query = new ApiQuery(ActionEdit, this->User->GetSite());
    // prevent message from being sent twice
    this->query->RetryOnTimeoutFailure = false;
    this->query->Timeout = 600;
    this->query->Target = "Writing " + this->User->GetTalk();
    this->query->UsingPOST = true;
    QString summary = this->Summary;
    QString parameters = "";
    if (!this->BaseTimestamp.isEmpty())
    {
        parameters = "&basetimestamp=" + QUrl::toPercentEncoding(this->BaseTimestamp);
        HUGGLE_DEBUG("Using base timestamp for edit of " + this->User->GetTalk() + ": " + this->BaseTimestamp, 2);
    }
    if (!this->StartTimestamp.isEmpty())
    {
        parameters += "&starttimestamp=" + QUrl::toPercentEncoding(this->StartTimestamp);
        HUGGLE_DEBUG("Using start timestamp for edit of " + this->User->GetTalk() + ": " + this->StartTimestamp, 2);
    }
    if (this->CreateOnly)
    {
        parameters += "&createonly=1";
    }
    if (this->Suffix)
    {
        summary = Configuration::GenerateSuffix(summary, this->User->Site->GetProjectConfig());
    }
    if (this->SectionKeep || !this->CreateInNewSection)
    {
        if (this->SectionKeep)
        {
            this->Text = this->Append(this->Text, this->Page, this->Title);
        } else
        {
            // original page needs to be included in new value
            if (!this->Page.isEmpty())
                this->Text = this->Page + "\n\n" + this->Text;
        }
        this->query->Parameters = "title=" + QUrl::toPercentEncoding(User->GetTalk()) + "&summary=" + QUrl::toPercentEncoding(summary)
                + "&text=" + QUrl::toPercentEncoding(this->Text) + parameters
                + "&token=" + QUrl::toPercentEncoding(this->User->GetSite()->GetProjectConfig()->Token_Csrf);
    }else
    {
        this->query->Parameters = "title=" + QUrl::toPercentEncoding(User->GetTalk()) + "&section=new&sectiontitle="
                + QUrl::toPercentEncoding(this->Title) + "&summary=" + QUrl::toPercentEncoding(summary)
                + "&text=" + QUrl::toPercentEncoding(this->Text) + parameters + "&token="
                + QUrl::toPercentEncoding(this->User->GetSite()->GetProjectConfig()->Token_Csrf);
    }
    HUGGLE_DEBUG(QString(" Message to %1 with parameters: %2").arg(this->User->Username, parameters), 2);
    QueryPool::HugglePool->AppendQuery(query);
    this->query->Process();
}

void Message::ProcessTalk()
{
    QDomDocument d;
    d.setContent(this->query->Result->Data);
    this->PreviousTalkPageRetrieved = true;
    QDomNodeList page = d.elementsByTagName("rev");
    QDomNodeList code = d.elementsByTagName("page");
    bool missing = false;
    if (code.count() > 0)
    {
        QDomElement e = code.at(0).toElement();
        if (e.attributes().contains("missing"))
        {
            missing = true;
        }
    }
    // get last id
    if (missing != true && page.count() > 0)
    {
        QDomElement e = page.at(0).toElement();
        if (e.nodeName() == "rev")
        {
            this->Page = e.text();
            this->PreviousTalkPageRetrieved = true;
            return;
        } else
        {
            // Unable to retrieve this->user->GetTalk() stopping message delivery to that user
            this->Fail(_l("message-fail-re-user-tp", this->User->GetTalk()));
            return;
        }
    } else
    {
        if (!missing)
        {
            this->Fail(_l("message-fail-re-user-tp", this->User->GetTalk()));
            Huggle::Syslog::HuggleLogs->DebugLog(this->query->Result->Data);
            return;
        }
    }
}

QString Message::Append(QString text, QString OriginalText, QString Label)
{
    if (Label.isEmpty())
    {
        // there is nothing to insert this to
        return OriginalText += "\n\n" + text + "\n\n";
    }
    QRegExp regex("\\s*==\\s*" + QRegExp::escape(Label) + "\\s*==");
    if (!OriginalText.contains(regex))
    {
        // there is no section to append to
        if (!OriginalText.isEmpty())
        {
            OriginalText = OriginalText + "\n\n";
        }
        OriginalText += "== " + Label + " ==\n\n" + text;
        return OriginalText;
    }
    QRegExp header("^\\s*==.*==\\s*$");
    int Post = Text.lastIndexOf(regex);
    // we need to check if there is any other section after this one
    QString Section = OriginalText.mid(Post);
    if (Section.contains("\n"))
    {
        // cut the header text
        int Diff = Section.indexOf("\n") + 1;
        Post += Diff;
        Section = Section.mid(Diff);
    }
    // we assume there is no other section and if there is some we change this
    int StartPoint = OriginalText.length();
    if (Section.contains(header))
    {
        // yes there is some other section, so we need to know where it is
        StartPoint = OriginalText.indexOf(header, Post);
    }
    // write the text exactly after the start point, but leave some space after it
    OriginalText = OriginalText.insert(StartPoint, "\n\n" + text + "\n\n");
    return OriginalText;
}
