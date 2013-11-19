//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "message.hpp"

using namespace Huggle;

Message::Message(WikiUser *target, QString Message, QString Summary)
{
    this->user = target;
    this->text = Message;
    summary = Summary;
    Done = false;
    Sending = false;
    this->Suffix = true;
    this->Dependency = NULL;
    this->query = NULL;
    this->PreviousTalkPageRetrieved = false;
    this->Page = target->GetContentsOfTalkPage();
    this->token = "none";
    /// \todo LOCALIZE ME
    /// \todo some users might prefer to have a month as header
    title = "Message from " + Configuration::HuggleConfiguration->UserName;
}

Message::~Message()
{
    if (this->query != NULL)
    {
        this->query->UnregisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
    }
}

void Message::Send()
{
    // first we need to get an edit token
    this->Sending = true;
    this->query = new ApiQuery();
    this->query->SetAction(ActionQuery);
    this->query->Parameters = "prop=info&intoken=edit&titles=" + QUrl::toPercentEncoding(user->GetTalk());
    /// \todo LOCALIZE ME
    this->query->Target = "Retrieving token to edit " + user->GetTalk();
    this->query->RegisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
    Core::HuggleCore->AppendQuery(query);
    this->query->Process();
}

void Message::Fail(QString reason)
{
    /// \todo LOCALIZE ME
    Huggle::Syslog::HuggleLogs->Log("Error: unable to deliver the message to " + user->Username + "; " + reason);
    this->Done = true;
    this->Sending = false;
    this->query->UnregisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
    this->query = NULL;
}

bool Message::Finished()
{
    if (this->Dependency != NULL)
    {
        if (!this->Dependency->Processed())
        {
            return false;
        } else
        {
            if (this->Dependency->Result->Failed)
            {
                // we can't continue because the dependency is fucked
                this->Dependency->UnregisterConsumer("keep");
                this->Dependency = NULL;
                this->Sending = false;
                this->Done = true;
                if (this->query != NULL)
                {
                    this->query->UnregisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
                }
                return true;
            }
            this->Dependency->UnregisterConsumer("keep");
            this->Dependency = NULL;
        }
    }
    if (this->Sending)
    {
        this->Finish();
    }
    if (this->Done)
    {
        return true;
    }
    return false;
}

void Message::Finish()
{
    if (Done)
    {
        // we really need to quit now because query is null
        return;
    }
    if (this->query == NULL)
    {
        return;
    }
    // we need to get a token
    if (token == "none")
    {
        if (!this->query->Processed())
        {
            return;
        }
        if (this->query->Result->Failed)
        {
            /// \todo LOCALIZE ME
            this->Fail("unable to retrieve the edit token");
            return;
        }
        QDomDocument d;
        d.setContent(this->query->Result->Data);
        QDomNodeList l = d.elementsByTagName("page");
        if (l.count() == 0)
        {
            /// \todo LOCALIZE ME
            this->Fail("no token was returned by request");
            Huggle::Syslog::HuggleLogs->DebugLog("No page");
            return;
        }
        QDomElement element = l.at(0).toElement();
        if (!element.attributes().contains("edittoken"))
        {
            /// \todo LOCALIZE ME
            this->Fail("the result doesn't contain the token");
            Huggle::Syslog::HuggleLogs->DebugLog("No token");
            return;
        }
        this->token = element.attribute("edittoken");
        this->query->UnregisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
        this->query = NULL;
        if (!this->Section && !this->PreviousTalkPageRetrieved)
        {
            // we need to retrieve the talk page
            this->query = new ApiQuery();
            this->query->RegisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
            this->query->SetAction(ActionQuery);
            this->query->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") + "&titles=" +
                    QUrl::toPercentEncoding(this->user->GetTalk());
            // inform user what is going on
            Core::HuggleCore->AppendQuery(this->query);
            this->query->Target = "Reading TP of " + this->user->Username;
            this->query->Process();
        } else
        {
            this->ProcessSend();
        }
        return;
    }
    if (!this->PreviousTalkPageRetrieved)
    {
        if (!this->query->Processed())
        {
            return;
        }
        if (this->query->Result->Failed)
        {
            /// \todo LOCALIZE ME
            Fail("unable to retrieve the user talk page");
            return;
        }
        this->ProcessTalk();
        this->query->UnregisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
        this->ProcessSend();
        return;
    }
    // we need to check the query
    if (!this->query->Processed())
    {
        return;
    }

    if (this->query->Result->Failed)
    {
        this->Fail("Failed to deliver the message");
        return;
    }

    bool sent = false;

    QDomDocument d;
    d.setContent(query->Result->Data);
    QDomNodeList l = d.elementsByTagName("edit");
    if (l.count() > 0)
    {
        QDomElement element = l.at(0).toElement();
        if (element.attributes().contains("result"))
        {
            if (element.attribute("result") == "Success")
            {
                /// \todo LOCALIZE ME
                Huggle::Syslog::HuggleLogs->Log("Successfuly delivered message to " + user->Username);
                sent = true;
                HistoryItem item;
                item.Result = "Success";
                item.Type = HistoryMessage;
                item.Target = user->Username;
                if (Core::HuggleCore->Main != NULL)
                {
                    Core::HuggleCore->Main->_History->Prepend(item);
                }
            }
        }
    }

    if (!sent)
    {
        /// \todo LOCALIZE ME
        Huggle::Syslog::HuggleLogs->Log("Failed to deliver a message to " + user->Username + " please check logs");
        Huggle::Syslog::HuggleLogs->DebugLog(query->Result->Data);
    }

    query->UnregisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
    Done = true;
    query = NULL;
}

void Message::ProcessSend()
{
    this->query = new ApiQuery();
    // prevent message from being sent twice
    this->query->RetryOnTimeoutFailure = false;
    this->query->Timeout = 600;
    this->query->Target = "Writing " + this->user->GetTalk();
    this->query->UsingPOST = true;
    this->query->RegisterConsumer(HUGGLECONSUMER_MESSAGE_SEND);
    this->query->SetAction(ActionEdit);
    QString s = summary;
    if (this->Suffix)
    {
        s += " " + Configuration::HuggleConfiguration->EditSuffixOfHuggle;
    }
    if (this->Section == false)
    {
        // original page needs to be included in new value
        this->text = this->Page + "\n\n" + this->text;
        this->query->Parameters = "title=" + QUrl::toPercentEncoding(user->GetTalk()) + "&summary=" + QUrl::toPercentEncoding(s)
                + "&text=" + QUrl::toPercentEncoding(this->text)
                + "&token=" + QUrl::toPercentEncoding(this->token);
    }else
    {
        this->query->Parameters = "title=" + QUrl::toPercentEncoding(user->GetTalk()) + "&section=new&sectiontitle="
                + QUrl::toPercentEncoding(this->title) + "&summary=" + QUrl::toPercentEncoding(s)
                + "&text=" + QUrl::toPercentEncoding(this->text)
                + "&token=" + QUrl::toPercentEncoding(this->token);
    }
    Core::HuggleCore->AppendQuery(query);
    this->query->Process();
}

void Message::ProcessTalk()
{
    QDomDocument d;
    d.setContent(this->query->Result->Data);
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
            /// \todo LOCALIZE ME
            this->Fail("Unable to retrieve " + this->user->GetTalk() + " stopping message delivery to that user");
            return;
        }
    } else
    {
        if (!missing)
        {
            /// \todo LOCALIZE ME
            this->Fail("Unable to retrieve " + this->user->GetTalk() + " stopping message delivery to that user");
            Huggle::Syslog::HuggleLogs->DebugLog(this->query->Result->Data);
            return;
        }
    }
}
