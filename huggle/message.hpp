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
#include "apiquery.hpp"
#include "wikiuser.hpp"

namespace Huggle
{
    class Query;

    //! This is similar to query, just it's more simple, you can use it to deliver messages to users
    class Message
    {
    public:
        Message(WikiUser *target, QString Message, QString Summary);
        ~Message();
        //! Send a message to user
        void Send();
        //! Returns true in case that message was sent
        bool Finished();
        //! If this dependency is not a NULL then a message is sent after it is Processed (see Query::Processed())
        Query *Dependency;
        //! Title
        QString title;
        //! Token that is needed in order to write to page
        QString token;
        //! If edit will be created in new section
        bool Section;
        //! User to deliver a message to
        WikiUser *user;
        //! Text of message that will be appended to talk page
        QString text;
        QString summary;
    private:
        void Fail(QString reason);
        void Finish();
        ApiQuery *query;
        bool Sending;
        bool Done;
    };
}

#endif // MESSAGE_H
