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
    //! other queries are derived from
    class Query : public QObject
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
        bool IsManaged();
        virtual QString QueryTypeToString();
        virtual QString QueryTargetToString();
        virtual QString QueryStatusToString();
        //! If you inherit query you should allways call this from a signal that
        //! you receive when the query finish
        void ProcessCallback();
        //! Use this if you are not sure if you can delete this object in this moment
        virtual bool SafeDelete(bool forced = false);
        /*!
         * \brief Registers a consumer
         *
         * This function will store a string which prevent the object from being removed
         * by QueryGC
         * \param consumer String that lock the object
         */
        void RegisterConsumer(QString consumer);
        /*!
         * \brief This function will remove a string which prevent the object from being removed
         * \param consumer Unique string that unlock the object
         */
        void UnregisterConsumer(QString consumer);
        //! Print some debug information regarding the query into system log
        QString DebugQgc();
        //! Every query has own unique ID which can be used to work with them
        //! this function returns that
        unsigned int QueryID();

    private:
        //! Internal variable that contains a cache whether object is managed
        bool Managed;
        //! Some queries are needed for dependency setup, so we need to delete them
        //! later once the dependency is processed
        QStringList Consumers;
        //! Every query has own unique ID which can be used to work with them
        unsigned int ID;
        //! This is a last ID used by a constructor of a query
        static unsigned int LastID;
    };
}

#endif // QUERY_H
