//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "querypool.hpp"

using namespace Huggle;

QueryPool *QueryPool::HugglePool = NULL;

QueryPool::QueryPool()
{
    this->Processes = NULL;
}

void QueryPool::AppendQuery(Query *item)
{
    item->RegisterConsumer(HUGGLECONSUMER_QP);
    this->RunningQueries.append(item);
}

void QueryPool::PreProcessEdit(WikiEdit *_e)
{
    if (_e == NULL)
    {
        throw new Exception("NULL edit");
    }

    if (_e->Status == StatusProcessed)
    {
        return;
    }

    if (_e->User == NULL)
    {
        throw new Exception("Edit user was NULL in Core::PreProcessEdit");
    }

    if (_e->Bot)
    {
        _e->User->SetBot(true);
    }

    _e->EditMadeByHuggle = _e->Summary.contains(Configuration::HuggleConfiguration->ProjectConfig_EditSuffixOfHuggle);

    int x = 0;
    while (x < Configuration::HuggleConfiguration->ProjectConfig_Assisted.count())
    {
        if (_e->Summary.contains(Configuration::HuggleConfiguration->ProjectConfig_Assisted.at(x)))
        {
            _e->TrustworthEdit = true;
            break;
        }
        x++;
    }

    if (WikiUtil::IsRevert(_e->Summary))
    {
        _e->IsRevert = true;
        if (HuggleFeed::PrimaryFeedProvider != NULL)
        {
            HuggleFeed::PrimaryFeedProvider->RvCounter++;
        }
        if (Configuration::HuggleConfiguration->UserConfig_DeleteEditsAfterRevert)
        {
            _e->RegisterConsumer("UncheckedReverts");
            this->UncheckedReverts.append(_e);
        }
    }

    _e->Status = StatusProcessed;
}

void QueryPool::PostProcessEdit(WikiEdit *_e)
{
    if (_e == NULL)
    {
        throw new Exception("NULL edit in PostProcessEdit(WikiEdit *_e) is not a valid edit");
    }
    _e->RegisterConsumer(HUGGLECONSUMER_CORE_POSTPROCESS);
    _e->UnregisterConsumer(HUGGLECONSUMER_WIKIEDIT);
    _e->PostProcess();
    this->ProcessingEdits.append(_e);
}

void QueryPool::CheckQueries()
{
    int curr = 0;
    if (this->PendingMods.count() > 0)
    {
        while (curr < this->PendingMods.count())
        {
            if (this->PendingMods.at(curr)->IsProcessed())
            {
                EditQuery *e = this->PendingMods.at(curr);
                this->PendingMods.removeAt(curr);
                e->UnregisterConsumer("WikiUtil::EditPage");
            } else
            {
                curr++;
            }
        }
    }
    curr = 0;
    if (this->RunningQueries.count() < 1)
    {
        return;
    }
    QList<Query*> Finished;
    while (curr < this->RunningQueries.count())
    {
        Query *q = this->RunningQueries.at(curr);
        if (this->Processes != NULL) { this->Processes->UpdateQuery(q); }
        if (q->IsProcessed())
        {
            Finished.append(q);
            // this is pretty spamy :o
            Huggle::Syslog::HuggleLogs->DebugLog("Query finished with: " + q->Result->Data, 8);
            if (this->Processes != NULL)
            {
                this->Processes->UpdateQuery(q);
                this->Processes->RemoveQuery(q);
            }
        }
        curr++;
    }
    curr = 0;
    while (curr < Finished.count())
    {
        Query *item = Finished.at(curr);
        this->RunningQueries.removeOne(item);
        item->Lock();
        item->UnregisterConsumer(HUGGLECONSUMER_QP);
        item->SafeDelete();
        curr++;
    }
}

int QueryPool::RunningQueriesGetCount()
{
    return this->RunningQueries.count();
}

