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

#include "definitions.hpp"

#include <QHash>
#include <QMouseEvent>
#include <QFrame>
#include "collectable_smartptr.hpp"
#include "wikiedit.hpp"
#include "hugglequeue.hpp"

namespace Ui
{
    class HuggleQueueItemLabel;
}

namespace Huggle
{
    class WikiEdit;
    class HuggleQueue;

    //! This is item of queue, it is derived from qt object
    class HUGGLE_EX HuggleQueueItemLabel : public QFrame
    {
            Q_OBJECT
        public:
            explicit HuggleQueueItemLabel(QWidget *parent = nullptr);
            ~HuggleQueueItemLabel();
            void SetName(QString name);
            QString GetName();
            void Process(QLayoutItem *qi = nullptr);
            void Remove(QLayoutItem *qi = nullptr);
            void UpdatePixmap();
            void SetLabelToolTip(QString text);
            HuggleQueue *ParentQueue;
            Collectable_SmartPtr<WikiEdit> Edit;

        protected:
            void mousePressEvent(QMouseEvent *event);

        private:
            void RefreshInfo();
            QString getColor(int id);
            Ui::HuggleQueueItemLabel *ui;
            QHash<int, QString> buffer;
    };
}

#endif // HUGGLEQUEUEITEMLABEL_H
