//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editqueue.hpp"

using namespace Huggle;

EditQueue *EditQueue::Primary = nullptr;

EditQueue::EditQueue()
{

}

EditQueue::~EditQueue()
{

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
