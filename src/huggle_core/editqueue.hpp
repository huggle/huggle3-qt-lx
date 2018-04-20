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

namespace Huggle
{
    class WikiEdit;
    class WikiUser;
    class WikiSite;
    class HUGGLE_EX EditQueue
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
    };
}

#endif // EDITQUEUE_HPP
