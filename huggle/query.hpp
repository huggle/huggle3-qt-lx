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

#include "definitions.hpp"
// now we need to ensure that python is included first
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QNetworkAccessManager>
#include "historyitem.hpp"
#include "queryresult.hpp"
#include "collectable.hpp"

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
    class Query : public Collectable
    {
        public:
            //! We need to have a shared manager for all queries
            //! so that sessions work in wiki
            static QNetworkAccessManager *NetworkManager;

            //! Creates empty query
            Query();
            //! Destructor for query
            virtual ~Query();
            //! Returns true in case that query is processed
            virtual bool IsProcessed();
            //! Execute query

            //! This is a main() of every query, your implementation goes here
            virtual void Process() {}
            //! Terminates a query

            //! In case it's not running nothing happens, in case query is currently running
            //! it should be immediately stopped and error result should be generated

            //! This is only a virtual interface implemented in Query which does nothing by default
            //! it is necessary for every query to implement this for it to work properly
            virtual void Kill() {}
            //! Convert a type of this query to a string
            virtual QString QueryTypeToString();
            //! Return a target of a query

            //! Target is either explicitly defined abstract identifier that is used for statistical
            //! purposes, or it is provided by query itself, based on a type of that query
            //! typical example would be a page that is affected by ApiQuery
            virtual QString QueryTargetToString();
            virtual QString QueryStatusToString();
            //! If you inherit query you should allways call this from a signal that
            //! you receive when the query finish
            void ProcessCallback();
            //! Every query has own unique ID which can be used to work with them
            //! this function returns that
            unsigned int QueryID();
            bool IsFailed();
            //! Result of query, see documentation of QueryResult for more
            QueryResult *Result = nullptr;
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
            Callback callback = nullptr;
            //! This is a pointer to object returned by your callback function
            void* CallbackResult = nullptr;
            bool RetryOnTimeoutFailure;
            QDateTime StartTime;
            int Timeout;
            //! Query doesn't have internal data displayed in debug log, this is usefull
            //! when you are working with passwords in parameters
            bool HiddenQuery;
            //! History item
            HistoryItem *HI = nullptr;
            //! Dependency for query

            //! If you put anything in here, it either must be NULL or query
            //! that is processed. The query will not be flagged as processed
            //! until the dependency is processed as well, for most types
            //! of queries they will not even start before that
            Query *Dependency = nullptr;

        private:
            //! Every query has own unique ID which can be used to work with them
            unsigned int ID;
            //! This is a last ID used by a constructor of a query
            static unsigned int LastID;
            //! When a query fail and retry this is changed to true so that it doesn't endlessly restart
            bool Repeated;
    };

    inline QString Query::QueryTargetToString()
    {
        return "Invalid target";
    }

    inline unsigned int Query::QueryID()
    {
        return this->ID;
    }
}

#endif // QUERY_H
