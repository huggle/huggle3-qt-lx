//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEQUEUEFILTER_H
#define HUGGLEQUEUEFILTER_H

#include "definitions.hpp"

#include <QString>
#include <QHash>
#include <QList>
#include "mediawikiobject.hpp"

namespace Huggle
{
    enum HuggleQueueFilterMatch
    {
        //! The attribute is required for item to be considered matching
        HuggleQueueFilterMatchRequire,
        //! The attribute is not relevant at all for item to be considered matching
        HuggleQueueFilterMatchIgnore,
        //! The attribute must not be contained for this item to match
        HuggleQueueFilterMatchExclude
    };

    class WikiEdit;
    class WikiSite;

    //! Filter that can be applied to edit queue
    class HUGGLE_EX HuggleQueueFilter : public MediaWikiObject
    {
        public:
            static void Delete();
            static HuggleQueueFilter* GetFilter(QString filter_name, WikiSite *site);
            static void SetFilters();
            static QHash<WikiSite*,QList<HuggleQueueFilter*>*> Filters;
            static HuggleQueueFilter* DefaultFilter;

            //! ctr
            HuggleQueueFilter();
            ~HuggleQueueFilter() { /* not implemented yet :P */ }
            //! Returns true if edit is ok for this filter (that means it is not filtered out)
            //! if this is false the edit should not be processed later
            bool Matches(WikiEdit *edit);
            //! Information if this filter is matching minor edits or not
            HuggleQueueFilterMatch getIgnoreMinor() const;
            //! Changes if this filter is matching minor edits or not
            void setIgnoreMinor(HuggleQueueFilterMatch value);
            //! Information if this filter is matching registered user edits or not
            HuggleQueueFilterMatch getIgnoreUsers() const;
            //! Changes if this filter is matching registered users or not
            void setIgnoreUsers(HuggleQueueFilterMatch value);
            //! Information if this filter is matching edits of wl users or not
            HuggleQueueFilterMatch getIgnoreWL() const;
            //! Changes if this filter is matching edits of wl users or not
            void setIgnoreWL(HuggleQueueFilterMatch value);
            //! Information if this filter is matching IP edits or not
            HuggleQueueFilterMatch getIgnoreIP() const;
            //! Changes if this filter is matching IP edits or not
            void setIgnoreIP(HuggleQueueFilterMatch value);
            //! Information if this filter is matching bots edits or not
            HuggleQueueFilterMatch getIgnoreBots() const;
            //! Changes if this filter is matching bots edits or not
            void setIgnoreBots(HuggleQueueFilterMatch value);
            //! Information if this filter is matching new pages or not
            HuggleQueueFilterMatch getIgnoreNP() const;
            //! Changes if this filter is matching new pages or not
            void setIgnoreNP(HuggleQueueFilterMatch value);
            //! Information if this filter is matching edits made by other tools (awb / twinkle) or not
            HuggleQueueFilterMatch getIgnoreFriends() const;
            //! Changes if this filter is matching edits made by other tools edits or not
            void setIgnoreFriends(HuggleQueueFilterMatch value);
            //! Information if this filter is matching edits made by this user
            HuggleQueueFilterMatch getIgnoreSelf() const;
            //! Changes if this filter is matching edits made by current user
            void setIgnoreSelf(HuggleQueueFilterMatch value);
            bool IsDefault() const;
            bool IsChangeable() const;
            void setIgnoreTalk(HuggleQueueFilterMatch value);
            HuggleQueueFilterMatch getIgnoreTalk() const;
            HuggleQueueFilterMatch getIgnoreReverts() const;
            void setIgnoreReverts(HuggleQueueFilterMatch value);
            HuggleQueueFilterMatch getIgnore_UserSpace() const;
            void setIgnore_UserSpace(HuggleQueueFilterMatch value);
            bool IgnoresNS(int ns);
            //! Name of this queue, must be unique
            QString QueueName;
            bool ProjectSpecific;
            QHash<int,bool> Namespaces;
        private:
            HuggleQueueFilterMatch Minor;
            HuggleQueueFilterMatch Users;
            HuggleQueueFilterMatch WL;
            HuggleQueueFilterMatch IP;
            HuggleQueueFilterMatch Reverts;
            HuggleQueueFilterMatch Bots;
            HuggleQueueFilterMatch NewPages;
            HuggleQueueFilterMatch Friends;
            HuggleQueueFilterMatch Self;
            HuggleQueueFilterMatch UserSpace;
            HuggleQueueFilterMatch TalkPage;
    };

    inline HuggleQueueFilterMatch HuggleQueueFilter::getIgnoreMinor() const
    {
        return this->Minor;
    }

    inline void HuggleQueueFilter::setIgnoreMinor(HuggleQueueFilterMatch value)
    {
        this->Minor = value;
    }

    inline HuggleQueueFilterMatch HuggleQueueFilter::getIgnoreUsers() const
    {
        return this->Users;
    }

    inline void HuggleQueueFilter::setIgnoreUsers(HuggleQueueFilterMatch value)
    {
        this->Users = value;
    }

    inline HuggleQueueFilterMatch HuggleQueueFilter::getIgnoreWL() const
    {
        return this->WL;
    }

    inline void HuggleQueueFilter::setIgnoreWL(HuggleQueueFilterMatch value)
    {
        this->WL = value;
    }

    inline HuggleQueueFilterMatch HuggleQueueFilter::getIgnoreIP() const
    {
        return this->IP;
    }

    inline void HuggleQueueFilter::setIgnoreIP(HuggleQueueFilterMatch value)
    {
        this->IP = value;
    }

    inline HuggleQueueFilterMatch HuggleQueueFilter::getIgnoreBots() const
    {
        return this->Bots;
    }

    inline void HuggleQueueFilter::setIgnoreBots(HuggleQueueFilterMatch value)
    {
        this->Bots = value;
    }

    inline HuggleQueueFilterMatch HuggleQueueFilter::getIgnoreNP() const
    {
        return this->NewPages;
    }

    inline void HuggleQueueFilter::setIgnoreNP(HuggleQueueFilterMatch value)
    {
        this->NewPages = value;
    }

    inline HuggleQueueFilterMatch HuggleQueueFilter::getIgnoreFriends() const
    {
        return this->Friends;
    }

    inline HuggleQueueFilterMatch HuggleQueueFilter::getIgnoreReverts() const
    {
        return this->Reverts;
    }

    inline void HuggleQueueFilter::setIgnoreReverts(HuggleQueueFilterMatch value)
    {
        this->Reverts = value;
    }

    inline HuggleQueueFilterMatch HuggleQueueFilter::getIgnore_UserSpace() const
    {
        return this->UserSpace;
    }

    inline void HuggleQueueFilter::setIgnore_UserSpace(HuggleQueueFilterMatch value)
    {
        this->UserSpace = value;
    }

    inline void HuggleQueueFilter::setIgnoreFriends(HuggleQueueFilterMatch value)
    {
        this->Friends = value;
    }

    inline HuggleQueueFilterMatch HuggleQueueFilter::getIgnoreSelf() const
    {
        return this->Self;
    }

    inline void HuggleQueueFilter::setIgnoreTalk(HuggleQueueFilterMatch value)
    {
        this->TalkPage = value;
    }

    inline HuggleQueueFilterMatch HuggleQueueFilter::getIgnoreTalk() const
    {
        return this->TalkPage;
    }

    inline void HuggleQueueFilter::setIgnoreSelf(HuggleQueueFilterMatch value)
    {
        this->Self = value;
    }

    inline bool HuggleQueueFilter::IsDefault() const
    {
        return this == HuggleQueueFilter::DefaultFilter;
    }

    inline bool HuggleQueueFilter::IsChangeable() const
    {
        return !this->IsDefault() && !this->ProjectSpecific;
    }
}

#endif // HUGGLEQUEUEFILTER_H
