//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WIKIPAGE_H
#define WIKIPAGE_H

#include <QString>
#include "configuration.h"
#include "wikisite.h"

class WikiPage
{
public:
    WikiPage();
    WikiPage(QString name);
    WikiPage(WikiPage *page);
    WikiPage(const WikiPage& page);
    QString PageName;
    WikiSite *Site;
    bool IsTalk();
    bool IsUserpage();
};

#endif // WIKIPAGE_H
