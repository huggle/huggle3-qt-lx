//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef USERCONFIGURATION_H
#define USERCONFIGURATION_H

// Include file with all global defines
#include "definitions.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QString>

#include <QList>
#include <QStringList>
#include <QHash>
#include "huggleoption.hpp"

namespace Huggle
{
    //! This enum defines what action should be done when revert etc
    enum Configuration_OnNext
    {
        //! Display same edit
        Configuration_OnNext_Stay,
        //! Display next
        Configuration_OnNext_Next,
        //! Display revert
        Configuration_OnNext_Revert
    };

    //! User configuration, for a user per project
    class UserConfiguration
    {
        public:
            ~UserConfiguration();
            QHash<QString, HuggleOption*> UserOptions;
            bool                    EnforceMonthsAsHeaders = true;
            unsigned int            TalkPageFreshness = 20;
            //! If history and user info should be automatically loaded for every edit
            bool                    HistoryLoad = true;
            //! Defines what should be done on next edit
            Configuration_OnNext    GoNext = Configuration_OnNext_Next;
            bool                    DeleteEditsAfterRevert = true;
            //! Fetch only the last edit of page, that means if there is a newer edit
            //! it get automatically loaded instead of cached version
            bool                    LastEdit = false;
            bool                    SectionKeep = true;
            unsigned int            HistoryMax = 50;
            bool                    TruncateEdits = false;
            bool                    RevertNewBySame = true;
            //! If this is set to false the warning will be selected by huggle when user decide to
            //! use the "warn only" feature in huggle (W) for example, it doesn't affect reverting
            bool                    ManualWarning = false;
            //! Large title of every page in top of diff
            bool                    DisplayTitle = false;
            //! Result of "Stop feed, Remove old edits" in main form
            bool                    RemoveOldQueueEdits = false;
            bool                    CheckTP = false;
            QString                 QueueID = "default";
            //! Display messages from users in vandal window
            bool                    HAN_DisplayUser = true;
            bool                    HAN_DisplayBots = true;
            bool                    HAN_DisplayUserTalk = true;
            //! Welcome new users on a good edit
            bool                    WelcomeGood = true;
    };
}

#endif
