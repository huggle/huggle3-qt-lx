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

#include <QString>
#include <QList>
#include "wikiedit.hpp"

namespace Huggle
{
    class WikiEdit;

    //! Filter that can be applied to edit queue
    class HuggleQueueFilter
    {
    public:
        static QList<HuggleQueueFilter*> Filters;
        static HuggleQueueFilter *DefaultFilter;
        QString QueueName;
        HuggleQueueFilter();
        bool Matches(WikiEdit *edit);
        /// \todo DOCUMENT ME
        bool getIgnoreMinor() const;
        /// \todo DOCUMENT ME
        void setIgnoreMinor(bool value);
        /// \todo DOCUMENT ME
        bool getIgnoreUsers() const;
        /// \todo DOCUMENT ME
        void setIgnoreUsers(bool value);
        /// \todo DOCUMENT ME
        bool getIgnoreWL() const;
        /// \todo DOCUMENT ME
        void setIgnoreWL(bool value);
        /// \todo DOCUMENT ME
        bool getIgnoreIP() const;
        /// \todo DOCUMENT ME
        void setIgnoreIP(bool value);
        /// \todo DOCUMENT ME
        bool getIgnoreBots() const;
        /// \todo DOCUMENT ME
        void setIgnoreBots(bool value);
        /// \todo DOCUMENT ME
        bool getIgnoreNP() const;
        /// \todo DOCUMENT ME
        void setIgnoreNP(bool value);
        /// \todo DOCUMENT ME
        bool getIgnoreFriends() const;
        /// \todo DOCUMENT ME
        void setIgnoreFriends(bool value);

    private:
        bool IgnoreMinor;
        bool IgnoreUsers;
        bool IgnoreWL;
        bool IgnoreIP;
        bool IgnoreBots;
        bool IgnoreNP;
        bool IgnoreFriends;
        bool IgnoreTalk;
    };
}

#endif // HUGGLEQUEUEFILTER_H
