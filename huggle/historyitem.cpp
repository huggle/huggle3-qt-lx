//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "exception.hpp"
#include "historyitem.hpp"

using namespace Huggle;
long HistoryItem::Last = 0;
HistoryItem::HistoryItem()
{
    HistoryItem::Last++;
    this->Target = "Unknown target";
    this->UndoRevBaseTime = "";
    this->Type = HistoryUnknown;
    this->ID = HistoryItem::Last;
    this->Result = "Unknown??";
}

HistoryItem::HistoryItem(const HistoryItem &item) : MediaWikiObject(item)
{
    this->ID = item.ID;
    this->Target = item.Target;
    this->NewPage = item.NewPage;
    this->UndoRevBaseTime = item.UndoRevBaseTime;
    this->Type = item.Type;
    this->UndoDependency = item.UndoDependency;
    this->Result = item.Result;
    this->IsRevertable = item.IsRevertable;
}

HistoryItem::HistoryItem(HistoryItem *item) : MediaWikiObject(item)
{
    if (item == nullptr)
    {
        throw new Huggle::Exception("HistoryItem item must not be NULL", "HistoryItem::HistoryItem(HistoryItem *item)");
    }
    this->ID = item->ID;
    this->Type = item->Type;
    this->NewPage = item->NewPage;
    this->IsRevertable = item->IsRevertable;
    this->UndoRevBaseTime = item->UndoRevBaseTime;
    this->Target = item->Target;
    this->UndoDependency = item->UndoDependency;
    this->Result = item->Result;
}

QString HistoryItem::TypeToString(HistoryType type)
{
    switch (type)
    {
        case HistoryUnknown:
            return "Unknown";
        case HistoryMessage:
            return "Message";
        case HistoryEdit:
            return "Edit";
        case HistoryRollback:
            return "Rollback";
    }
    return "Unknown";
}
