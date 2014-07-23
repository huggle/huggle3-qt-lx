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
#include "mediawikiobject.hpp"

namespace Huggle
{
    class HuggleQueueFilter;
    class WikiEdit;
    class WikiSite;

    //! Feed provider stub class every provider must be derived from this one
    class HuggleFeed : public MediaWikiObject
    {
        public:
            static QList<HuggleFeed*> Providers;

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
            //! If provider is not to be automatically inserted to a list of providers
            //! Builtin providers have hardcoded menus, so they are ignored.
            virtual bool IsBuiltin() { return true; }
            //! Returns true in case that a provider is stopped and can be safely deleted

            //! This is useful in case we are running some background threads and we need to
            //! wait for them to finish before we can delete the object
            virtual bool IsStopped() { return true; }
            //! Return a last edit from cache or NULL
            virtual WikiEdit *RetrieveEdit() { return nullptr; }
            virtual QString ToString() = 0;
            double GetUptime();
            HuggleQueueFilter *Filter;
            //! Number of edits made since you logged in
            double EditCounter;
            //! Number of reverts made since you logged in
            double RvCounter;
        protected:
            QDateTime UptimeDate;
    };
}

#endif // HUGGLEFEED_H
