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

#include <QThread>
#include <QObject>
#include <QString>
#include <QMutex>
#include <QStringList>
#include <QNetworkAccessManager>
#include "queryresult.h"
#include "collectable.h"
#include "exception.h"
#include "gc.h"

namespace Huggle
{
    class Query;
    typedef void* (*Callback) (Query*);

    //! Status of a query
    enum _Status
    {
        StatusNull,
        StatusDone,
        StatusProcessing,
        StatusInError
    };

    /*!
     * \brief The QueryType enum
     */
    enum QueryType
    {
        //! Edit
        QueryEdit,
        //! Default
        QueryNull,
        //! Whitelist
        QueryWl,
        //! Api
        QueryApi,
        //! Revert
        QueryRevert
    };

    //! Query base class for all http queries executed by huggle

    //! Every request to website is processed as a query, this is a base object that all
    //! other queries are derived from. The query system is using own GC see a QueryGC
    //! That means every query is either unmanaged or managed. In case it is managed,
    //! the GC will care about it being removed from operating memory and you must not
    //! call a delete on it, otherwise program will crash.
    class Query : public QObject, public Collectable
    {
    public:
        //! Result of query, see documentation of QueryResult for more
        QueryResult *Result;
        //! Current status of a query
        enum _Status Status;
        //! Custom status

        //! This can be used to override the current string representation of status with
        //! some custom string, the system will still process the primary status but
        //! user will see this custom string in a process list
        QString CustomStatus;
        //! Type of a query

        //! This is very useful when you are casting a query to different type
        QueryType Type;
        //! Callback

        //! If this is not a NULL this function will be called by query
        //! once it's finished, a consumer called "delegate" will be created and you
        //! will have to either replace it or remove in your function
        //! otherwise you create a leak in huggle
        Callback callback;
        //! This is a pointer to object returned by your callback function
        void* CallbackResult;
        static QNetworkAccessManager NetworkManager;
        //! Query doesn't have internal data displayed in debug log, this is usefull
        //! when you are working with passwords in parameters
        bool HiddenQuery;
        //! Dependency for query

        //! If you put anything in here, it either must be NULL or query
        //! that is processed. The query will not be flagged as processed
        //! until the dependency is processed as well, for most types
        //! of queries they will not even start before that
        Query *Dependency;
        //! Creates empty query
        Query();
        //! Destructor for query
        virtual ~Query();
        //! Returns true in case that query is processed
        virtual bool Processed();
        virtual void Process() {}
        virtual void Kill() {}
        virtual QString QueryTypeToString();
        virtual QString QueryTargetToString();
        virtual QString QueryStatusToString();
        //! If you inherit query you should allways call this from a signal that
        //! you receive when the query finish
        void ProcessCallback();
        //! Every query has own unique ID which can be used to work with them
        //! this function returns that
        unsigned int QueryID();

    private:
        //! Every query has own unique ID which can be used to work with them
        unsigned int ID;
        //! This is a last ID used by a constructor of a query
        static unsigned int LastID;
    };
}

#endif // QUERY_H
