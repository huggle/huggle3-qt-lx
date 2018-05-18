//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef EDITTYPE_HPP
#define EDITTYPE_HPP

namespace Huggle
{
    //! This is a "type" of an edit which we use to resolve the proper icon for the edit
    enum EditType
    {
        EditType_Normal,
        EditType_Anon,
        EditType_1,
        EditType_2,
        EditType_3,
        EditType_4,
        EditType_W,
        EditType_Blocked,
        EditType_Revert,
        EditType_Reported,
        EditType_Bot,
        EditType_New,
        EditType_Self
    };
}

#endif // EDITTYPE_HPP
