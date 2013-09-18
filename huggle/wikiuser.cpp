//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikiuser.h"

QRegExp WikiUser::IPv4Regex("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$");

WikiUser::WikiUser()
{
    this->Username = "";
    this->IP = true;
}

WikiUser::WikiUser(WikiUser *u)
{
    this->IP = u->IP;
    this->Username = u->Username;
}

WikiUser::WikiUser(const WikiUser &u)
{
    this->IP = u.IP;
    this->Username = u.Username;
}

WikiUser::WikiUser(QString user)
{
    this->IP = WikiUser::IPv4Regex.exactMatch(user);
    this->Username = user;
}
