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
// now we need to ensure that python is included first
#ifdef PYTHONENGINE
#include <Python.h>
#endif

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
            bool getIgnoreReverts() const;
            void setIgnoreReverts(bool value);
            //! Name of this queue, must be unique
            QString QueueName;
            bool ProjectSpecific;

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
            bool IgnoreTalk;
    };
}

#endif // HUGGLEQUEUEFILTER_H
