//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "querypool.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "syslog.hpp"
#include "wikiutil.hpp"

using namespace Huggle;

QueryPool *QueryPool::HugglePool = nullptr;

QueryPool::QueryPool()
{
    this->Processes = nullptr;
}

QueryPool::~QueryPool()
{
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
    while(this->RunningQueries.count() != 0)
    {
        this->RunningQueries.at(0)->UnregisterConsumer(HUGGLECONSUMER_QP);
        this->RunningQueries.removeAt(0);
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
    this->RunningQueries.append(item);
}

void QueryPool::PreProcessEdit(WikiEdit *edit)
{
    if (edit == nullptr)
        throw new Huggle::Exception("NULL edit", "void QueryPool::PreProcessEdit(WikiEdit *edit)");
    if (edit->Status == StatusProcessed)
        return;
    if (edit->User == nullptr)
        throw new Huggle::Exception("Edit user was NULL in Core::PreProcessEdit");
    if (edit->Bot)
        edit->User->SetBot(true);

    edit->EditMadeByHuggle = edit->Summary.contains(Configuration::HuggleConfiguration->ProjectConfig->EditSuffixOfHuggle);

    int x = 0;
    while (x < Configuration::HuggleConfiguration->ProjectConfig->Assisted.count())
    {
        if (edit->Summary.contains(Configuration::HuggleConfiguration->ProjectConfig->Assisted.at(x)))
        {
            edit->TrustworthEdit = true;
            break;
        }
        x++;
    }

    if (WikiUtil::IsRevert(edit->Summary))
    {
        edit->IsRevert = true;
        if (HuggleFeed::PrimaryFeedProvider != nullptr)
        {
            HuggleFeed::PrimaryFeedProvider->RvCounter++;
        }
        if (Configuration::HuggleConfiguration->UserConfig->DeleteEditsAfterRevert)
        {
            edit->RegisterConsumer(HUGGLECONSUMER_QP_UNCHECKED);
            this->UncheckedReverts.append(edit);
        }
    }
    edit->Status = StatusProcessed;
}

void QueryPool::PostProcessEdit(WikiEdit *edit)
{
    if (edit == nullptr)
    {
        throw new Exception("NULL edit in PostProcessEdit(WikiEdit *_e) is not a valid edit");
    }
    edit->RegisterConsumer(HUGGLECONSUMER_CORE_POSTPROCESS);
    edit->PostProcess();
    this->ProcessingEdits.append(edit);
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
                e->UnregisterConsumer(HUGGLECONSUMER_QP_MODS);
            } else
            {
                curr++;
            }
        }
    }
    curr = 0;
    if (this->RunningQueries.count() < 1)
        return;
    while (curr < this->RunningQueries.count())
    {
        Query *q = this->RunningQueries.at(curr);
        if (this->Processes != nullptr)
            this->Processes->UpdateQuery(q);
        if (q->IsProcessed())
        {
            this->RunningQueries.removeAt(curr);
            // this is pretty spamy :o
            Huggle::Syslog::HuggleLogs->DebugLog("Query finished with: " + q->Result->Data, 8);
            if (this->Processes != nullptr)
            {
                this->Processes->UpdateQuery(q);
                this->Processes->RemoveQuery(q);
            }
            q->UnregisterConsumer(HUGGLECONSUMER_QP);
        } else
        {
            curr++;
        }
    }
}

int QueryPool::RunningQueriesGetCount()
{
    return this->RunningQueries.count();
}

