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

#include <QString>
#include <QVariant>
#include <QHash>
#include <QThread>
#include <QDateTime>
#include <QList>
#include "apiquery.hpp"
#include "collectable.hpp"
#include "collectable_smartptr.hpp"
#include "edittype.hpp"

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
    class MainWindow;
    class WikiPage;
    class WikiEdit;
    class WikiUser;
    class WikiSite;

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

    //! Basically all changes to pages can be represented by this class.
    //! Edits are representing an edit made to a page on a wiki, they
    //! have always 3 basic statuses:
    //! None - edit was just created and is not suitable for use
    //! Processed - edit was pre processed, the basic information were properly set. This
    //!             is a very quick serial operation that is handled by Core::Process
    //! Postprocessed - edit was processed by processor thread and the detailed information
    //!                 were downloaded using separate queries, in this phase the edit
    //!                 structure contains all possible needed information we want.
    //! \image html ../documentation/providers.png
    class HUGGLE_EX WikiEdit : public Collectable
    {
        public:
            //! This function will return a constant (which needs to be generated runtime)
            //! which is used as "unknown time" in case we don't know the edit's time
            static QDateTime GetUnknownEditTime();
            static Collectable_SmartPtr<WikiEdit> FromCacheByRevID(revid_ht revid, QString prev = "prev");
            static QString GetPixmapFromEditType(EditType edit_type);
            //! This list contains reference to all existing edits in memory
            static QList<WikiEdit*> EditList;
            static QMutex *Lock_EditList;

            //! Creates a new empty wiki edit
            WikiEdit();
            ~WikiEdit();
            //! This function is called by internals of huggle
            void PostProcess(); 
            WikiSite *GetSite();
            void SetSize(long size);
            long GetSize();
            QString GetPixmap();
            //! Return a full url to edit
            QString GetFullUrl();
            //! Return true in case this edit was post processed already
            bool IsPostProcessed();
            //! Processes all score words in text
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
            bool SizeIsKnown = false;
            QString DiffTo = "prev";
            //! Diff id - this is probably same as RevID and can be safely removed
            revid_ht Diff;
            //! Priority in queue
            int Priority;
            bool IsValid = true;
            //! Old id
            revid_ht OldID;
            bool IsRevert;
            //! Revision ID
            revid_ht RevID;
            //! Indicator whether the edit was processed or not
            WEStatus Status;
            //! Current warning level
            WarningLevel CurrentUserWarningLevel;
            //! Summary of edit
            QString Summary;
            //! If diff is split holds a new text of a diff
            QString DiffText_New;
            //! If diff is split this holds an old text of a diff
            QString DiffText_Old;
            //! Text of diff, usually formatted in html style returned by mediawiki
            QString DiffText;
            bool DiffText_IsSplit = false;
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
            QHash<QString, QVariant> PropertyBag;
            //! List of parsed score words which were found in this edit
            QStringList ScoreWords;
            QDateTime Time;
            //! This variable is used by worker thread and needs to be public so that it is working
            bool PostProcessing;
            //! This variable is used by worker thread and needs to be public so that it is working
            bool ProcessedByWorkerThread;
        private:
            //! This function is called by core
            bool FinalizePostProcessing();
            friend class ProcessorThread;
            friend class MainWindow;
            bool ProcessingByWorkerThread;
            bool ProcessingRevs;
            bool ProcessingDiff;
            Collectable_SmartPtr<ApiQuery> qTalkpage;
            //! This is a query used to retrieve information about the user
            Collectable_SmartPtr<ApiQuery> qUser;
            Collectable_SmartPtr<ApiQuery> qDifference;
            Collectable_SmartPtr<ApiQuery> qFounder;
            Collectable_SmartPtr<ApiQuery> qText;
            //! Size of change of edit
            long Size;
    };

    inline QDateTime WikiEdit::GetUnknownEditTime()
    {
        return QDateTime::fromMSecsSinceEpoch(0);
    }

    inline bool WikiEdit::IsPostProcessed()
    {
        return (this->Status == StatusPostProcessed);
    }

    inline long WikiEdit::GetSize()
    {
        return this->Size;
    }

    inline void WikiEdit::SetSize(long size)
    {
        this->SizeIsKnown = true;
        this->Size = size;
    }
}

#endif // WIKIEDIT_H
