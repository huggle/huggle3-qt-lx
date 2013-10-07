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

MediaWikiNS WikiPage::GetNS()
{
    if (PageName.startsWith(Configuration::LocalConfig_NSTalk) ||
            PageName.startsWith(MEDIAWIKI_DEFAULT_NS_TALK))
    {
        return MediaWikiNS_Talk;
    }
    if (PageName.startsWith(Configuration::LocalConfig_NSProject) || PageName.startsWith(MEDIAWIKI_DEFAULT_NS_PROJECT))
    {
        return MediaWikiNS_Project;
    }
    if (PageName.startsWith(Configuration::LocalConfig_NSUser) ||
            PageName.startsWith(MEDIAWIKI_DEFAULT_NS_USER))
    {
        return MediaWikiNS_User;
    }
    if (PageName.startsWith(Configuration::LocalConfig_NSUserTalk) ||
            PageName.startsWith(MEDIAWIKI_DEFAULT_NS_USERTALK))
    {
        return MediaWikiNS_UserTalk;
    }
    return MediaWikiNS_Main;
}

bool WikiPage::IsTalk()
{
    MediaWikiNS NS = this->GetNS();
    if (NS == MediaWikiNS_Talk || NS == MediaWikiNS_HelpTalk || NS == MediaWikiNS_UserTalk)
    {
        return true;
    }
    return false;
}

bool WikiPage::IsUserpage()
{
    if (this->GetNS() == MediaWikiNS_User)
    {
        return true;
    }
    return false;
}
