//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEFEED_H
#define HUGGLEFEED_H

#include "definitions.hpp"

#include <QDateTime>
#include <QList>
#include <QMutex>
#include "mediawikiobject.hpp"

#define HUGGLE_FEED_PROVIDER_IRC          0
#define HUGGLE_FEED_PROVIDER_WIKI         1
#define HUGGLE_FEED_PROVIDER_XMLRPC       2

namespace Huggle
{
    class HuggleQueueFilter;
    class WikiEdit;
    class WikiSite;

    class HUGGLE_EX_CORE StatisticsBlock
    {
        public:
            StatisticsBlock();
            QDateTime Uptime;
            double Edits;
            double Reverts;
    };

    //! Feed provider stub class every provider must be derived from this one
    class HUGGLE_EX_CORE HuggleFeed : public MediaWikiObject
    {
        public:
            static QList<HuggleFeed*> GetProviders();
            static QList<HuggleFeed*> GetProvidersForSite(WikiSite *site);
            static HuggleFeed *GetAlternativeFeedProvider(HuggleFeed *provider);
            static HuggleFeed *GetProviderByID(WikiSite *site, int id);
            static unsigned long long GetTotalBytesRcvd();
            static unsigned long long GetTotalBytesSent();

            HuggleFeed(WikiSite *site);
            virtual ~HuggleFeed();
            //! Return true if this feed is operational or not
            virtual bool IsWorking() { return false; }
            //! Restart the feed engine
            virtual bool Restart() {return false;}
            //! Stop the feed engine
            virtual void Stop() {}
            //! Start the feed engine
            virtual bool Start() { return false; }
            //! This is useful to stop parsing edits from irc and like in case that queue is full
            virtual void Pause() {}
            //! Resume edit parsing
            virtual void Resume() {}
            //! Check if feed is containing some edits in buffer
            virtual bool ContainsEdit() { return false; }
            virtual bool IsPaused() { return false; }
            virtual QString GetError();
            virtual int GetID()=0;
            //! If provider is not to be automatically inserted to a list of providers
            //! Builtin providers have hardcoded menus, so they are ignored.
            virtual bool IsBuiltin() { return true; }
            //! Returns true in case that a provider is stopped and can be safely deleted

            //! This is useful in case we are running some background threads and we need to
            //! wait for them to finish before we can delete the object
            virtual bool IsStopped() { return true; }
            virtual double GetRevertsPerMinute();
            virtual double GetEditsPerMinute();
            //! Return a last edit from cache or NULL
            virtual WikiEdit *RetrieveEdit() { return nullptr; }
            virtual QString ToString() = 0;
            virtual void IncrementEdits();
            virtual void IncrementReverts();
            //! Used to find alternative feed provider in case that current one fails
            virtual int FeedPriority();
            qint64 GetUptime();
            virtual unsigned long long GetBytesReceived()=0;
            virtual unsigned long long GetBytesSent()=0;
            HuggleQueueFilter *Filter;
        protected:
            static QList<HuggleFeed*> providerList;
            void rotateStats();
            StatisticsBlock *getLatestStatisticsBlock();
            //! Number of edits made since you logged in
            double editCounter;
            //! Number of reverts made since you logged in
            double rvCounter;
            QMutex *statisticsMutex;
            QDateTime startupTime;
            QList<StatisticsBlock*> statisticsBlocks;
    };
}

#endif // HUGGLEFEED_H
