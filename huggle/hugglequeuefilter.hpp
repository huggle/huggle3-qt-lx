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
#include <QList>
#include "mediawikiobject.hpp"

namespace Huggle
{
    class WikiEdit;

    //! Filter that can be applied to edit queue
    class HuggleQueueFilter : public MediaWikiObject
    {
        public:
            static QList<HuggleQueueFilter*> Filters;
            static HuggleQueueFilter *DefaultFilter;

            //! ctr
            HuggleQueueFilter();
            //! Returns true if edit is ok for this filter (that means it is not filtered out)
            //! if this is false the edit should not be processed later
            bool Matches(WikiEdit *edit);
            //! Information if this filter is matching minor edits or not
            bool getIgnoreMinor() const;
            //! Changes if this filter is matching minor edits or not
            void setIgnoreMinor(bool value);
            //! Information if this filter is matching registered user edits or not
            bool getIgnoreUsers() const;
            //! Changes if this filter is matching registered users or not
            void setIgnoreUsers(bool value);
            //! Information if this filter is matching edits of wl users or not
            bool getIgnoreWL() const;
            //! Changes if this filter is matching edits of wl users or not
            void setIgnoreWL(bool value);
            //! Information if this filter is matching IP edits or not
            bool getIgnoreIP() const;
            //! Changes if this filter is matching IP edits or not
            void setIgnoreIP(bool value);
            //! Information if this filter is matching bots edits or not
            bool getIgnoreBots() const;
            //! Changes if this filter is matching bots edits or not
            void setIgnoreBots(bool value);
            //! Information if this filter is matching new pages or not
            bool getIgnoreNP() const;
            //! Changes if this filter is matching new pages or not
            void setIgnoreNP(bool value);
            //! Information if this filter is matching edits made by other tools (awb / twinkle) or not
            bool getIgnoreFriends() const;
            //! Changes if this filter is matching edits made by other tools edits or not
            void setIgnoreFriends(bool value);
            //! Information if this filter is matching edits made by this user
            bool getIgnoreSelf() const;
            //! Changes if this filter is matching edits made by current user
            void setIgnoreSelf(bool value);
            bool IsDefault() const;
            bool IsChangeable() const;
            void setIgnoreTalk(bool value);
            bool getIgnoreTalk() const;
            bool getIgnoreReverts() const;
            void setIgnoreReverts(bool value);
            //! Name of this queue, must be unique
            QString QueueName;
            bool ProjectSpecific;
            bool getIgnore_UserSpace() const;
            void setIgnore_UserSpace(bool value);

        private:
            bool IgnoreMinor;
            bool IgnoreUsers;
            bool IgnoreWL;
            bool IgnoreIP;
            bool IgnoreReverts;
            bool IgnoreBots;
            bool IgnoreNP;
            bool IgnoreFriends;
            bool IgnoreSelf;
            bool Ignore_UserSpace;
            bool IgnoreTalk;
    };

    inline bool HuggleQueueFilter::getIgnoreMinor() const
    {
        return this->IgnoreMinor;
    }

    inline void HuggleQueueFilter::setIgnoreMinor(bool value)
    {
        this->IgnoreMinor = value;
    }

    inline bool HuggleQueueFilter::getIgnoreUsers() const
    {
        return this->IgnoreUsers;
    }

    inline void HuggleQueueFilter::setIgnoreUsers(bool value)
    {
        this->IgnoreUsers = value;
    }

    inline bool HuggleQueueFilter::getIgnoreWL() const
    {
        return this->IgnoreWL;
    }

    inline void HuggleQueueFilter::setIgnoreWL(bool value)
    {
        this->IgnoreWL = value;
    }

    inline bool HuggleQueueFilter::getIgnoreIP() const
    {
        return this->IgnoreIP;
    }

    inline void HuggleQueueFilter::setIgnoreIP(bool value)
    {
        this->IgnoreIP = value;
    }

    inline bool HuggleQueueFilter::getIgnoreBots() const
    {
        return this->IgnoreBots;
    }

    inline void HuggleQueueFilter::setIgnoreBots(bool value)
    {
        this->IgnoreBots = value;
    }

    inline bool HuggleQueueFilter::getIgnoreNP() const
    {
        return this->IgnoreNP;
    }

    inline void HuggleQueueFilter::setIgnoreNP(bool value)
    {
        this->IgnoreNP = value;
    }

    inline bool HuggleQueueFilter::getIgnoreFriends() const
    {
        return this->IgnoreFriends;
    }

    inline bool HuggleQueueFilter::getIgnoreReverts() const
    {
        return this->IgnoreReverts;
    }

    inline void HuggleQueueFilter::setIgnoreReverts(bool value)
    {
        this->IgnoreReverts = value;
    }

    inline bool HuggleQueueFilter::getIgnore_UserSpace() const
    {
        return Ignore_UserSpace;
    }

    inline void HuggleQueueFilter::setIgnore_UserSpace(bool value)
    {
        Ignore_UserSpace = value;
    }

    inline void HuggleQueueFilter::setIgnoreFriends(bool value)
    {
        this->IgnoreFriends = value;
    }

    inline bool HuggleQueueFilter::getIgnoreSelf() const
    {
        return this->IgnoreSelf;
    }

    inline void HuggleQueueFilter::setIgnoreTalk(bool value)
    {
        this->IgnoreTalk = value;
    }

    inline bool HuggleQueueFilter::getIgnoreTalk() const
    {
        return this->IgnoreTalk;
    }

    inline void HuggleQueueFilter::setIgnoreSelf(bool value)
    {
        this->IgnoreSelf = value;
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
