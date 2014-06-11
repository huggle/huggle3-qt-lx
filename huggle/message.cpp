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
#include "configuration.hpp"
#include "core.hpp"
#include "collectable.hpp"
#include "querypool.hpp"
#include "localization.hpp"
#include "syslog.hpp"
#include "generic.hpp"

using namespace Huggle;

Message::Message(WikiUser *target, QString MessageText, QString MessageSummary)
{
    // we copy the user here so that we can let the caller delete it
    this->user = new WikiUser(target);
    this->Text = MessageText;
    this->Summary = MessageSummary;
    this->Suffix = true;
    this->Dependency = nullptr;
    this->qToken = nullptr;
    this->SectionKeep = false;
    this->query = nullptr;
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
    if (this->query != nullptr)
    {
        this->query->UnregisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
    }
    GC_DECNAMEDREF(this->Dependency, "keep");
    delete this->user;
}

void Message::RetrieveToken()
{
    this->_Status = Huggle::MessageStatus_RetrievingToken;
    if (this->qToken != nullptr)
    {
        Syslog::HuggleLogs->DebugLog("Memory leak at void Message::RetrieveToken()");
    }
    this->qToken = new ApiQuery();
    this->qToken->SetAction(ActionQuery);
    this->qToken->Parameters = "prop=info&intoken=edit&titles=" + QUrl::toPercentEncoding(this->user->GetTalk());
    this->qToken->Target = _l("message-retrieve-new-token", this->user->GetTalk());
    this->qToken->RegisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
    QueryPool::HugglePool->AppendQuery(this->qToken);
    this->qToken->Process();
}

void Message::Send()
{
    if (this->HasValidEditToken())
    {
        this->PreflightCheck();
        return;
    }
    // first we need to get an edit token
    this->RetrieveToken();
}

void Message::Fail(QString reason)
{
    QStringList parameters;
    parameters << this->user->Username << reason;
    Huggle::Syslog::HuggleLogs->ErrorLog(_l("message-er", parameters));
    this->_Status = Huggle::MessageStatus_Failed;
    this->Error = Huggle::MessageError_Unknown;
    this->ErrorText = reason;
    GC_DECNAMEDREF(this->qToken, HUGGLECONSUMER_MESSAGE_SEND);
    GC_DECNAMEDREF(this->query, HUGGLECONSUMER_MESSAGE_SEND);
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
            this->Dependency->UnregisterConsumer("keep");
            this->Dependency = nullptr;
            this->_Status = Huggle::MessageStatus_Failed;
            this->Error = MessageError_Dependency;
            if (this->query != nullptr)
            {
                this->query->UnregisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
                this->query = nullptr;
            }
            return true;
        }
    }
    if (this->RetrievingToken())
    {
        // we are retrieving token so we can check if the token
        // has beed retrieved and if yes, we can continue
        if (this->FinishToken())
        {
            // good, we have the token now so we can edit the page, however some checks are needed before we do that
            this->PreflightCheck();
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
    return (!Configuration::HuggleConfiguration->TemporaryConfig_EditToken.isEmpty());
}

bool Message::RetrievingToken()
{
    return (this->_Status == Huggle::MessageStatus_RetrievingToken);
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
        // we need to get a token
        if (this->_Status == Huggle::MessageStatus_RetrievingToken)
        {
            // we are already retrieving the token, so let's wait for it to finish
            return;
        } else
        {
            this->RetrieveToken();
            return;
        }
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
        if (this->query->Result->Failed)
        {
            this->Fail(_l("message-fail-retrieve-talk"));
            return;
        }
        this->ProcessTalk();
        this->query->UnregisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
        this->query = nullptr;
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
            this->Fail("Failed to deliver the message");
            return;
        }
        bool sent = false;
        QDomDocument dResult_;
        dResult_.setContent(query->Result->Data);
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
                    this->Fail("Edit conflict");
                    Huggle::Syslog::HuggleLogs->DebugLog("EC while delivering message to " + this->user->Username);
                    this->Error = MessageError_Obsolete;
                } else if (ec == "articleexists")
                {
                    this->Fail("Edit conflict");
                    this->Error = MessageError_ArticleExist;
                } else
                {
                    this->Fail("Unknown error: " + ec);
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
                    Huggle::Syslog::HuggleLogs->Log(_l("message-done", this->user->Username));
                    sent = true;
                    HistoryItem *item = new HistoryItem();
                    item->Result = "Success";
                    item->NewPage = this->CreateOnly;
                    item->Type = HistoryMessage;
                    item->Target = user->Username;
                    if (this->Dependency && this->Dependency->HI)
                    {
                        this->Dependency->HI->UndoDependency = item;
                        item->ReferencedBy = this->Dependency->HI;
                    }
                    if (Core::HuggleCore->Main != nullptr)
                    {
                        Core::HuggleCore->Main->_History->Prepend(item);
                    }
                }
            }
        }
        if (!sent)
        {
            /// \todo LOCALIZE ME
            Huggle::Syslog::HuggleLogs->Log("Failed to deliver a message to " + this->user->Username + " please check logs");
            Huggle::Syslog::HuggleLogs->DebugLog(this->query->Result->Data);
        }
        this->query->UnregisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
        GC_DECNAMEDREF(this->Dependency, "keep");
        this->_Status = Huggle::MessageStatus_Done;
        this->query = nullptr;
    }
}

bool Message::FinishToken()
{
    if (this->qToken == nullptr)
    {
        throw new Huggle::Exception("qToken must not be NULL in this context", "void Message::FinishToken()");
    }
    if (!this->qToken->IsProcessed())
    {
        return false;
    }
    if (this->qToken->Result->Failed)
    {
        this->Fail(_l("message-fail-token-1"));
        return false;
    }
    QDomDocument dToken_;
    dToken_.setContent(this->qToken->Result->Data);
    QDomNodeList l = dToken_.elementsByTagName("page");
    if (l.count() == 0)
    {
        this->Fail(_l("message-fail-token-2"));
        Huggle::Syslog::HuggleLogs->DebugLog("No page");
        return false;
    }
    QDomElement element = l.at(0).toElement();
    if (!element.attributes().contains("edittoken"))
    {
        /// \todo LOCALIZE ME
        this->Fail("the result doesn't contain the token");
        Huggle::Syslog::HuggleLogs->DebugLog("No token");
        return false;
    }
    Configuration::HuggleConfiguration->TemporaryConfig_EditToken = element.attribute("edittoken");
    this->qToken->UnregisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
    this->qToken = nullptr;
    return true;
}

void Message::PreflightCheck()
{
    // check if we can directly append the data or if we need to fetch the previous page content before we do that
    if ((!this->CreateInNewSection || this->SectionKeep) && !this->PreviousTalkPageRetrieved)
    {
        if (this->query != nullptr)
        {
            Syslog::HuggleLogs->DebugLog("Warning, Message::query != NULL on new allocation, possible memory leak in void Message::PreflightCheck()");
        }
        this->_Status = MessageStatus_RetrievingTalkPage;
        // we need to retrieve the talk page
        this->query = Generic::RetrieveWikiPageContents(this->user->GetTalk());
        this->query->RegisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
        // inform user what is going on
        QueryPool::HugglePool->AppendQuery(this->query);
        /// \todo LOCALIZE ME
        this->query->Target = "Reading TP of " + this->user->Username;
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
        if (!this->CreateOnly && (this->user->TalkPage_RetrievalTime().addSecs(
                                  Configuration::HuggleConfiguration->UserConfig->TalkPageFreshness
                                      ) < QDateTime::currentDateTime()))
        {
            this->Error = Huggle::MessageError_Expired;
            this->_Status = Huggle::MessageStatus_Failed;
            return;
        } else if (!this->CreateOnly && Configuration::HuggleConfiguration->Verbosity >= 2)
        {
            Syslog::HuggleLogs->DebugLog("Message to " + this->user->Username + " old " +
                                         QString::number(this->user->TalkPage_RetrievalTime()
                                         .addSecs(Configuration::HuggleConfiguration->UserConfig->TalkPageFreshness)
                                         .secsTo(QDateTime::currentDateTime())), 2);
        }
    }
    if (this->query != nullptr)
    {
        Syslog::HuggleLogs->DebugLog("Warning, Message::query != NULL on new allocation, possible memory leak in void Message::ProcessSend()");
    }
    this->query = new ApiQuery();
    // prevent message from being sent twice
    this->query->RetryOnTimeoutFailure = false;
    this->query->Timeout = 600;
    this->query->Target = "Writing " + this->user->GetTalk();
    this->query->UsingPOST = true;
    this->query->RegisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
    this->query->SetAction(ActionEdit);
    QString s = Summary;
    QString parameters = "";
    if (!this->BaseTimestamp.isEmpty())
    {
        parameters = "&basetimestamp=" + QUrl::toPercentEncoding(this->BaseTimestamp);
        Syslog::HuggleLogs->DebugLog("Using base timestamp for edit of " + user->GetTalk() + ": " + this->BaseTimestamp, 2);
    }
    if (this->StartTimestamp != "")
    {
        parameters += "&starttimestamp=" + QUrl::toPercentEncoding(this->StartTimestamp);
        Syslog::HuggleLogs->DebugLog("Using start timestamp for edit of " + user->GetTalk() + ": " + this->StartTimestamp, 2);
    }
    if (this->CreateOnly)
    {
        parameters += "&createonly=1";
    }
    if (this->Suffix)
    {
        s += " " + Configuration::HuggleConfiguration->ProjectConfig->EditSuffixOfHuggle;
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
        this->query->Parameters = "title=" + QUrl::toPercentEncoding(user->GetTalk()) + "&summary=" + QUrl::toPercentEncoding(s)
                + "&text=" + QUrl::toPercentEncoding(this->Text) + parameters
                + "&token=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->TemporaryConfig_EditToken);
    }else
    {
        this->query->Parameters = "title=" + QUrl::toPercentEncoding(user->GetTalk()) + "&section=new&sectiontitle="
                + QUrl::toPercentEncoding(this->Title) + "&summary=" + QUrl::toPercentEncoding(s)
                + "&text=" + QUrl::toPercentEncoding(this->Text) + parameters + "&token="
                + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->TemporaryConfig_EditToken);
    }
    Syslog::HuggleLogs->DebugLog(QString(" Message to %1 with parameters: %2").arg(this->user->Username, parameters), 2);
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
            this->Fail(_l("message-fail-re-user-tp", this->user->GetTalk()));
            return;
        }
    } else
    {
        if (!missing)
        {
            this->Fail(_l("message-fail-re-user-tp", this->user->GetTalk()));
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
