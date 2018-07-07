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

QList<HuggleFeed*> HuggleFeed::providerList;

QList<HuggleFeed *> HuggleFeed::GetProviders()
{
    return HuggleFeed::providerList;
}

QList<HuggleFeed *> HuggleFeed::GetProvidersForSite(WikiSite *site)
{
    QList<HuggleFeed *> providers;
    foreach (HuggleFeed *provider, HuggleFeed::providerList)
    {
        if (provider->GetSite() == site)
            providers.append(provider);
    }
    return providers;
}

HuggleFeed *HuggleFeed::GetAlternativeFeedProvider(HuggleFeed *provider)
{
    QList<HuggleFeed*> providers = HuggleFeed::GetProvidersForSite(provider->GetSite());
    HuggleFeed *best_pr = nullptr;
    foreach (HuggleFeed *px, providers)
    {
        if (px->FeedPriority() <= provider->FeedPriority() && px->GetID() != provider->GetID())
        {
            if (!best_pr)
                best_pr = px;
            else if (best_pr->FeedPriority() < px->FeedPriority())
                best_pr = px;
        }
    }
    return best_pr;
}

HuggleFeed *HuggleFeed::GetProviderByID(WikiSite *site, int id)
{
    QList<HuggleFeed*> providers = HuggleFeed::GetProvidersForSite(site);
    foreach (HuggleFeed *provider, providers)
    {
        if (provider->GetID() == id)
            return provider;
    }
    return nullptr;
}

unsigned long long HuggleFeed::GetTotalBytesRcvd()
{
    unsigned long long result = 0;
    foreach (HuggleFeed *feed, HuggleFeed::providerList)
        result += feed->GetBytesReceived();
    return result;
}

unsigned long long HuggleFeed::GetTotalBytesSent()
{
    unsigned long long result = 0;
    foreach (HuggleFeed *feed, HuggleFeed::providerList)
        result += feed->GetBytesSent();
    return result;
}

HuggleFeed::HuggleFeed(WikiSite *site) : MediaWikiObject(site)
{
    this->statisticsMutex = new QMutex(QMutex::Recursive);
    this->statisticsBlocks.append(new StatisticsBlock());
    this->editCounter = 0;
    this->rvCounter = 0;
    this->startupTime = QDateTime::currentDateTime();
    providerList.append(this);
}

HuggleFeed::~HuggleFeed()
{
    this->statisticsMutex->lock();
    while (this->statisticsBlocks.count())
    {
        delete this->statisticsBlocks.at(0);
        this->statisticsBlocks.removeAt(0);
    }
    this->statisticsMutex->unlock();
    if (providerList.contains(this))
        providerList.removeOne(this);
    delete this->statisticsMutex;
}

QString HuggleFeed::GetError()
{
    return "Unknown error";
}

double HuggleFeed::GetRevertsPerMinute()
{
    if (this->statisticsBlocks.count() < 1)
        throw new Huggle::Exception("Invalid number of statistics blocks", BOOST_CURRENT_FUNCTION);
    // first we need to get an uptime
    double uptime = ((double)this->statisticsBlocks.at(0)->Uptime.secsTo(QDateTime::currentDateTime())) / 60;
    if (uptime == 0)
        return 0;
    // now we need to get a number of all reverts for latest blocks
    this->statisticsMutex->lock();
    double reverts = 0;
    foreach (StatisticsBlock *ptr, this->statisticsBlocks)
        reverts += ptr->Reverts;
    this->statisticsMutex->unlock();
    // now we know how many reverts per minute we have
    return reverts / uptime;
}

double HuggleFeed::GetEditsPerMinute()
{
    this->rotateStats();
    if (this->statisticsBlocks.count() < 1)
        throw new Huggle::Exception("Invalid number of statistics blocks", BOOST_CURRENT_FUNCTION);
    // first we need to get an uptime
    double uptime = ((double)this->statisticsBlocks.at(0)->Uptime.secsTo(QDateTime::currentDateTime())) / 60;
    if (uptime == 0)
        return 0;
    this->statisticsMutex->lock();
    double edits = 0;
    foreach (StatisticsBlock *ptr, this->statisticsBlocks)
        edits += ptr->Edits;
    this->statisticsMutex->unlock();
    // now we know how many edits per minute we have
    return edits / uptime;
}

void HuggleFeed::IncrementEdits()
{
    if (!this->IsWorking())
        return;
    this->editCounter++;
    this->getLatestStatisticsBlock()->Edits++;
}

void HuggleFeed::IncrementReverts()
{
    if (!this->IsWorking())
        return;
    this->rvCounter++;
    this->getLatestStatisticsBlock()->Reverts++;
}

int HuggleFeed::FeedPriority()
{
    return 0;
}

double HuggleFeed::GetUptime()
{
    return (double)this->startupTime.secsTo(QDateTime::currentDateTime());
}

void HuggleFeed::rotateStats()
{
    if (this->statisticsBlocks.count() < 2)
        return;

    this->statisticsMutex->lock();

    if (this->statisticsBlocks.at(0)->Uptime.secsTo(QDateTime::currentDateTime()) > HUGGLE_STATISTICS_LIFETIME)
    {
        // since this can get in a race condition we need to first remove it and then delete it
        StatisticsBlock *b = this->statisticsBlocks.at(0);
        this->statisticsBlocks.removeAt(0);
        delete b;
    }

    this->statisticsMutex->unlock();
}

StatisticsBlock *HuggleFeed::getLatestStatisticsBlock()
{
    this->statisticsMutex->lock();
    if (!this->statisticsBlocks.count())
        this->statisticsBlocks.append(new StatisticsBlock());
    if (this->statisticsBlocks.last()->Uptime.secsTo(QDateTime::currentDateTime()) > HUGGLE_STATISTICS_BLOCK_SIZE)
        this->statisticsBlocks.append(new StatisticsBlock());
    this->statisticsMutex->unlock();
    return this->statisticsBlocks.last();
}

StatisticsBlock::StatisticsBlock()
{
    this->Uptime = QDateTime::currentDateTime();
    this->Edits = 0;
    this->Reverts = 0;
}
