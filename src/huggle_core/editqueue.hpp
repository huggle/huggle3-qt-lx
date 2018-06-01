//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef EDITQUEUE_HPP
#define EDITQUEUE_HPP

#include "definitions.hpp"
#include "collectable_smartptr.hpp"
#include "mediawikiobject.hpp"
#include <QHash>

namespace Huggle
{
    class ApiQuery;
    class WikiEdit;
    class WikiUser;
    class WikiSite;
    class EditQueue;

    /*!
     * \brief The EditQueue_UnprocessedEdit class is used to process information about new edit that is supposed to be obtained only
     *        by revision ID. Since 3.4.3 there is an option to insert new edits to queue by revision ID, this class is used to
     *        fetch information about the edit, that would otherwise be provided by feed provider.
     */
    class HUGGLE_EX_CORE EditQueue_UnprocessedEdit : public MediaWikiObject
    {
        public:
            EditQueue_UnprocessedEdit(WikiSite *site, revid_ht rev_id, EditQueue *parent);
            revid_ht GetID();
            // Remove from queue
            void Remove();
            bool IsPostProcessing();
            EditQueue *Parent;
            Collectable_SmartPtr<WikiEdit> Edit;
        protected:
            revid_ht revID;
    };

    /*!
     * \brief The EditQueue class handles basic operations and logic of edit queue
     */
    class HUGGLE_EX_CORE EditQueue
    {
        public:
            static EditQueue *Primary;

            EditQueue();
            virtual ~EditQueue();
            /*!
             * \brief Insert a new item to queue
             *  This function is automatically inserting the item to proper location, you shouldn't
             *  call a sort function after this
             * \param page is a pointer to wiki edit you want to insert to queue
             */
            virtual void AddItem(WikiEdit *page);
            //! This can be used to add new edit to queue by rev id
            //! edit will be processed on background and then inserted
            virtual void AddUnprocessedEditFromRevID(revid_ht rev_id, WikiSite *site);
            /*!
             * \brief DeleteByScore deletes all edits that have lower than specified score
             * \param Score
             * \return number of records that matched the score
             */
            virtual int DeleteByScore(long Score);
            virtual bool DeleteByRevID(revid_ht RevID, WikiSite *site);
            //! Delete all edits to the page that are older than this edit
            virtual void DeleteOlder(WikiEdit *edit);
            virtual void UpdateUser(WikiUser *user);
            virtual WikiEdit *GetWikiEditByRevID(revid_ht RevID, WikiSite *site)=0;
        protected:
            // Callbacks
            static void newEditByRevID_Success(WikiEdit *edit, void *source, QString error);
            static void newEditByRevID_Fail(WikiEdit *edit, void *source, QString error);
            static void newEditByRevID_Finish(WikiEdit *edit);
            //! Returns true if this edit was already requested and is now being processed
            bool checkIfNewEditByRevIDExists(revid_ht rev_id, WikiSite *site);
            WikiSite *currentSite = nullptr;
            //! List of queries that are attached to unknown rev_id's to be added to queue
            QList<EditQueue_UnprocessedEdit*> newUnprocessedEditsByRevID;
            friend class EditQueue_UnprocessedEdit;
            void cleanupUnprocessedEdit(EditQueue_UnprocessedEdit *edit);
    };
}

#endif // EDITQUEUE_HPP
