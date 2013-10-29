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
#include "wikiedit.h"

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
        bool getIgnoreMinor() const;
        void setIgnoreMinor(bool value);
        bool getIgnoreUsers() const;
        void setIgnoreUsers(bool value);
        bool getIgnoreWL() const;
        void setIgnoreWL(bool value);
        bool getIgnoreIP() const;
        void setIgnoreIP(bool value);
        bool getIgnoreBots() const;
        void setIgnoreBots(bool value);
        bool getIgnoreNP() const;
        void setIgnoreNP(bool value);
        bool getIgnoreFriends() const;
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
