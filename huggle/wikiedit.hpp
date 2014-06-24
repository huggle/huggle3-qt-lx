//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WIKIEDIT_H
#define WIKIEDIT_H

#include "definitions.hpp"
// now we need to ensure that python is included first
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QString>
#include <QThread>
#include <QDateTime>
#include <QList>
#include "apiquery.hpp"
#include "collectable.hpp"
#include "collectable_smartptr.hpp"

namespace Huggle
{
    enum WarningLevel
    {
        WarningLevelNone,
        WarningLevel1,
        WarningLevel2,
        WarningLevel3,
        WarningLevel4
    };

    enum WEStatus
    {
        StatusNone,
        StatusProcessed,
        StatusPostProcessed
    };

    class Query;
    class ApiQuery;
    class WikiPage;
    class WikiEdit;
    class WikiUser;

    //! Edits are post processed in this thread
    class ProcessorThread :  public QThread
    {
            Q_OBJECT
        public:
            static QList<WikiEdit *> PendingEdits;
            static QMutex EditLock;
            void Process(WikiEdit *edit);
        protected:
            void run();
    };

    //! Wiki edit

    //! Basically all changes to pages can be represented by this class
    //! \image html ../documentation/providers.png
    class WikiEdit : public Collectable
    {
        public:
            //! This function will return a constant (which needs to be generated runtime)
            //! which is used as "unknown time" in case we don't know the edit's time
            static QDateTime GetUnknownEditTime();
            //! This list contains reference to all existing edits in memory
            static QList<WikiEdit*> EditList;
            static QMutex *Lock_EditList;

            //! Creates a new empty wiki edit
            WikiEdit();
            ~WikiEdit();
            //! This function is called by core
            bool FinalizePostProcessing();
            //! This function is called by internals of huggle
            void PostProcess();
            //! Return a full url to edit
            QString GetFullUrl();
            //! Return true in case this edit was post processed already
            bool IsPostProcessed();
            void ProcessWords();
            void RemoveFromHistoryChain();
            //! Page that was changed by edit
            WikiPage *Page;
            //! User who changed the page
            WikiUser *User;
            //! Edit is a minor edit
            bool Minor;
            //! Edit is a bot edit
            bool Bot;
            //! Edit is a new page
            bool NewPage;
            //! Size of change of edit
            int Size;
            //! Diff id
            int Diff;
            //! Priority in queue
            int Priority;
            //! Old id
            int OldID;
            bool IsRevert;
            //! Revision ID
            int RevID;
            //! Indicator whether the edit was processed or not
            WEStatus Status;
            //! Current warning level
            WarningLevel CurrentUserWarningLevel;
            //! Summary of edit
            QString Summary;
            //! Token that can be used to rollback this edit

            //! This token needs to be retrieved in same time as information about edit, so that
            //! it's not possible for other user to change the page meanwhile it's reviewed
            QString RollbackToken;
            //! Text of diff, usually formatted in html style returned by mediawiki
            QString DiffText;
            //! Base time of last revision of talk page which is needed to check if someone changed the talk
            //! page meanwhile before we change it
            QString TPRevBaseTime;
            //! If this is true the edit was made by huggle
            bool EditMadeByHuggle;
            //! If this is true the edit was made by some other
            //! tool for vandalism reverting
            bool TrustworthEdit;
            //! Edit was made by you
            bool OwnEdit;
            //! Link to previous edit in huggle history
            WikiEdit *Previous;
            //! Link to next edit in huggle history
            WikiEdit *Next;
            //! Badness score of this edit
            long Score;
            //! List of parsed score words which were found in this edit
            QStringList ScoreWords;
            QString PatrolToken;
            QDateTime Time;
            //! This variable is used by worker thread and needs to be public so that it is working
            bool PostProcessing;
            //! This variable is used by worker thread and needs to be public so that it is working
            bool ProcessedByWorkerThread;
        private:
            bool ProcessingByWorkerThread;
            bool ProcessingRevs;
            bool ProcessingDiff;
            Collectable_SmartPtr<ApiQuery> qTalkpage;
            //! This is a query used to retrieve information about the user
            Collectable_SmartPtr<ApiQuery> qUser;
            Collectable_SmartPtr<ApiQuery> qDifference;
            Collectable_SmartPtr<ApiQuery> qText;
    };

    inline QDateTime WikiEdit::GetUnknownEditTime()
    {
        return QDateTime::fromMSecsSinceEpoch(0);
    }

    inline bool WikiEdit::IsPostProcessed()
    {
        return (this->Status == StatusPostProcessed);
    }
}

#endif // WIKIEDIT_H
