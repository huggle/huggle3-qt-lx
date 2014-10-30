//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEQUEUE_H
#define HUGGLEQUEUE_H

#include "definitions.hpp"

#include <QDockWidget>
#include <QList>
#include <QFrame>
#include <QWidget>
#include <QVBoxLayout>
#include "hugglequeuefilter.hpp"
#include "hugglequeueitemlabel.hpp"
#include "wikiedit.hpp"

namespace Ui
{
    class HuggleQueue;
}

namespace Huggle
{
    class HuggleQueueFilter;
    class HuggleQueueItemLabel;
    class WikiEdit;
    class WikiUser;

    //! Queue of edits
    class HuggleQueue : public QDockWidget
    {
            Q_OBJECT
        public:
            explicit HuggleQueue(QWidget *parent = 0);
            ~HuggleQueue();
            /*!
             * \brief Insert a new item to queue
             *  This function is automatically inserting the item to proper location, you shouldn't
             *  call a sort function after this
             * \param page is a pointer to wiki edit you want to insert to queue
             */
            void AddItem(WikiEdit *page);
            void Delete(HuggleQueueItemLabel *item, QLayoutItem *qi = nullptr);
            /*!
             * \brief DeleteByScore deletes all edits that have lower than specified score
             * \param Score
             * \return number of records that matched the score
             */
            int DeleteByScore(long Score);
            bool DeleteByRevID(int RevID, WikiSite *site);
            //! Delete all edits to the page that are older than this edit
            void DeleteOlder(WikiEdit *edit);
            void UpdateUser(WikiUser *user);
            //! Reload filters
            void Filters();
            //! Switch and render next edit in queue
            void Next();
            WikiEdit *GetWikiEditByRevID(int RevID, WikiSite *site);
            void Sort();
            void SortItemByEdit(WikiEdit *e);
            void Trim(int i);
            //! Remove 1 item
            void Trim();
            void Clear();
            void RedrawTitle();
            HuggleQueueFilter *CurrentFilter;
            QList<HuggleQueueItemLabel*> Items;
        private slots:
            void on_comboBox_currentIndexChanged(int index);
        private:
            long GetScore(int id);
            void ResortItem(QLayoutItem *item, int position = -1);
            //! Internal function
            bool DeleteItem(HuggleQueueItemLabel *item);
            Ui::HuggleQueue *ui;
            bool loading;
    };
}

#endif // HUGGLEQUEUE_H
