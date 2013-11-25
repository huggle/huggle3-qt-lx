//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEQUEUEITEMLABEL_H
#define HUGGLEQUEUEITEMLABEL_H

#include <QMouseEvent>
#include <QFrame>
#include "core.hpp"
#include "exception.hpp"

namespace Ui
{
    class HuggleQueueItemLabel;
}

namespace Huggle
{
    class WikiEdit;
    class HuggleQueue;

    //! This is item of queue, it is derived from qt object
    class HuggleQueueItemLabel : public QFrame
    {
            Q_OBJECT

        public:
            static int Count;

            explicit HuggleQueueItemLabel(QWidget *parent = 0);
            ~HuggleQueueItemLabel();
            void SetName(QString name);
            QString GetName();
            void Process(QLayoutItem *qi = NULL);
            void Remove();
            HuggleQueue *ParentQueue;
            WikiEdit *Page;

        protected:
            void mousePressEvent(QMouseEvent *event);

        private:
            Ui::HuggleQueueItemLabel *ui;
    };
}

#endif // HUGGLEQUEUEITEMLABEL_H
