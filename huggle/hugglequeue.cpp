//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglequeue.hpp"
#include "mainwindow.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "huggleprofiler.hpp"
#include "hugglequeueitemlabel.hpp"
#include "vandalnw.hpp"
#include "localization.hpp"
#include "ui_hugglequeue.h"
#include "syslog.hpp"
#include "wikipage.hpp"
#include "wikiuser.hpp"
#include "wikisite.hpp"

using namespace Huggle;

HuggleQueue::HuggleQueue(QWidget *parent) : QDockWidget(parent), ui(new Ui::HuggleQueue)
{
    this->ui->setupUi(this);
    this->setWindowTitle(_l("main-queue"));
    this->Filters();
}

HuggleQueue::~HuggleQueue()
{
    delete this->ui;
}

void HuggleQueue::AddItem(WikiEdit *page)
{
    if (page == nullptr)
        throw new Huggle::NullPointerException("WikiEdit *page", BOOST_CURRENT_FUNCTION);

    if (!page->IsValid)
    {
        HUGGLE_DEBUG1("Not inserting edit " + page->Page->PageName + " because it's broken");
        return;
    }

    page->RegisterConsumer(HUGGLECONSUMER_QUEUE);
    if (MainWindow::HuggleMain != nullptr)
    {
        if (MainWindow::HuggleMain->VandalDock != nullptr)
        {
            if (MainWindow::HuggleMain->VandalDock->IsParsed(page))
            {
                // we don't even need to insert this page to queue
                page->UnregisterConsumer(HUGGLECONSUMER_QUEUE);
                return;
            }
            MainWindow::HuggleMain->VandalDock->Rescore(page);
        }
    }
    // in case that we don't want to have this edit in queue, we can ignore this
    if (Configuration::HuggleConfiguration->UserConfig->DeleteEditsAfterRevert)
    {
        // check if there was a revert to this edit which is newer than itself
        int i = 0;
        WikiEdit::Lock_EditList->lock();
        while (i < Huggle::WikiEdit::EditList.count())
        {
            // retrieve the edit
            WikiEdit *edit = Huggle::WikiEdit::EditList.at(i);
            i++;
            // if this is a same edit we can go next
            if (edit == page)
                continue;
            // if this edit is not newer we can continue
            if (edit->RevID < page->RevID)
                continue;
            // if edit is not a revert we can continue
            if (!edit->IsRevert)
                continue;
            // if edit is not made to this page, we can continue
            if (edit->Page->PageName != page->Page->PageName)
                continue;
            // we found it
            HUGGLE_DEBUG("Ignoring edit to " + page->Page->PageName + " because it was reverted by someone", 1);
            WikiEdit::Lock_EditList->unlock();
            page->UnregisterConsumer(HUGGLECONSUMER_QUEUE);
            return;
        }
        WikiEdit::Lock_EditList->unlock();
    }
    if (Configuration::HuggleConfiguration->UserConfig->TruncateEdits)
    {
        // if we want to keep only newest edits in queue we can remove all older edits made to this page
        this->DeleteOlder(page);
    }
    // so we need to insert the item somehow
    HuggleQueueItemLabel *label = new HuggleQueueItemLabel(this);
    // we create a new label here
    label->Edit = page;
    label->SetName(page->Page->PageName);
    if (page->Score <= MINIMAL_SCORE)
    {
        if (this->ui->itemList->count() == 0)
        {
            // this should never happen - if there were 0 items in a queue it means there is no spacer, let's crash here
            throw Huggle::Exception("The container must have at least one spacer", BOOST_CURRENT_FUNCTION);
        }
        this->ui->itemList->insertWidget(this->ui->itemList->count() - 1, label);
    } else
    {
        int id = 0;
        if (Configuration::HuggleConfiguration->SystemConfig_QueueNewEditsUp)
        {
            long score = GetScore(id);
            while (score > page->Score && score > MINIMAL_SCORE)
            {
                score = GetScore(id);
                id++;
            }
        }
        else
        {
            long score = GetScore(id);
            while (score >= page->Score && score > MINIMAL_SCORE)
            {
                id++;
                score = GetScore(id);
            }
        }
        if (id >= this->ui->itemList->count() && this->ui->itemList->count() > 0)
        {
            id = this->ui->itemList->count() - 1;
        }
        this->ui->itemList->insertWidget(id, label);
    }
    this->Items.append(label);
    this->RedrawTitle();
}

void HuggleQueue::Next()
{
    if (this->Items.count() < 1)
    {
        // there are no items in a list
        return;
    }
    QLayoutItem *i = this->ui->itemList->itemAt(0);
    if (i == this->ui->verticalSpacer)
    {
        // this should never happen
        HUGGLE_DEBUG("Reached spacer", 1);
        return;
    }
    HuggleQueueItemLabel *label = (HuggleQueueItemLabel*)i->widget();
    label->Process(i);
}

WikiEdit *HuggleQueue::GetWikiEditByRevID(revid_ht RevID, WikiSite *site)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    int c = 0;
    while (c < this->Items.count())
    {
        HuggleQueueItemLabel *item = this->Items.at(c);
        if (item->Edit->RevID == RevID && item->Edit->GetSite() == site)
        {
            return item->Edit;
        }
        c++;
    }
    return nullptr;
}

bool HuggleQueue::DeleteByRevID(revid_ht RevID, WikiSite *site)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    int c = 0;
    while (c < this->Items.count())
    {
        HuggleQueueItemLabel *item = this->Items.at(c);
        if (item->Edit->RevID == RevID && item->Edit->GetSite() == site)
        {
            if (MainWindow::HuggleMain->CurrentEdit == item->Edit)
            {
                // we can't delete item that is being reviewed now
                return false;
            }
            return this->DeleteItem(item);
        }
        c++;
    }
    // we didn't find it
    return false;
}

void HuggleQueue::Sort()
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    int c = 0;
    // we need to make sure that we won't touch the expander we have in list
    while (c < this->ui->itemList->count() - 1)
    {
        QLayoutItem *i = this->ui->itemList->itemAt(c);
        this->ResortItem(i, c);
        c++;
    }
}

void HuggleQueue::SortItemByEdit(WikiEdit *e)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    int c = 0;
    while (c < this->ui->itemList->count() - 1)
    {
        QLayoutItem *i = this->ui->itemList->itemAt(c);
        HuggleQueueItemLabel *x = (HuggleQueueItemLabel*)i->widget();
        if (x && x->Edit == e)
        {
            this->ResortItem(i, c);
            return;
        }
        c++;
    }
}

void HuggleQueue::ResortItem(QLayoutItem *item, int position)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (position < 0)
    {
        // we don't know the position so we need to calculate it
        position = 0;
        while (position < this->ui->itemList->count() - 1)
        {
            if (item == this->ui->itemList->itemAt(position))
            {
                break;
            }
            position++;
        }
        if (position == this->ui->itemList->count() - 1)
        {
            HUGGLE_DEBUG("Unable to sort the queue because item wasn't present", 1);
            return;
        }
    }

    // first we get the item
    HuggleQueueItemLabel *q1 = (HuggleQueueItemLabel*)item->widget();
    int Score = q1->Edit->Score;
    int x = 0;
    bool sorted = true;
    while (x < position)
    {
        // every item on left side has to be lower than this one
        // get the item by index and compare the size
        QLayoutItem *l2 = this->ui->itemList->itemAt(x);
        if (l2 != item)
        {
            HuggleQueueItemLabel *q2 = (HuggleQueueItemLabel*)l2->widget();
            if (q2->Edit->Score < Score)
            {
                sorted = false;
                break;
            }
        }
        x++;
    }
    while (x < this->ui->itemList->count() - 1)
    {
        // every item on right side has to be bigger than this one
        // get the item by index and compare the size
        QLayoutItem *l2 = this->ui->itemList->itemAt(x);
        if (l2 != item)
        {
            HuggleQueueItemLabel *q2 = (HuggleQueueItemLabel*)l2->widget();
            // we need to check here if we accidentaly didn't get some
            // element that actually isn't an item
            if (q2 != nullptr && q2->Edit->Score > Score)
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
        WikiEdit *page = q1->Edit;
        page->RegisterConsumer("HuggleQueue::ResortItem");
        this->DeleteItem(q1);
        this->AddItem(page);
        page->UnregisterConsumer("HuggleQueue::ResortItem");
    }
}

bool HuggleQueue::DeleteItem(HuggleQueueItemLabel *item)
{
    int removed = this->Items.removeAll(item);
    this->Delete(item);
    if (removed > 0)
    {
        this->RedrawTitle();
        return true;
    }
    return false;
}

void HuggleQueue::Delete(HuggleQueueItemLabel *item, QLayoutItem *qi)
{
    if (item == nullptr)
    {
        throw new Huggle::NullPointerException("HuggleQueueItemLabel *item", BOOST_CURRENT_FUNCTION);
    }
    if (qi != nullptr)
    {
        item->Edit->UnregisterConsumer(HUGGLECONSUMER_QUEUE);
        item->Edit = nullptr;
        this->ui->itemList->removeItem(qi);
        delete qi;
        delete item;
        return;
    }
    int curr=0;
    while(curr<(this->ui->itemList->count()-1))
    {
        QLayoutItem *i = this->ui->itemList->itemAt(curr);
        HuggleQueueItemLabel *label = (HuggleQueueItemLabel*)i->widget();
        if (label == item)
        {
            this->ui->itemList->removeItem(i);
            if (label->Edit != nullptr)
            {
                label->Edit->UnregisterConsumer(HUGGLECONSUMER_QUEUE);
                label->Edit = nullptr;
            }
            delete i;
            delete item;
            return;
        }
        curr++;
    }
}

int HuggleQueue::DeleteByScore(long Score)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    int result = 0;
    int c = 0;
    while (c < this->Items.count())
    {
        HuggleQueueItemLabel *item = this->Items.at(c);
        if (item->Edit->Score < Score)
        {
            if (MainWindow::HuggleMain->CurrentEdit == item->Edit)
            {
                // we can't delete item that is being reviewed now
                c++;
                continue;
            }
            if (this->DeleteItem(item))
            {
                // we sucessfuly deleted the item
                result++;
                continue;
            } else
            {
                c++;
                continue;
            }
        }
        c++;
    }
    this->RedrawTitle();
    return result;
}

void HuggleQueue::Trim(unsigned int i)
{
    while (i > 0)
    {
        Trim();
        i--;
    }
}

void HuggleQueue::Trim()
{
    if (this->ui->itemList->count() <= 1)
    {
        return;
    }
    int x = this->ui->itemList->count() - 2;
    QLayoutItem *i = this->ui->itemList->itemAt(x);
    if (i == this->ui->verticalSpacer)
    {
        throw new Huggle::Exception("Vertical spacer was not last in queue", BOOST_CURRENT_FUNCTION);
    }
    HuggleQueueItemLabel *label = (HuggleQueueItemLabel*)i->widget();
    label->Remove();
}

void HuggleQueue::Filters()
{
    this->loading = true;
    this->ui->comboBox->clear();
    int x = 0;
    int id = 0;
    if (!HuggleQueueFilter::Filters.contains(this->CurrentSite()))
        throw new Huggle::Exception("The filter list doesn't contain current site", BOOST_CURRENT_FUNCTION);
    while (x < HuggleQueueFilter::Filters[this->CurrentSite()]->count())
    {
        HuggleQueueFilter *FilthyFilter = HuggleQueueFilter::Filters[this->CurrentSite()]->at(x);
        if (this->CurrentSite()->UserConfig->QueueID == FilthyFilter->QueueName)
        {
            id = x;
        }
        x++;
        this->ui->comboBox->addItem(FilthyFilter->QueueName);
    }
    this->loading = false;
    this->ui->comboBox->setCurrentIndex(id);
}

void HuggleQueue::DeleteOlder(WikiEdit *edit)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    int i = 0;
    while (i < this->Items.count())
    {
        WikiEdit *_e = this->Items.at(i)->Edit;
        if (edit->RevID > _e->RevID)
        {
            if (edit->Page->PageName == _e->Page->PageName)
            {
                HUGGLE_DEBUG("Deleting old edit to page " + _e->Page->PageName, 3);
                // remove it
                if (this->DeleteByRevID(_e->RevID, _e->GetSite()))
                {
                    // we can only continue if some edit was deleted
                    // otherwise we end up looping here
                    continue;
                }
            }
        }
        i++;
    }
    this->RedrawTitle();
}

void HuggleQueue::UpdateUser(WikiUser *user)
{
    HUGGLE_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    int i = 0;
    while (i < this->Items.count())
    {
        WikiEdit *ed = this->Items.at(i)->Edit;
        if (ed->User->Username == user->Username)
        {
            // we have a match, let's update the icon, but only if the levels are actually different for performance reasons
            if (ed->User->GetWarningLevel() != user->GetWarningLevel())
            {
                ed->User->SetWarningLevel(user->GetWarningLevel());
                this->Items.at(i)->UpdatePixmap();
            }
        }
        i++;
    }
}

void HuggleQueue::Clear()
{
    // now we need to remove all items
    while (this->Items.count() > 0)
    {
        if (!this->DeleteItem(this->Items.at(0)))
        {
            // we failed to remove the item, break or we end up
            // looping here
            HUGGLE_DEBUG("Failed to clear the queue", 1);
            return;
        }
    }
    this->RedrawTitle();
}

void HuggleQueue::RedrawTitle()
{
    this->setWindowTitle(_l("main-queue") + "[" + QString::number(this->Items.count()) + "]");
}

WikiSite *HuggleQueue::CurrentSite()
{
    if (this->Site == nullptr)
        return hcfg->Project;
    return this->Site;
}

void HuggleQueue::ChangeSite(WikiSite *site)
{
    if (this->CurrentSite() != site)
    {
        // let's switch the list of queues
        this->Site = site;
        this->Filters();
    }
}

long HuggleQueue::GetScore(int id)
{
    if (this->ui->itemList->count() - 1 <= id)
    {
        return MINIMAL_SCORE;
    }

    QLayoutItem *i = this->ui->itemList->itemAt(id);
    HuggleQueueItemLabel *label = (HuggleQueueItemLabel*)i->widget();
    if (!label)
    {
        throw new Huggle::NullPointerException("label was nullptr", BOOST_CURRENT_FUNCTION);
    }
    if (label->Edit == nullptr)
    {
        return MINIMAL_SCORE;
    }
    return label->Edit->Score;
}

void HuggleQueue::on_comboBox_currentIndexChanged(int index)
{
    if (!this->loading)
    {
        if (!HuggleQueueFilter::Filters.contains(this->CurrentSite()))
            throw new Huggle::Exception("The filter list doesn't contain site", BOOST_CURRENT_FUNCTION);
        if (index > -1 && index < HuggleQueueFilter::Filters[this->CurrentSite()]->count())
        {
            this->CurrentSite()->CurrentFilter = HuggleQueueFilter::Filters[this->CurrentSite()]->at(index);
            hcfg->UserConfig->QueueID = this->CurrentSite()->CurrentFilter->QueueName;
        }
    }
}

