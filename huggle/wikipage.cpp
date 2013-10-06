//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikipage.h"
using namespace Huggle;

WikiPage::WikiPage()
{
    this->Site = NULL;
    this->PageName = "Unknown page";
    this->Contents = "";
}

WikiPage::WikiPage(QString name)
{
    PageName = name;
    Site = NULL;
    this->Contents = "";
}

WikiPage::WikiPage(WikiPage *page)
{
    this->PageName = page->PageName;
    this->Site = page->Site;
    this->Contents = page->Contents;
}

WikiPage::WikiPage(const WikiPage &page)
{
    this->PageName = page.PageName;
    this->Site = page.Site;
    this->Contents = page.Contents;
}

bool WikiPage::IsTalk()
{
    if (PageName.contains("Talk:") || PageName.contains("talk:"))
    {
        return true;
    }
    return false;
}

bool WikiPage::IsUserpage()
{
    if (PageName.contains("User:") || PageName.contains("User:"))
    {
        return true;
    }
    return false;
}
