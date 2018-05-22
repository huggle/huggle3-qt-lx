//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "hugglequeryjs.hpp"
#include "jsmarshallinghelper.hpp"
#include "script.hpp"
#include "../apiquery.hpp"
#include "../configuration.hpp"
#include "../syslog.hpp"
#include "../wikisite.hpp"

using namespace Huggle;

HuggleQueryJS::HuggleQueryJS(Script *s) : GenericJSClass(s)
{
    this->functions.insert("register_api_failure_callback", "(int query, string callback): registers function name to call on failure of query");
    this->functions.insert("register_api_success_callback", "(int query, string callback): registers function name to call on success of query");
    this->functions.insert("int get_all_bytes_received", "(): return approximate bytes received by all queries");
    this->functions.insert("int get_all_bytes_sent", "(): get approximate bytes sent to all queries");
    this->functions.insert("int create_api_query", "(int type, string site, string parameters, [bool using_post = false], [bool auto_delete = false]): creates new API query");
    this->functions.insert("bool process_api_query", "(int query): process query");
    this->functions.insert("bool kill_api_query", "(int query)");
    this->functions.insert("hash get_api_query_info", "(int query)");
    this->functions.insert("bool destroy_api_query", "(int query)");
}

HuggleQueryJS::~HuggleQueryJS()
{
    this->autoDeletes.clear();
}

QJSValue HuggleQueryJS::get_all_bytes_sent()
{
    return QJSValue(static_cast<double>(Query::GetBytesSentSinceStartup()));
}

QJSValue HuggleQueryJS::get_all_bytes_received()
{
    return QJSValue(static_cast<double>(Query::GetBytesReceivedSinceStartup()));
}

static void apifailed(Query *qr)
{
    ((HuggleQueryJS*)qr->CallbackOwner)->ProcessFailureCallback(qr);
    qr->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
}

static void apisuccess(Query *qr)
{
    ((HuggleQueryJS*)qr->CallbackOwner)->ProcessSuccessCallback(qr);
    qr->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
}

int HuggleQueryJS::create_api_query(int type, QString site, QString parameters, bool using_post, bool auto_delete)
{
    if (type > 0 || type > 17)
    {
        HUGGLE_ERROR(this->script->GetName() + ": create_api_query(...): invalid query type");
        return -1;
    }
    WikiSite *s = nullptr;
    foreach (WikiSite *sx, hcfg->Projects)
    {
        if (sx->Name == site)
        {
            s = sx;
            break;
        }
    }
    if (!s)
    {
        HUGGLE_ERROR(this->script->GetName() + ": create_api_query(...): invalid site");
        return -1;
    }
    Action action = static_cast<Action>(type);
    int query_id = this->lastAPI++;
    Collectable_SmartPtr<ApiQuery> query = new ApiQuery(action, s);
    query->CallbackOwner = this;
    query->SuccessCallback = (Callback)apisuccess;
    query->FailureCallback = (Callback)apifailed;
    query->UsingPOST = using_post;
    query->Parameters = parameters;
    if (auto_delete)
        this->autoDeletes.append(query.GetPtr());
    this->apiQueries.insert(query_id, query);
    return query_id;
}

bool HuggleQueryJS::register_api_success_callback(int query, QString callback)
{
    if (!this->apiQueries.contains(query))
        return false;

    if (this->successCallbacks.contains(query))
        this->successCallbacks[query] = callback;
    else
        this->successCallbacks.insert(query, callback);
    return true;
}

bool HuggleQueryJS::register_api_failure_callback(int query, QString callback)
{
    if (!this->apiQueries.contains(query))
        return false;

    if (this->failureCallbacks.contains(query))
        this->failureCallbacks[query] = callback;
    else
        this->failureCallbacks.insert(query, callback);
    return true;
}

bool HuggleQueryJS::process_api_query(int query)
{
    if (!this->apiQueries.contains(query))
        return false;

    this->apiQueries[query]->Process();
    return true;
}

bool HuggleQueryJS::kill_api_query(int query)
{
    if (!this->apiQueries.contains(query))
        return false;

    this->apiQueries[query]->Kill();
    return true;
}

QJSValue HuggleQueryJS::get_api_query_info(int query)
{
    if (!this->apiQueries.contains(query))
        return QJSValue(QJSValue::SpecialValue::NullValue);
    return JSMarshallingHelper::FromApiQuery(this->apiQueries[query].GetPtr(), this->script->GetEngine());
}

bool HuggleQueryJS::destroy_api_query(int query)
{
    if (!this->apiQueries.contains(query))
        return false;

    this->removeApiQueryById(query);
    return true;
}

void HuggleQueryJS::ProcessSuccessCallback(Query *query)
{
    int id = this->getApiByPtr((ApiQuery*)query);
    if (id < 0)
        return;
    if (this->successCallbacks.contains(id))
        this->GetScript()->ExecuteFunction(this->successCallbacks[id]);
    if (this->autoDeletes.contains((ApiQuery*)query))
        this->removeApiQueryById(id);
}

void HuggleQueryJS::ProcessFailureCallback(Query *query)
{
    int id = this->getApiByPtr((ApiQuery*)query);
    if (id < 0)
        return;
    if (this->failureCallbacks.contains(id))
        this->GetScript()->ExecuteFunction(this->failureCallbacks[id]);
    if (this->autoDeletes.contains((ApiQuery*)query))
        this->removeApiQueryById(id);
}

int HuggleQueryJS::getApiByPtr(ApiQuery *query)
{
    foreach (int id, this->apiQueries.keys())
    {
        if (this->apiQueries[id].GetPtr() == query)
            return id;
    }
    return -1;
}

void HuggleQueryJS::removeApiQueryById(int id)
{
    if (!this->apiQueries.contains(id))
        return;

    this->autoDeletes.removeAll(this->apiQueries[id].GetPtr());
    this->apiQueries.remove(id);
    this->failureCallbacks.remove(id);
    this->successCallbacks.remove(id);
}
