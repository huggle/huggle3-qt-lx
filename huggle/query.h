//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef QUERY_H
#define QUERY_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QNetworkAccessManager>
#include "queryresult.h"
#include "exception.h"
#include "querygc.h"

enum _Status
{
    Null,
    Done,
    Processing,
    InError
};

enum QueryType
{
    QueryEdit,
    QueryNull,
    QueryApi
};

//! Every request to website is processed as a query, this is a base object that all
//! other queries are derived from
class Query : public QObject
{
public:
    //! Result
    QueryResult *Result;
    unsigned int ID;
    //! Current status
    enum _Status Status;
    QString CustomStatus;
    QueryType Type;
    //! Return true in case this query has been finished
    static QNetworkAccessManager NetworkManager;
    //! If you put anything in here, it either must be NULL or query
    //! that is processed
    Query *Dependency;
    Query();
    virtual ~Query();
    virtual bool Processed();
    virtual void Process() {}
    virtual void Kill() {}
    bool IsManaged();
    virtual QString QueryTypeToString();
    virtual QString QueryTargetToString();
    virtual QString QueryStatusToString();
    //! Use this if you are not sure if you can delete this object in this moment
    virtual bool SafeDelete(bool forced = false);
    void RegisterConsumer(QString consumer);
    void UnregisterConsumer(QString consumer);
    QString DebugQgc();

private:
    bool Managed;
    //! Some queries are needed for dependency setup, so we need to delete them
    //! later once the dependency is processed
    QStringList Consumers;
    static unsigned int LastID;
};

#endif // QUERY_H
