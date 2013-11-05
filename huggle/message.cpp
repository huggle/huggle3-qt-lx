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
    user = target;
    text = Message;
    summary = Summary;
    Done = false;
    Sending = false;
    this->Dependency = NULL;
    this->query = NULL;
    this->token = "none";
    title = "Message from " + Configuration::UserName;
}

Message::~Message()
{
    //delete query;
}

void Message::Send()
{
    // first we need to get an edit token
    Sending = true;
    query = new ApiQuery();
    query->SetAction(ActionQuery);
    query->Parameters = "prop=info&intoken=edit&titles=" + QUrl::toPercentEncoding(user->GetTalk());
    /// \todo LOCALIZE ME
    query->Target = "Retrieving token to edit " + user->GetTalk();
    query->RegisterConsumer("Message::Send()");
    Core::AppendQuery(query);
    query->Process();
}

void Message::Fail(QString reason)
{
    /// \todo LOCALIZE ME
    Core::Log("Error: unable to deliver the message to " + user->Username + "; " + reason);
    Done = true;
    Sending = false;
    query->UnregisterConsumer("Message::Send()");
    query = NULL;
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
                if (query != NULL)
                {
                    query->UnregisterConsumer("Message::Send()");
                }
                return true;
            }
            this->Dependency->UnregisterConsumer("keep");
            this->Dependency = NULL;
        }
    }
    if (this->Sending)
    {
        Finish();
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
    // we need to get a token
    if (token == "none")
    {
        if (!query->Processed())
        {
            return;
        }
        if (query->Result->Failed)
        {
            /// \todo LOCALIZE ME
            Fail("unable to retrieve the edit token");
            return;
        }
        QDomDocument d;
        d.setContent(query->Result->Data);
        QDomNodeList l = d.elementsByTagName("page");
        if (l.count() == 0)
        {
            /// \todo LOCALIZE ME
            Fail("no token was returned by request");
            Core::DebugLog("No page");
            return;
        }
        QDomElement element = l.at(0).toElement();
        if (!element.attributes().contains("edittoken"))
        {
            /// \todo LOCALIZE ME
            Fail("the result doesn't contain the token");
            Core::DebugLog("No token");
            return;
        }
        token = element.attribute("edittoken");
        query->UnregisterConsumer("Message::Send()");
        query = new ApiQuery();
        query->Target = "Writing " + user->GetTalk();
        query->UsingPOST = true;
        query->RegisterConsumer("Message::Finish()");
        query->SetAction(ActionEdit);
        query->Parameters = "title=" + QUrl::toPercentEncoding(user->GetTalk()) + "&section=new&sectiontitle="
                + QUrl::toPercentEncoding(this->title) + "&summary=" + QUrl::toPercentEncoding(summary + " " + Configuration::EditSuffixOfHuggle)
                + "&text=" + QUrl::toPercentEncoding(this->text)
                + "&token=" + QUrl::toPercentEncoding(this->token);
        Core::AppendQuery(query);
        query->Process();
        return;
    }
    // we need to check the query
    if (!query->Processed())
    {
        return;
    }

    if (query->Result->Failed)
    {
        Fail("Failed to deliver the message");
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
                Core::Log("Successfuly delivered message to " + user->Username);
                sent = true;
                HistoryItem item;
                item.Result = "Success";
                item.Type = HistoryMessage;
                item.Target = user->Username;
                if (Core::Main != NULL)
                {
                    Core::Main->_History->Prepend(item);
                }
            }
        }
    }

    if (!sent)
    {
        /// \todo LOCALIZE ME
        Core::Log("Failed to deliver a message to " + user->Username + " please check logs");
        Core::DebugLog(query->Result->Data);
    }

    query->UnregisterConsumer("Message::Finish()");
    Done = true;
    query = NULL;
}
