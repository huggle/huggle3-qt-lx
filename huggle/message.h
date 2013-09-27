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
#include "core.h"
#include "history.h"
#include "apiquery.h"
#include "wikiuser.h"

class Query;

class Message
{
public:
    Message(WikiUser *target, QString Message, QString Summary);
    ~Message();
    void Send();
    void Fail(QString reason);
    bool Finished();
    Query *Dependency;
    QString title;
    ApiQuery *query;
    QString token;
    //! If edit will be created in new section
    bool Section;
    WikiUser *user;
    QString text;
    QString summary;
private:
    void Finish();
    bool Sending;
    bool Done;
};

#endif // MESSAGE_H
