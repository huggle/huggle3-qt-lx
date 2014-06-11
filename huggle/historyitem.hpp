//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HISTORYITEM_HPP
#define HISTORYITEM_HPP

#include "definitions.hpp"
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QString>
#include "collectable.hpp"

namespace Huggle
{
    //! Types of history items
    enum HistoryType
    {
        HistoryUnknown,
        HistoryEdit,
        HistoryRollback,
        HistoryMessage
    };

    //! History consist of these items
    class HistoryItem : public Collectable
    {
        public:
            static QString TypeToString(HistoryType type);
            static long Last;

            HistoryItem();
            HistoryItem(const HistoryItem &item);
            HistoryItem(HistoryItem * item);
            //! Unique ID of this item
            long ID;
            HistoryItem *UndoDependency = nullptr;
            QString UndoRevBaseTime;
            QString Result;
            QString Target;
            //! Change this to false in case that item can't be reverted
            bool IsRevertable = true;
            bool NewPage = false;
            //! Type of item
            HistoryType Type;
            bool Undone = false;
            HistoryItem *ReferencedBy = nullptr;
    };
}

#endif // HISTORYITEM_HPP
