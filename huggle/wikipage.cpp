//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikipage.h"

WikiPage::WikiPage()
{
    Site = NULL;
}

WikiPage::WikiPage(QString name)
{
    PageName = name;
    Site = NULL;
}

WikiPage::WikiPage(WikiPage *page)
{
    this->PageName = page->PageName;
    this->Site = page->Site;
}

WikiPage::WikiPage(const WikiPage &page)
{
    this->PageName = page.PageName;
    this->Site = page.Site;
}

bool WikiPage::IsTalk()
{
    if (PageName.contains("Talk:") || PageName.contains("talk:"))
    {
        return true;
    }
    return false;
}
