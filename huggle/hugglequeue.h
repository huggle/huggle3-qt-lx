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
#include "hugglequeuefilter.h"
#include "hugglequeueitemlabel.h"
#include "wikiedit.h"

namespace Ui {
class HuggleQueue;
}

class HuggleQueueFilter;
class HuggleQueueItemLabel;

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
    void Delete(HuggleQueueItemLabel *item, QLayoutItem *qi = NULL);
    //! Remove 1 item
    void Trim();

private:
    int GetScore(int id);
    Ui::HuggleQueue *ui;
    QVBoxLayout *layout;
    QWidget *xx;
    QFrame *frame;
};

#endif // HUGGLEQUEUE_H
