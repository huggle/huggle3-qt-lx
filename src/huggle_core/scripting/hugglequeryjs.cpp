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
#include "../querypool.hpp"
#include "../wikiedit.hpp"
#include "../wikipage.hpp"
#include "../wikiutil.hpp"
#include "../wikisite.hpp"

using namespace Huggle;

HuggleQueryJS::HuggleQueryJS(Script *s) : GenericJSClass(s)
{
    this->functions.insert("register_api_failure_callback", "(int query, string callback): registers function name to call on failure of query");
    this->functions.insert("register_api_success_callback", "(int query, string callback): registers function name to call on success of query");
    this->functions.insert("get_all_bytes_received", "(): (int) returns approximate bytes received by all queries");
    this->functions.insert("get_all_bytes_sent", "(): (int) get approximate bytes sent to all queries");
    this->functions.insert("create_api_query", "(int type, string site, string parameters, [bool using_post = false], [bool auto_delete = false]): (int) creates new API query");
    this->functions.insert("process_api_query", "(int query): (bool) process query");
    this->functions.insert("kill_api_query", "(int query): (bool) kills the API query with given ID, returns false on failure");
    this->functions.insert("get_api_query_info", "(int query): (returns hash) returns info about API query");
    this->functions.insert("destroy_api_query", "(int query): (bool) delete API query from memory");
    this->functions.insert("edit_page", "(string page, string text, string summary, string site_name, bool minor, [string base_timestamp], [unsigned int section = 0], [bool auto_delete = true])");
    this->functions.insert("edit_page_append_text", "(string page_name, string text, string summary, [bool minor = false], [bool auto_delete = true]): (int) appends text to a wiki page");
    this->functions.insert("kill_edit_query", "(int query): (bool) kills running edit query");
    this->functions.insert("get_edit_query_info", "(int query): (hash) return info about edit query");
    this->functions.insert("destroy_edit_query", "(int query): (bool) remove edit query from memory");
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
    (reinterpret_cast<HuggleQueryJS*>(qr->CallbackOwner))->ProcessAQFailureCallback(qr);
    qr->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
}

static void apisuccess(Query *qr)
{
    (reinterpret_cast<HuggleQueryJS*>(qr->CallbackOwner))->ProcessAQSuccessCallback(qr);
    qr->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
}

int HuggleQueryJS::create_api_query(int type, QString site, QString parameters, bool using_post, bool auto_delete)
{
    if (type < 0 || type > 17)
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
    int query_id = this->lastQuery++;
    Collectable_SmartPtr<ApiQuery> query = new ApiQuery(action, s);
    query->CallbackOwner = this;
    query->SuccessCallback = reinterpret_cast<Callback>(apisuccess);
    query->FailureCallback = reinterpret_cast<Callback>(apifailed);
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

static void editfailed(Query *qr)
{
    (reinterpret_cast<HuggleQueryJS*>(qr->CallbackOwner))->ProcessEQFailureCallback(qr);
    qr->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
}

static void editsuccess(Query *qr)
{
    (reinterpret_cast<HuggleQueryJS*>(qr->CallbackOwner))->ProcessEQSuccessCallback(qr);
    qr->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
}

int HuggleQueryJS::edit_page_append_text(QString page_name, QString text, QString summary, bool minor, bool auto_delete)
{
    int query_id = this->lastQuery++;
    Collectable_SmartPtr<EditQuery> edit = WikiUtil::AppendTextToPage(page_name, text, summary, minor);
    QueryPool::HugglePool->AppendQuery(edit);
    this->editQueries.insert(query_id, edit);
    edit->CallbackOwner = this;
    edit->SuccessCallback = reinterpret_cast<Callback>(editsuccess);
    edit->FailureCallback = reinterpret_cast<Callback>(editfailed);
    if (auto_delete)
        this->autoDeletes.append(edit.GetPtr());
    return query_id;
}

int HuggleQueryJS::edit_page(QString page, QString text, QString summary, QString site_name, bool minor, QString base_timestamp, unsigned int section, bool auto_delete)
{
    WikiSite *site = nullptr;
    foreach (WikiSite *s, hcfg->Projects)
    {
        if (site_name == s->Name)
        {
            site = s;
            break;
        }
    }
    if (site == nullptr)
    {
        HUGGLE_ERROR(this->script->GetName() + ": edit_page(...): invalid site");
        return -1;
    }
    int query_id = this->lastQuery++;
    Collectable_SmartPtr<EditQuery> edit = WikiUtil::EditPage(site, page, text, summary, minor, base_timestamp, section);
    QueryPool::HugglePool->AppendQuery(edit);
    this->editQueries.insert(query_id, edit);
    edit->CallbackOwner = this;
    edit->SuccessCallback = reinterpret_cast<Callback>(editsuccess);
    edit->FailureCallback = reinterpret_cast<Callback>(editfailed);
    if (auto_delete)
        this->autoDeletes.append(edit.GetPtr());
    return query_id;
}

bool HuggleQueryJS::register_edit_success_callback(int query, QString callback)
{
    if (!this->editQueries.contains(query))
        return false;

    if (this->successCallbacks.contains(query))
        this->successCallbacks[query] = callback;
    else
        this->successCallbacks.insert(query, callback);
    return true;
}

bool HuggleQueryJS::register_edit_failure_callback(int query, QString callback)
{
    if (!this->editQueries.contains(query))
        return false;

    if (this->failureCallbacks.contains(query))
        this->failureCallbacks[query] = callback;
    else
        this->failureCallbacks.insert(query, callback);
    return true;
}

bool HuggleQueryJS::kill_edit_query(int query)
{
    if (!this->editQueries.contains(query))
        return false;

    this->editQueries[query]->Kill();
    return true;
}

QJSValue HuggleQueryJS::get_edit_query_info(int query)
{
    if (!this->editQueries.contains(query))
        return QJSValue(QJSValue::SpecialValue::NullValue);
    return JSMarshallingHelper::FromEditQuery(this->editQueries[query].GetPtr(), this->script->GetEngine());
}

bool HuggleQueryJS::destroy_edit_query(int query)
{
    if (!this->editQueries.contains(query))
        return false;

    this->removeEditQueryById(query);
    return true;
}

void HuggleQueryJS::ProcessEQSuccessCallback(Query *query)
{
    int id = this->getEQByPtr(dynamic_cast<EditQuery*>(query));
    if (id < 0)
        return;
    if (this->successCallbacks.contains(id))
        this->GetScript()->ExecuteFunction(this->successCallbacks[id]);
    if (this->autoDeletes.contains(query))
        this->removeEditQueryById(id);
}

void HuggleQueryJS::ProcessEQFailureCallback(Query *query)
{
    int id = this->getEQByPtr(dynamic_cast<EditQuery*>(query));
    if (id < 0)
        return;
    if (this->failureCallbacks.contains(id))
        this->GetScript()->ExecuteFunction(this->failureCallbacks[id]);
    if (this->autoDeletes.contains(query))
        this->removeEditQueryById(id);
}

void HuggleQueryJS::ProcessAQSuccessCallback(Query *query)
{
    int id = this->getApiByPtr(dynamic_cast<ApiQuery*>(query));
    if (id < 0)
        return;
    if (this->successCallbacks.contains(id))
        this->GetScript()->ExecuteFunction(this->successCallbacks[id]);
    if (this->autoDeletes.contains(query))
        this->removeApiQueryById(id);
}

void HuggleQueryJS::ProcessAQFailureCallback(Query *query)
{
    int id = this->getApiByPtr(dynamic_cast<ApiQuery*>(query));
    if (id < 0)
        return;
    if (this->failureCallbacks.contains(id))
        this->GetScript()->ExecuteFunction(this->failureCallbacks[id]);
    if (this->autoDeletes.contains(query))
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

int HuggleQueryJS::getEQByPtr(EditQuery *query)
{
    foreach (int id, this->editQueries.keys())
    {
        if (this->editQueries[id].GetPtr() == query)
            return id;
    }
    return -1;
}

void HuggleQueryJS::removeEditQueryById(int id)
{
    if (!this->editQueries.contains(id))
        return;

    this->autoDeletes.removeAll(this->editQueries[id].GetPtr());
    this->editQueries.remove(id);
    this->failureCallbacks.remove(id);
    this->successCallbacks.remove(id);
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
