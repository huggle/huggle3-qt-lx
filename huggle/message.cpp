//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "message.h"

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
    delete query;
}

void Message::Send()
{
    // first we need to get an edit token
    Sending = true;
    query = new ApiQuery();
    query->SetAction(ActionQuery);
    query->Parameters = "prop=info&intoken=edit&titles=" + user->GetTalk();
    query->Target = "Retrieving token to edit " + user->GetTalk();
    query->DeleteLater = true;
    Core::RunningQueries.append(query);
    query->Process();
}

void Message::Fail(QString reason)
{
    Core::Log("Error: unable to deliver the message to " + user->Username + "; " + reason);
    Done = true;
    Sending = false;
    query->SafeDelete();
    query->DeleteLater = false;
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
                this->Dependency->DeleteLater = false;
                this->Dependency = NULL;
                this->Sending = false;
                this->Done = true;
                return true;
            }
            this->Dependency->DeleteLater = false;
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
            Fail("unable to retrieve the edit token");
            return;
        }
        QDomDocument d;
        d.setContent(query->Result->Data);
        QDomNodeList l = d.elementsByTagName("page");
        if (l.count() == 0)
        {
            Fail("no token was returned by request");
            Core::DebugLog("No page");
            return;
        }
        QDomElement element = l.at(0).toElement();
        if (!element.attributes().contains("edittoken"))
        {
            Fail("the result doesn't contain the token");
            Core::DebugLog("No token");
            return;
        }
        token = element.attribute("edittoken");
        query->SafeDelete();
        query->DeleteLater = false;
        query = new ApiQuery();
        query->Target = "Writing " + user->GetTalk();
        query->UsingPOST = true;
        query->DeleteLater = true;
        query->SetAction(ActionEdit);
        query->Parameters = "title=" + QUrl::toPercentEncoding(user->GetTalk()) + "&section=new&sectiontitle="
                + QUrl::toPercentEncoding(this->title) + "&text=" + QUrl::toPercentEncoding(this->text)
                + QUrl::toPercentEncoding(this->title) + "&token=" + QUrl::toPercentEncoding(this->token);
        Core::RunningQueries.append(query);
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
                Core::Log("Successfuly delivered message to " + user->Username);
                sent = true;
            }
        }
    }

    if (!sent)
    {
        Core::Log("Failed to deliver a message to " + user->Username + " please check logs");
        Core::DebugLog(query->Result->Data);
    }

    query->SafeDelete();
    query->DeleteLater = false;
    Done = true;
    query = NULL;
}
