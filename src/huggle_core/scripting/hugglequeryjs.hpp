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
            Q_INVOKABLE int create_api_query(int type, QString site, QString parameters, bool using_post = false, bool auto_delete = false);
            Q_INVOKABLE bool register_api_success_callback(int query, QString callback);
            Q_INVOKABLE bool register_api_failure_callback(int query, QString callback);
            Q_INVOKABLE bool process_api_query(int query);
            Q_INVOKABLE bool kill_api_query(int query);
            Q_INVOKABLE QJSValue get_api_query_info(int query);
            Q_INVOKABLE bool destroy_api_query(int query);
            void ProcessSuccessCallback(Query *query);
            void ProcessFailureCallback(Query *query);

        private:
            int getApiByPtr(ApiQuery *query);
            void removeApiQueryById(int id);
            QHash<int, Collectable_SmartPtr<ApiQuery>> apiQueries;
            QHash<int, QString> successCallbacks;
            QHash<int, QString> failureCallbacks;
            QList<ApiQuery*> autoDeletes;
            QHash<QString, QString> functions;
            int lastAPI = 0;
    };
}

#endif // HUGGLEQUERYJS_HPP
