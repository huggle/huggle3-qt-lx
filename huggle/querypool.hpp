//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef QUERYPOOL_HPP
#define QUERYPOOL_HPP

#include "definitions.hpp"
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QList>
#include "query.hpp"
#include "processlist.hpp"
#include "syslog.hpp"
#include "editquery.hpp"

namespace Huggle
{
    class ApiQuery;
    class EditQuery;
    class ProcessList;
    class Query;

    class QueryPool
    {
        public:
            static QueryPool *HugglePool;
            QueryPool();
            /*!
             * \brief Insert a query to internal list of running queries, so that they can be watched
             * This will insert it to a process list in main form
             * \param item Query that is about to be inserted to list of running queries
             */
            void AppendQuery(Query* item);
            void CheckQueries();
            int RunningQueriesGetCount();
            ProcessList *Processes;
            //! Pending changes
            QList<EditQuery*> PendingMods;
        private:
            //! List of all running queries
            QList<Query*> RunningQueries;
    };
}

#endif // QUERYPOOL_HPP
