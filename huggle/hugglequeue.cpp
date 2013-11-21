//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglequeue.hpp"
#include "ui_hugglequeue.h"

using namespace Huggle;

HuggleQueue::HuggleQueue(QWidget *parent) : QDockWidget(parent), ui(new Ui::HuggleQueue)
{
    this->ui->setupUi(this);
    this->CurrentFilter = HuggleQueueFilter::DefaultFilter;
    this->layout = new QVBoxLayout(ui->scrollArea);
    this->layout->setMargin(0);
    this->layout->setSpacing(0);
    this->xx = new QWidget();
    this->frame = new QFrame();
    this->ui->scrollArea->setWidget(this->xx);
    this->xx->setLayout(this->layout);
    this->layout->addWidget(this->frame);
    this->Filters();
    this->ui->comboBox->setCurrentIndex(0);
}

HuggleQueue::~HuggleQueue()
{
    delete this->layout;
    delete this->CurrentFilter;
    delete this->ui;
}

void HuggleQueue::AddItem(WikiEdit *page)
{
    if (page == NULL)
    {
        throw new Exception("WikiEdit *page must not be NULL", "void HuggleQueue::AddItem(WikiEdit *page)");
    }
    // so we need to insert the item somehow
    HuggleQueueItemLabel *label = new HuggleQueueItemLabel(this);
    page->RegisterConsumer(HUGGLECONSUMER_QUEUE);
    // we no longer to prevent this from being deleted because we already have different lock for that
    page->UnregisterConsumer("DeletionLock");
    label->page = page;
    label->SetName(page->Page->PageName);
    if (page->Score <= MINIMAL_SCORE)
    {
        this->layout->addWidget(label);
    } else
    {
        int id = 0;
        if (Configuration::HuggleConfiguration->QueueNewEditsUp)
        {
            while (GetScore(id) > page->Score && GetScore(id) > MINIMAL_SCORE)
            {
                id++;
            }
        }
        else
        {
            while (GetScore(id) >= page->Score && GetScore(id) > MINIMAL_SCORE)
            {
                id++;
            }
        }
        if (id >= this->layout->count() && this->layout->count() > 0)
        {
            id = this->layout->count() - 1;
        }
        this->layout->insertWidget(id, label);
    }
    this->Items.append(label);
    HuggleQueueItemLabel::Count++;
}

void HuggleQueue::Next()
{
    if (HuggleQueueItemLabel::Count < 1)
    {
        return;
    }
    QLayoutItem *i = this->layout->itemAt(0);
    HuggleQueueItemLabel *label = (HuggleQueueItemLabel*)i->widget();
    label->Process(i);
    this->layout->removeItem(i);
    delete label;
}

WikiEdit *HuggleQueue::GetWikiEditByRevID(int RevID)
{
    int c = 0;
    while (c < this->Items.count())
    {
        HuggleQueueItemLabel *item = this->Items.at(c);
        if (item->page->RevID == RevID)
        {
            return item->page;
        }
        c++;
    }
    return NULL;
}

void HuggleQueue::DeleteByRevID(int RevID)
{
    int c = 0;
    while (c < this->Items.count())
    {
        HuggleQueueItemLabel *item = this->Items.at(c);
        if (item->page->RevID == RevID)
        {
            if (Core::HuggleCore->Main->CurrentEdit == item->page)
            {
                // we can't delete item that is being reviewed now
                return;
            }
            this->DeleteItem(item);
            return;
        }
        c++;
    }
}

void HuggleQueue::Sort()
{
    int c = 0;
    while (c < this->layout->count() - 1)
    {
        QLayoutItem *i = this->layout->itemAt(c);
        this->ResortItem(i, c);
        c++;
    }
}

void HuggleQueue::SortItemByEdit(WikiEdit *e)
{
    int c = 0;
    while (c < this->layout->count() - 1)
    {
        QLayoutItem *i = this->layout->itemAt(c);
        HuggleQueueItemLabel *x = (HuggleQueueItemLabel*)i->widget();
        if (x->page == e)
        {
            this->ResortItem(i, c);
            return;
        }
        c++;
    }
}

void HuggleQueue::ResortItem(QLayoutItem *item, int position)
{
    if (position < 0)
    {
        // we don't know the position so we need to calculate it
        position = 0;
        while (position < this->layout->count() - 1)
        {
            if (item == this->layout->itemAt(position))
            {
                break;
            }
            position++;
        }
        if (position == this->layout->count() - 1)
        {
            Syslog::HuggleLogs->DebugLog("Unable to sort the queue because item wasn't present");
            return;
        }
    }

    // first we get the item
    HuggleQueueItemLabel *q1 = (HuggleQueueItemLabel*)item->widget();
    int Score = q1->page->Score;
    int x = 0;
    bool sorted = true;
    while (x < position)
    {
        // every item on left side has to be lower than this one
        // get the item by index and compare the size
        QLayoutItem *l2 = this->layout->itemAt(x);
        if (l2 != item)
        {
            HuggleQueueItemLabel *q2 = (HuggleQueueItemLabel*)l2->widget();
            if (q2->page->Score < Score)
            {
                sorted = false;
                break;
            }
        }
        x++;
    }
    while (x < this->layout->count() - 1)
    {
        // every item on right side has to be bigger than this one
        // get the item by index and compare the size
        QLayoutItem *l2 = this->layout->itemAt(x);
        if (l2 != item)
        {
            HuggleQueueItemLabel *q2 = (HuggleQueueItemLabel*)l2->widget();
            if (q2->page->Score > Score)
            {
                sorted = false;
                break;
            }
        }
        x++;
    }
    if (!sorted)
    {
        // now we need to remove the item and place it again
        // because QT doesn't allow manual insertion of item for unknown reasons, we need to readd whole item
        WikiEdit *page = q1->page;
        page->RegisterConsumer("HuggleQueue::ResortItem");
        this->DeleteItem(q1);
        this->AddItem(page);
        page->UnregisterConsumer("HuggleQueue::ResortItem");
    }
}

void HuggleQueue::Delete(HuggleQueueItemLabel *item, QLayoutItem *qi)
{
    if (item == NULL)
    {
        throw new Exception("HuggleQueueItemLabel *item must not be NULL in this context", "void HuggleQueue::Delete(HuggleQueueItemLabel *item, QLayoutItem *qi)");
    }
    if (qi != NULL)
    {
        item->page->UnregisterConsumer(HUGGLECONSUMER_QUEUE);
        item->page = NULL;
        this->layout->removeItem(qi);
        return;
    }
    int curr=0;
    while(curr<(this->layout->count()-1))
    {
        QLayoutItem *i = this->layout->itemAt(curr);
        HuggleQueueItemLabel *label = (HuggleQueueItemLabel*)i->widget();
        if (label == item)
        {
            this->layout->removeItem(i);
            if (label->page != NULL)
            {
                label->page->UnregisterConsumer(HUGGLECONSUMER_QUEUE);
                label->page = NULL;
            }
            break;
        }
        curr++;
    }
}

void HuggleQueue::Trim(int i)
{
    if (i < 1)
    {
        throw new Huggle::Exception("Parameter i must be greater than 0 in HuggleQueue::Trim(i)");
    }

    while (i > 0)
    {
        Trim();
        i--;
    }
}

void HuggleQueue::Trim()
{
    if (HuggleQueueItemLabel::Count < 1)
    {
        return;
    }
    int x = this->layout->count() - 1;
    QLayoutItem *i = this->layout->itemAt(x);
    if (i->widget() == this->frame)
    {
        x--;
        i = this->layout->itemAt(x);
    }
    HuggleQueueItemLabel *label = (HuggleQueueItemLabel*)i->widget();
    label->Remove();
    this->layout->removeItem(i);
    delete label;
}

void HuggleQueue::Filters()
{
    this->ui->comboBox->clear();
    int x = 0;
    int id = 0;
    while (x < HuggleQueueFilter::Filters.count())
    {
        HuggleQueueFilter *FilthyFilter = HuggleQueueFilter::Filters.at(x);
        if (this->CurrentFilter == FilthyFilter)
        {
            id = x;
        }
        x++;
        this->ui->comboBox->addItem(FilthyFilter->QueueName);
    }
    this->ui->comboBox->setCurrentIndex(id);
}

long HuggleQueue::GetScore(int id)
{
    if (this->layout->count() - 1 <= id)
    {
        return MINIMAL_SCORE;
    }

    QLayoutItem *i = this->layout->itemAt(id);
    HuggleQueueItemLabel *label = (HuggleQueueItemLabel*)i->widget();
    if (label->page == NULL)
    {
        return MINIMAL_SCORE;
    }
    return label->page->Score;
}

void HuggleQueue::DeleteItem(HuggleQueueItemLabel *item)
{
    HuggleQueueItemLabel::Count--;
    item->close();
    this->Delete(item);
    this->Items.removeAll(item);
    delete item;
}

void HuggleQueue::on_comboBox_currentIndexChanged(int index)
{
    if (index > -1 && index < HuggleQueueFilter::Filters.count())
    {
        this->CurrentFilter = HuggleQueueFilter::Filters.at(index);
    }
}

