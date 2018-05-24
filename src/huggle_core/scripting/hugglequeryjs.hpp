//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef HUGGLEQUERYJS_HPP
#define HUGGLEQUERYJS_HPP

#include "../definitions.hpp"

#include "genericjsclass.hpp"
#include "../collectable_smartptr.hpp"
#include <QVariant>
#include <QHash>
#include <QObject>
#include <QString>
#include <QJSEngine>

namespace Huggle
{
    class ApiQuery;
    class EditQuery;
    class WikiSite;
    class Query;

    class HuggleQueryJS : public GenericJSClass
    {
            Q_OBJECT
        public:
            HuggleQueryJS(Script *s);
            ~HuggleQueryJS();
            QHash<QString, QString> GetFunctions() { return functions; }
            Q_INVOKABLE QJSValue get_all_bytes_sent();
            Q_INVOKABLE QJSValue get_all_bytes_received();
            // Query - API
            Q_INVOKABLE int create_api_query(int type, QString site, QString parameters, bool using_post = false, bool auto_delete = false);
            Q_INVOKABLE bool register_api_success_callback(int query, QString callback);
            Q_INVOKABLE bool register_api_failure_callback(int query, QString callback);
            Q_INVOKABLE bool process_api_query(int query);
            Q_INVOKABLE bool kill_api_query(int query);
            Q_INVOKABLE QJSValue get_api_query_info(int query);
            Q_INVOKABLE bool destroy_api_query(int query);
            void ProcessAQSuccessCallback(Query *query);
            void ProcessAQFailureCallback(Query *query);
            // Query - edit
            Q_INVOKABLE int edit_page_append_text(QString page_name, QString text, QString summary, bool minor = false, bool auto_delete = true);
            Q_INVOKABLE int edit_page(QString page, QString text, QString summary, QString site_name, bool minor = false, QString base_timestamp = "", unsigned int section = 0, bool auto_delete = true);
            Q_INVOKABLE bool register_edit_success_callback(int query, QString callback);
            Q_INVOKABLE bool register_edit_failure_callback(int query, QString callback);
            Q_INVOKABLE bool kill_edit_query(int query);
            Q_INVOKABLE QJSValue get_edit_query_info(int query);
            Q_INVOKABLE bool destroy_edit_query(int query);
            void ProcessEQSuccessCallback(Query *query);
            void ProcessEQFailureCallback(Query *query);

        private:
            int getApiByPtr(ApiQuery *query);
            int getEQByPtr(EditQuery *query);
            void removeEditQueryById(int id);
            void removeApiQueryById(int id);
            QHash<int, Collectable_SmartPtr<ApiQuery>> apiQueries;
            QHash<int, Collectable_SmartPtr<EditQuery>> editQueries;
            QHash<int, QString> successCallbacks;
            QHash<int, QString> failureCallbacks;
            //! List of queries that can be auto deleted when finish
            QList<Query*> autoDeletes;
            QHash<QString, QString> functions;
            int lastQuery = 0;
    };
}

#endif // HUGGLEQUERYJS_HPP
