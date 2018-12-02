//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "querypool.hpp"
#include <QtXml>
#include "configuration.hpp"
#include "editquery.hpp"
#include "editqueue.hpp"
#include "exception.hpp"
#include "apiqueryresult.hpp"
#include "hugglefeed.hpp"
#include "query.hpp"
#include "hooks.hpp"
#include "message.hpp"
#include "syslog.hpp"
#include "wikiedit.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"
#include "wikiutil.hpp"

using namespace Huggle;

QueryPool *QueryPool::HugglePool = nullptr;

QueryPool::QueryPool()
{

}

QueryPool::~QueryPool()
{
    while (this->RevertBuffer.count() != 0)
    {
        this->RevertBuffer.at(0)->UnregisterConsumer(HUGGLECONSUMER_QP_REVERTBUFFER);
        this->RevertBuffer.removeAt(0);
    }
    while (this->PendingMods.count() != 0)
    {
        this->PendingMods.at(0)->UnregisterConsumer(HUGGLECONSUMER_QP_MODS);
        this->PendingMods.removeAt(0);
    }
    while (this->ProcessingEdits.count() != 0)
    {
        this->ProcessingEdits.at(0)->UnregisterConsumer(HUGGLECONSUMER_CORE_POSTPROCESS);
        this->ProcessingEdits.removeAt(0);
    }
    while (this->PendingWatches.count())
    {
        this->PendingWatches.at(0)->UnregisterConsumer(HUGGLECONSUMER_QP_WATCHLIST);
        this->PendingWatches.removeAt(0);
    }
    while(this->runningQueries.count() != 0)
    {
        this->runningQueries.at(0)->UnregisterConsumer(HUGGLECONSUMER_QP);
        this->runningQueries.removeAt(0);
    }
    while (this->UncheckedReverts.count() != 0)
    {
        this->UncheckedReverts.at(0)->UnregisterConsumer(HUGGLECONSUMER_QP_UNCHECKED);
        this->UncheckedReverts.removeAt(0);
    }
}

void QueryPool::AppendQuery(Query *item)
{
    item->RegisterConsumer(HUGGLECONSUMER_QP);
    this->runningQueries.append(item);
}

void QueryPool::PreProcessEdit(WikiEdit *edit)
{
    if (edit == nullptr)
        throw new Huggle::NullPointerException("WikiEdit *edit", BOOST_CURRENT_FUNCTION);
    if (edit->Status == StatusProcessed)
        return;
    if (edit->Status == StatusPostProcessed)
        throw new Huggle::Exception("Pre process of edit that was already post processed", BOOST_CURRENT_FUNCTION);
    if (edit->User == nullptr)
        throw new Huggle::NullPointerException("edit->User", BOOST_CURRENT_FUNCTION);
    if (edit->Bot)
        edit->User->SetBot(true);

    edit->EditMadeByHuggle = edit->Summary.contains(hcfg->ProjectConfig->EditSuffixOfHuggle);

    int x = 0;
    while (x < Configuration::HuggleConfiguration->ProjectConfig->Assisted.count())
    {
        if (edit->Summary.contains(hcfg->ProjectConfig->Assisted.at(x)))
        {
            edit->TrustworthEdit = true;
            break;
        }
        x++;
    }

    if (WikiUtil::IsRevert(edit->Summary))
    {
        edit->IsRevert = true;
        if (edit->GetSite()->Provider != nullptr)
            edit->GetSite()->Provider->IncrementReverts();

        if (Configuration::HuggleConfiguration->UserConfig->DeleteEditsAfterRevert)
        {
            edit->RegisterConsumer(HUGGLECONSUMER_QP_UNCHECKED);
            this->UncheckedReverts.append(edit);
        }
    }

    if (hcfg->UserConfig->RemoveAfterTrustedEdit && edit->User->IsWhitelisted() && EditQueue::Primary)
        EditQueue::Primary->DeleteOlder(edit);

    edit->Status = StatusProcessed;
    Hooks::EditAfterPreProcess(edit);
}

void QueryPool::PostProcessEdit(WikiEdit *edit)
{
    if (edit == nullptr)
    {
        throw new Huggle::NullPointerException("local::WikiEdit *edit", BOOST_CURRENT_FUNCTION);
    }
    edit->RegisterConsumer(HUGGLECONSUMER_CORE_POSTPROCESS);
    edit->PostProcess();
    this->ProcessingEdits.append(edit);
}

void QueryPool::CheckQueries()
{
    foreach (ApiQuery *query, this->PendingWatches)
    {
        if (!query->IsProcessed())
            continue;
        if (query->IsFailed())
        {
            // Get the error code and check what the reason for this error was
            ApiQueryResultNode *error = query->GetApiQueryResult()->GetNode("error");
            if (error != nullptr)
            {
                QString code = error->GetAttribute("code");
                if (code == "badtoken")
                {
                    // We got logged out of mediawiki
                    hcfg->Logout(query->GetSite());
                    query->Suspend();
                    continue;
                }
            }
            Syslog::HuggleLogs->ErrorLog("Unable to (un)watchlist " + query->Target + " on " + query->GetSite()->Name + " because of: " + query->GetFailureReason());
        } else
        {
            //! \todo The error checks should be implemented to make sure it really did this
            if (query->GetAction() == ActionUnwatch)
                Syslog::HuggleLogs->Log("Successfuly unwatchlisted " + query->Target);
            else
                Syslog::HuggleLogs->Log("Successfuly watchlisted " + query->Target);
        }
        query->UnregisterConsumer(HUGGLECONSUMER_QP_WATCHLIST);
        this->PendingWatches.removeAll(query);
    }
    int curr = 0;
    if (this->PendingMods.count() > 0)
    {
        while (curr < this->PendingMods.count())
        {
            if (this->PendingMods.at(curr)->IsProcessed())
            {
                EditQuery *e = this->PendingMods.at(curr);
                this->PendingMods.removeAt(curr);
                e->UnregisterConsumer(HUGGLECONSUMER_QP_MODS);
            } else
            {
                curr++;
            }
        }
    }
    curr = 0;
    if (this->runningQueries.count() < 1)
        return;
    while (curr < this->runningQueries.count())
    {
        Query *q = this->runningQueries.at(curr);
        Hooks::QueryPool_Update(q);
        if (q->IsProcessed())
        {
#ifdef HUGGLE_METRICS
            this->registerQueryPerfTime(q);
#endif
            this->runningQueries.removeAt(curr);
            // this is pretty spamy :o
            HUGGLE_DEBUG("Query finished with: " + q->Result->Data, 8);
            Hooks::QueryPool_Update(q);
            Hooks::QueryPool_Remove(q);
            q->UnregisterConsumer(HUGGLECONSUMER_QP);
        } else
        {
            curr++;
        }
    }
}

int QueryPool::RunningQueriesGetCount()
{
    return this->runningQueries.count();
}

int QueryPool::GetRunningEditingQueries()
{
    int n = 0;
    foreach (Query *query, this->runningQueries)
    {
        if (query->Type == QueryApi && (dynamic_cast<ApiQuery*>(query)->EditingQuery))
        {
            n++;
        }
    }
    return n;
}

#ifdef HUGGLE_METRICS
qint64 QueryPool::GetAverageExecutionTime()
{
    if (!this->performanceInfo.count())
        return -1;

    qint64 sum = 0;

    // Calculate average value
    foreach (qint64 i, this->performanceInfo)
        sum += i;

    return sum / this->performanceInfo.count();
}

void QueryPool::registerQueryPerfTime(Query *item)
{
    if (item->Type != QueryApi)
        return;

    while(this->performanceInfo.size() > HUGGLE_STATISTICS_BLOCK_SIZE)
        this->performanceInfo.removeFirst();

    this->performanceInfo.append(item->ExecutionTime());
}
#endif

