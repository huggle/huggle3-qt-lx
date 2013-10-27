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
#include "wikiedit.h"

namespace Huggle
{
    class WikiEdit;

    //! Filter that can be applied to edit queue
    class HuggleQueueFilter
    {
    public:
        QString QueueName;
        HuggleQueueFilter();
        bool Matches(WikiEdit *edit);
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
