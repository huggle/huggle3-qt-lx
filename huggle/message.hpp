//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QtXml>
#include "core.hpp"
#include "history.hpp"
#include "collectable.hpp"
#include "apiquery.hpp"
#include "wikiuser.hpp"

namespace Huggle
{
    class Query;

    enum MessageStatus
    {
        MessageStatus_None,
        MessageStatus_Done,
        MessageStatus_Failed,
        MessageStatus_RetrievingToken,
        MessageStatus_RetrievingTalkPage,
        MessageStatus_SendingMessage
    };

    enum MessageError
    {
        MessageError_NoError,
        MessageError_Dependency,
        MessageError_Obsolete,
        MessageError_Unknown
    };

    //! This is similar to query, just it's more simple, you can use it to deliver messages to users
    class Message : public Collectable
    {
        public:
            //! Creates a new instance of message class that is used to deliver a message to users
            Message(WikiUser *target, QString MessageText, QString MessageSummary);
            ~Message();
            void RetrieveToken();
            //! Send a message to user
            void Send();
            //! Returns true in case that message was sent
            bool IsFinished();
            bool IsFailed();
            MessageStatus _Status;
            //! If this dependency is not a NULL then a message is sent after it is Processed (see Query::Processed())
            Query *Dependency;
            //! Title
            QString Title;
            //! If edit will be created in new section
            bool Section;
            //! Set this to false to remove huggle suffix from summary
            bool Suffix;
            //! User to deliver a message to
            WikiUser *user;
            //! This is a time for base revision which is used to resolve edit conflicts of edit
            QString BaseTimestamp;
            //! Text of message that will be appended to talk page
            QString Text;
            QString Summary;
            MessageError Error;
            QString ErrorText;
            //! Changing this to true will make the message be appended to existing section of same name
            bool SectionKeep;
        private:
            bool Done();
            void Fail(QString reason);
            //! This is a generic finish that either finishes whole message sending, or call respective finish function
            //! that is needed to finish the current step
            void Finish();
            //! Finish parsing the token
            bool FinishToken();
            //! Returns true if there is a valid token in memory

            //! Valid token means that it is syntactically correct, not that it isn't expired
            bool HasValidEditToken();
            bool RetrievingToken();
            bool IsSending();
            //! This function perform several checks and if everything is ok, it automatically calls next functions that send the message
            void PreflightCheck();
            //! This function write the new text to a talk page assuming that all checks were passed

            //! If you call this function before performing the checks, you will get in serious troubles
            void ProcessSend();
            void ProcessTalk();
            QString Append(QString text, QString OriginalText, QString Label);
            ApiQuery *qToken;
            ApiQuery *query;
            //! This is a text of talk page that was present before we change it
            QString Page;
            bool PreviousTalkPageRetrieved;
    };
}

#endif // MESSAGE_H
