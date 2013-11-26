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

#include <QString>
#include <QThread>
#include <QMutex>
#include <QDateTime>
#include <QtXml>
#include <QList>
#include "apiquery.hpp"
#include "collectable.hpp"
#include "wikiuser.hpp"
#include "wikipage.hpp"

#define WIKI_UNKNOWN_REVID -1

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

    class Query;
    class ApiQuery;

    //! Wiki edit

    //! Basically all changes to pages can be represented by this class
    class WikiEdit : public Collectable
    {
        public:
            //! This list contains reference to all existing edits in memory
            static QList<WikiEdit*> EditList;
            //! Creates a new empty wiki edit
            WikiEdit();
            ~WikiEdit();
            //! Get a level of warning from talk page
            static int GetLevel(QString page);
            //! This function is called by core
            bool FinalizePostProcessing();
            //! This function is called by internals of huggle
            void PostProcess();
            //! Return a full url to edit
            QString GetFullUrl();
            //! Return true in case this edit was post processed already
            bool IsPostProcessed();
            void ProcessWords();
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
            WEStatus Status;
            //! Current warning level
            WarningLevel CurrentUserWarningLevel;
            //! Summary of edit
            QString Summary;
            QString RollbackToken;
            QString DiffText;
            //! If this is true the edit was made by huggle
            bool EditMadeByHuggle;
            //! If this is true the edit was made by some other
            //! tool for vandalism reverting
            bool TrustworthEdit;
            //! Edit was made by you
            bool OwnEdit;
            WikiEdit *Previous;
            WikiEdit *Next;
            long Score;
            QStringList ScoreWords;
            bool PostProcessing;
            bool ProcessingByWorkerThread;
            QDateTime Time;
            bool ProcessedByWorkerThread;
        private:
            bool ProcessingRevs;
            bool ProcessingDiff;
            ApiQuery* ProcessingQuery;
            ApiQuery* DifferenceQuery;
    };
}

#endif // WIKIEDIT_H
