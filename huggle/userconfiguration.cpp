//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "userconfiguration.hpp"

Huggle::UserConfiguration::~UserConfiguration()
{
    QStringList ol = this->UserOptions.keys();
    while (ol.count())
    {
        HuggleOption *option = this->UserOptions[ol[0]];
        this->UserOptions.remove(ol[0]);
        delete option;
        ol.removeAt(0);
    }
}
