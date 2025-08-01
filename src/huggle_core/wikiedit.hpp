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
    class WikiPage;
    class WikiEdit;
    class WikiUser;
    class WikiSite;

    typedef void* (*WEPostprocessedCallback) (WikiEdit*);

    //! Edits are post processed in this thread
    class HUGGLE_EX_CORE WikiEdit_ProcessorThread :  public QThread
    {
            Q_OBJECT
        public:
            static QList<WikiEdit *> PendingEdits;
            static HMUTEX_TYPE EditLock;
            void Process(WikiEdit *edit);
        protected:
            void run();
    };

    //! Wiki edit

	//! Edits are representing a change made to a page on a wiki. Information about edits are loaded using various methods using change information providers
	//! such as IRC or XmlRcs. Because each provider contains different amount of information about the edit, each edit needs postprocessing in order to
	//! load all missing information using MediaWiki API or other methods (retrieve the information from cache, or using some regexes etc.)

	//! That means each edit has has one of 3 basic states in its lifecycle:
	//! None - edit was just created and is not suitable for use elsewhere in the program as many attributes may be missing, these attributes are inserted during processing and post processing
	//! Processed - edit was processed, that means the basic informations about the edit were properly set by various helper functions.
	//!				Processing is a very quick operation, it happens within the program, no external API calls are needed. It fills up most of essential properties, but not all of them.
	//! Postprocessed - edit was processed by the processor thread which handles various API calls asynchronously in order to fetch all available detailed information about the edit and the detailed information.

	//! Edits are heavily cached across Huggle in order to avoid unnecessary API calls

    //! \image html ../documentation/providers.png
    class HUGGLE_EX_CORE WikiEdit : public Collectable
    {
        public:
            //! This function will return a constant (which needs to be generated runtime)
            //! which is used as "unknown time" in case we don't know the edit's time
            static QDateTime GetUnknownEditTime();
            static Collectable_SmartPtr<WikiEdit> FromCacheByRevID(revid_ht revid, const QString& prev = "prev");
            static QString GetPixmapFromEditType(EditType edit_type);
            //! This list contains reference to all existing edits in memory
            static QList<WikiEdit*> EditList;
            static HMUTEX_TYPE* Lock_EditList;

            //! Creates a new empty wiki edit
            WikiEdit();
            ~WikiEdit() override;
            //! This function is called by internals of huggle
            void PostProcess(); 
            WikiSite *GetSite();
            void SetSize(long size);
            long GetSize();
            QString GetPixmap();
            //! Return a full url to edit
            QString GetFullUrl();
            bool IsRangeOfEdits();
            //! Return true in case this edit was post processed already
            bool IsPostProcessed();
            //! If edit is ready to be added to queue
            bool IsReady();
            //! Processes all score words in text
            void ProcessWords();
            void RecordScore(const QString& name, score_ht score);
            void RemoveFromHistoryChain();
            //! Whether edit is revert - this is based on edit summary using patterns defined per project
            bool IsRevert();
            QString ContentModel;
            //! Page that was changed by edit
            WikiPage *Page;
            //! User who changed the page
            WikiUser *User;
            //! Edit is a minor edit
            bool IsMinor;
            //! Edit is a bot edit
            bool Bot;
            //! Edit is a new page
            bool NewPage;
            bool SizeIsKnown = false;
            QString DiffTo = "prev";
            //! Diff id - this is probably same as RevID and can be safely removed
            revid_ht Diff;
            bool IsValid = true;
            //! Old id
            revid_ht OldID;
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
            long Score = 0;
            //! This score is used to determine if edit was done in good faith, even if it wasn't OK
            long GoodfaithScore = 0;
            //! Function to call when post processing of edit is finished
            WEPostprocessedCallback PostprocessCallback = nullptr;
            void *PostprocessCallback_Owner = nullptr;
            QHash<QString, QVariant> PropertyBag;
            //! You can insert special properties for this edit here that are displayed in huggle interface
            QHash<QString, QString> MetaLabels;
            QStringList Tags;
            //! List of parsed score words which were found in this edit
            QStringList ScoreWords;
            QDateTime Time;
        protected:
            void processCallback();
            //! This function is called by core
            bool finalizePostProcessing();
            bool processingByWorkerThread;
            bool processingRevs;
            bool processingEditInfo;
            bool processingDiff = false;
            //! This variable is used by worker thread and needs to be public so that it is working
            bool postProcessing;
            //! This variable is used by worker thread and needs to be public so that it is working
            bool processedByWorkerThread;
            Collectable_SmartPtr<ApiQuery> qTalkpage;
            //! This is a query used to retrieve information about the user
            Collectable_SmartPtr<ApiQuery> qUser;
            //! Query that get information about revs
            Collectable_SmartPtr<ApiQuery> qRevisionInfo;
            Collectable_SmartPtr<ApiQuery> qDifference;
            Collectable_SmartPtr<ApiQuery> qFounder;
            Collectable_SmartPtr<ApiQuery> qText;
            Collectable_SmartPtr<ApiQuery> qCategoriesAndWatched;
            //! Size of change of edit
            long diffSize;
            //! Cached information whether edit is a revert or not
            //! -1 unknown, 0 false, 1 true
            int isRevert = -1;
            friend class WikiEdit_ProcessorThread;
            friend class MainWindow;
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
        return this->diffSize;
    }

    inline void WikiEdit::SetSize(long size)
    {
        this->SizeIsKnown = true;
        this->diffSize = size;
    }
}

#endif // WIKIEDIT_H
