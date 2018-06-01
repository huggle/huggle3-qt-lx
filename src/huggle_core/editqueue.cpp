//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editqueue.hpp"
#include "apiquery.hpp"
#include "apiqueryresult.hpp"
#include "configuration.hpp"
#include "querypool.hpp"
#include "syslog.hpp"
#include "wikiutil.hpp"
#include "wikiedit.hpp"
#include "wikiuser.hpp"
#include "wikisite.hpp"
#include <QUrl>

using namespace Huggle;

EditQueue *EditQueue::Primary = nullptr;

EditQueue::EditQueue()
{

}

EditQueue::~EditQueue()
{
    qDeleteAll(this->newUnprocessedEditsByRevID);
    this->newUnprocessedEditsByRevID.clear();
}

void EditQueue::AddItem(WikiEdit *page)
{

}

int EditQueue::DeleteByScore(long Score)
{
    return -1;
}

bool EditQueue::DeleteByRevID(revid_ht RevID, WikiSite *site)
{
    return false;
}

void EditQueue::DeleteOlder(WikiEdit *edit)
{

}

void EditQueue::UpdateUser(WikiUser *user)
{

}

void EditQueue::newEditByRevID_Success(WikiEdit *edit, void *source, QString error)
{
    EditQueue_UnprocessedEdit *new_edit = (EditQueue_UnprocessedEdit*)source;
    new_edit->Edit = edit;
    edit->PostprocessCallback = (WEPostprocessedCallback) EditQueue::newEditByRevID_Finish;
    edit->PostprocessCallback_Owner = new_edit;
    // Request a post processing of this edit
    QueryPool::HugglePool->PostProcessEdit(edit);
}

void EditQueue::newEditByRevID_Fail(WikiEdit *edit, void *source, QString error)
{
    EditQueue_UnprocessedEdit *new_edit = (EditQueue_UnprocessedEdit*)source;
    HUGGLE_ERROR(new_edit->GetSite()->Name + ": unable to insert edit to queue by ID: " + error);
    new_edit->Remove();
    delete new_edit;
}

void EditQueue::newEditByRevID_Finish(WikiEdit *edit)
{
    EditQueue_UnprocessedEdit *owner = (EditQueue_UnprocessedEdit*)edit->PostprocessCallback_Owner;
    edit->PostprocessCallback_Owner = nullptr;
    owner->Parent->AddItem(edit);
    owner->Remove();
    delete owner;
}

bool EditQueue::checkIfNewEditByRevIDExists(revid_ht rev_id, WikiSite *site)
{
    foreach (EditQueue_UnprocessedEdit *edit, this->newUnprocessedEditsByRevID)
    {
        if (edit->GetSite() == site && edit->GetID() == rev_id)
            return true;
    }
    return false;
}

void EditQueue::cleanupUnprocessedEdit(EditQueue_UnprocessedEdit *edit)
{
    this->newUnprocessedEditsByRevID.removeAll(edit);
}

void EditQueue::AddUnprocessedEditFromRevID(revid_ht rev_id, WikiSite *site)
{
    // First check if this edit is already in queue or not
    if (this->GetWikiEditByRevID(rev_id, site) != nullptr)
    {
        HUGGLE_DEBUG1("Not inserting edit " + QString::number(rev_id) + " to queue, because it's already there");
        return;
    }
    // Check if this edit was already requested in past but not processed yet
    if (this->checkIfNewEditByRevIDExists(rev_id, site))
    {
        HUGGLE_DEBUG1("Not inserting edit " + QString::number(rev_id) + " to queue, because it's already being retrieved");
        return;
    }
    EditQueue_UnprocessedEdit *edit = new EditQueue_UnprocessedEdit(site, rev_id, this);
    this->newUnprocessedEditsByRevID.append(edit);

    // Get the info
    WikiUtil::RetrieveEditByRevid(rev_id, site, edit,
                                  (WikiUtil::RetrieveEditByRevid_Callback)EditQueue::newEditByRevID_Success,
                                  (WikiUtil::RetrieveEditByRevid_Callback)EditQueue::newEditByRevID_Fail);
}

EditQueue_UnprocessedEdit::EditQueue_UnprocessedEdit(WikiSite *site, revid_ht rev_id, EditQueue *parent) : MediaWikiObject(site)
{
    this->Parent = parent;
    this->revID = rev_id;
}

revid_ht EditQueue_UnprocessedEdit::GetID()
{
    return this->revID;
}

void EditQueue_UnprocessedEdit::Remove()
{
    this->Parent->cleanupUnprocessedEdit(this);
}

bool EditQueue_UnprocessedEdit::IsPostProcessing()
{
    if (this->Edit == nullptr)
        return false;
    return this->Edit->Status != WEStatus::StatusPostProcessed;
}
