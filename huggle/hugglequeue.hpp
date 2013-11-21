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

#include <QDockWidget>
#include <QList>
#include <QFrame>
#include <QWidget>
#include <QVBoxLayout>
#include "hugglequeuefilter.hpp"
#include "exception.hpp"
#include "hugglequeueitemlabel.hpp"
#include "wikiedit.hpp"
#include "configuration.hpp"

namespace Ui
{
    class HuggleQueue;
}

namespace Huggle
{
    class HuggleQueueFilter;
    class HuggleQueueItemLabel;

    //! Queue of edits
    class HuggleQueue : public QDockWidget
    {
            Q_OBJECT

        public:
            HuggleQueueFilter *CurrentFilter;
            QList<HuggleQueueItemLabel*> Items;
            explicit HuggleQueue(QWidget *parent = 0);
            ~HuggleQueue();
            void AddItem(WikiEdit *page);
            void Next();
            WikiEdit *GetWikiEditByRevID(int RevID);
            void DeleteByRevID(int RevID);
            void Sort();
            void SortItemByEdit(WikiEdit *e);
            void ResortItem(QLayoutItem *item, int position = -1);
            void Delete(HuggleQueueItemLabel *item, QLayoutItem *qi = NULL);
            void Trim(int i);
            //! Remove 1 item
            void Trim();

        private:
            long GetScore(int id);
            //! Internal function
            void DeleteItem(HuggleQueueItemLabel *item);
            Ui::HuggleQueue *ui;
            QVBoxLayout *layout;
            QWidget *xx;
            QFrame *frame;
    };
}

#endif // HUGGLEQUEUE_H
