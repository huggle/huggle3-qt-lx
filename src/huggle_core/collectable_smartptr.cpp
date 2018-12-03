// IMPORTANT: this file has a different license than rest of huggle

//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2014 - 2018

#include "collectable.hpp"
#include "collectable_smartptr.hpp"
#include "apiquery.hpp"
#include "editquery.hpp"
#include "exception.hpp"
#include "gc.hpp"
#include "revertquery.hpp"
#include "webserverquery.hpp"
#include "wlquery.hpp"

// Required by MSVC
#ifdef HUGGLE_WIN
namespace Huggle
{
    template class Collectable_SmartPtr<Query>;
    template class Collectable_SmartPtr<RevertQuery>;
    template class Collectable_SmartPtr<EditQuery>;
    template class Collectable_SmartPtr<WikiEdit>;
    template class Collectable_SmartPtr<WebserverQuery>;
    template class Collectable_SmartPtr<HistoryItem>;
    template class Collectable_SmartPtr<WLQuery>;
    template class Collectable_SmartPtr<ApiQuery>;
}
#endif
