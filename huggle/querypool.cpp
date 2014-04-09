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

