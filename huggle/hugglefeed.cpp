//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "configuration.hpp"
#include "hugglefeed.hpp"
#include "exception.hpp"
#include "wikisite.hpp"

using namespace Huggle;

QList<HuggleFeed*> HuggleFeed::Providers;

HuggleFeed::HuggleFeed(WikiSite *site)
{
    this->mutex = new QMutex(QMutex::Recursive);
    this->Site = site;
    this->StatisticsBlocks.append(new StatisticsBlock());
    this->EditCounter = 0;
    this->RvCounter = 0;
    this->UptimeDate = QDateTime::currentDateTime();
    Providers.append(this);
}

HuggleFeed::~HuggleFeed()
{
    this->mutex->lock();
    while (this->StatisticsBlocks.count())
    {
        delete this->StatisticsBlocks.at(0);
        this->StatisticsBlocks.removeAt(0);
    }
    this->mutex->unlock();
    if (Providers.contains(this))
        Providers.removeOne(this);
    delete this->mutex;
}

double HuggleFeed::GetRevertsPerMinute()
{
    if (this->StatisticsBlocks.count() < 1)
        throw new Huggle::Exception("Invalid number of statistics blocks", BOOST_CURRENT_FUNCTION);
    // first we need to get an uptime
    double uptime = ((double)this->StatisticsBlocks.at(0)->Uptime.secsTo(QDateTime::currentDateTime())) / 60;
    if (uptime == 0)
        return 0;
    // now we need to get a number of all reverts for latest blocks
    this->mutex->lock();
    double reverts = 0;
    foreach (StatisticsBlock *ptr, this->StatisticsBlocks)
        reverts += ptr->Reverts;
    this->mutex->unlock();
    // now we know how many reverts per minute we have
    return reverts / uptime;
}

double HuggleFeed::GetEditsPerMinute()
{
    this->RotateStats();
    if (this->StatisticsBlocks.count() < 1)
        throw new Huggle::Exception("Invalid number of statistics blocks", BOOST_CURRENT_FUNCTION);
    // first we need to get an uptime
    double uptime = ((double)this->StatisticsBlocks.at(0)->Uptime.secsTo(QDateTime::currentDateTime())) / 60;
    if (uptime == 0)
        return 0;
    this->mutex->lock();
    double edits = 0;
    foreach (StatisticsBlock *ptr, this->StatisticsBlocks)
        edits += ptr->Edits;
    this->mutex->unlock();
    // now we know how many edits per minute we have
    return edits / uptime;
}

void HuggleFeed::IncrementEdits()
{
    if (!this->IsWorking())
        return;
    this->EditCounter++;
    this->GetLatestStatisticsBlock()->Edits++;
}

void HuggleFeed::IncrementReverts()
{
    if (!this->IsWorking())
        return;
    this->RvCounter++;
    this->GetLatestStatisticsBlock()->Reverts++;
}

double HuggleFeed::GetUptime()
{
    return (double)this->UptimeDate.secsTo(QDateTime::currentDateTime());
}

void HuggleFeed::RotateStats()
{
    if (this->StatisticsBlocks.count() < 2)
        return;

    this->mutex->lock();

    if (this->StatisticsBlocks.at(0)->Uptime.secsTo(QDateTime::currentDateTime()) > HUGGLE_STATISTICS_LIFETIME)
    {
        // since this can get in a race condition we need to first remove it and then delete it
        StatisticsBlock *b = this->StatisticsBlocks.at(0);
        this->StatisticsBlocks.removeAt(0);
        delete b;
    }

    this->mutex->unlock();
}

StatisticsBlock *HuggleFeed::GetLatestStatisticsBlock()
{
    this->mutex->lock();
    if (!this->StatisticsBlocks.count())
        this->StatisticsBlocks.append(new StatisticsBlock());
    if (this->StatisticsBlocks.last()->Uptime.secsTo(QDateTime::currentDateTime()) > HUGGLE_STATISTICS_BLOCK_SIZE)
        this->StatisticsBlocks.append(new StatisticsBlock());
    this->mutex->unlock();
    return this->StatisticsBlocks.last();
}

StatisticsBlock::StatisticsBlock()
{
    this->Uptime = QDateTime::currentDateTime();
    this->Edits = 0;
    this->Reverts = 0;
}
