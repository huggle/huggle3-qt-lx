//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WIKIUSER_H
#define WIKIUSER_H

#include <QString>

class WikiUser
{
public:
    WikiUser();
    WikiUser(WikiUser *u);
    WikiUser(const WikiUser& u);
    WikiUser(QString user);
    QString Username;
    bool IP;
};

#endif // WIKIUSER_H
