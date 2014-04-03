//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikipage.hpp"
using namespace Huggle;

WikiPage::WikiPage()
{
    this->Site = NULL;
    this->PageName = "Unknown page";
    this->Contents = "";
}

WikiPage::WikiPage(const QString &name)
{
    this->PageName = name;
    this->Site = NULL;
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

QString WikiPage::SanitizedName()
{
    return this->PageName.replace(" ", "_");
}

MediaWikiNS WikiPage::GetNS()
{
    if (PageName.startsWith(Configuration::HuggleConfiguration->ProjectConfig_NSTalk) || PageName.startsWith(MEDIAWIKI_DEFAULT_NS_TALK))
    {
        return MediaWikiNS_Talk;
    }
    if (PageName.startsWith(Configuration::HuggleConfiguration->ProjectConfig_NSProject) || PageName.startsWith(MEDIAWIKI_DEFAULT_NS_PROJECT))
    {
        return MediaWikiNS_Project;
    }
    if (PageName.startsWith(Configuration::HuggleConfiguration->ProjectConfig_NSProjectTalk) || PageName.startsWith(MEDIAWIKI_DEFAULT_NS_PROJECTTALK))
    {
        return MediaWikiNS_ProjectTalk;
    }
    if (PageName.startsWith(Configuration::HuggleConfiguration->ProjectConfig_NSUser) || PageName.startsWith(MEDIAWIKI_DEFAULT_NS_USER))
    {
        return MediaWikiNS_User;
    }
    if (PageName.startsWith(Configuration::HuggleConfiguration->ProjectConfig_NSUserTalk) || PageName.startsWith(MEDIAWIKI_DEFAULT_NS_USERTALK))
    {
        return MediaWikiNS_UserTalk;
    }
    if (PageName.startsWith(Configuration::HuggleConfiguration->ProjectConfig_NSCategory) || PageName.startsWith(MEDIAWIKI_DEFAULT_NS_CATEGORY))
    {
        return MediaWikiNS_Category;
    }
    if (PageName.startsWith(Configuration::HuggleConfiguration->ProjectConfig_NSCategoryTalk) || PageName.startsWith(MEDIAWIKI_DEFAULT_NS_CATEGORYTALK))
    {
        return MediaWikiNS_CategoryTalk;
    }
    if (PageName.startsWith(Configuration::HuggleConfiguration->ProjectConfig_NSFile) || PageName.startsWith(MEDIAWIKI_DEFAULT_NS_FILE))
    {
        return MediaWikiNS_File;
    }
    if (PageName.startsWith(Configuration::HuggleConfiguration->ProjectConfig_NSFileTalk) || PageName.startsWith(MEDIAWIKI_DEFAULT_NS_FILETALK))
    {
        return MediaWikiNS_FileTalk;
    }
    if (PageName.startsWith(Configuration::HuggleConfiguration->ProjectConfig_NSMediaWikiTalk) || PageName.startsWith(MEDIAWIKI_DEFAULT_NS_MEDIAWIKITALK))
    {
        return MediaWikiNS_MediawikiTalk;
    }
    if (PageName.startsWith(Configuration::HuggleConfiguration->ProjectConfig_NSMediaWiki) || PageName.startsWith(MEDIAWIKI_DEFAULT_NS_MEDIAWIKI))
    {
        return MediaWikiNS_Mediawiki;
    }
    if (PageName.startsWith(Configuration::HuggleConfiguration->ProjectConfig_NSPortal) || PageName.startsWith(MEDIAWIKI_DEFAULT_NS_PORTAL))
    {
        return MediaWikiNS_Portal;
    }
    if (PageName.startsWith(Configuration::HuggleConfiguration->ProjectConfig_NSPortalTalk) || PageName.startsWith(MEDIAWIKI_DEFAULT_NS_PORTALTALK))
    {
        return MediaWikiNS_PortalTalk;
    }
    return MediaWikiNS_Main;
}

bool WikiPage::IsTalk()
{
    MediaWikiNS NS = this->GetNS();
    if (NS == MediaWikiNS_Talk ||
            NS == MediaWikiNS_HelpTalk ||
            NS == MediaWikiNS_UserTalk ||
            NS == MediaWikiNS_CategoryTalk ||
            NS == MediaWikiNS_FileTalk ||
            NS == MediaWikiNS_MediawikiTalk ||
            NS == MediaWikiNS_ProjectTalk)
    {
        return true;
    }
    return false;
}

WikiPage *WikiPage::RetrieveTalk()
{
    MediaWikiNS NS = this->GetNS();
    if (!this->IsTalk())
    {
        return NULL;
    }
    QString prefix = "Talk:";
    switch (NS)
    {
        case MediaWikiNS_Category:
            prefix = Configuration::HuggleConfiguration->ProjectConfig_NSCategoryTalk;
            break;
        case MediaWikiNS_File:
            prefix = Configuration::HuggleConfiguration->ProjectConfig_NSFileTalk;
            break;
        case MediaWikiNS_Help:
            prefix = Configuration::HuggleConfiguration->ProjectConfig_NSHelpTalk;
            break;
        case MediaWikiNS_Mediawiki:
            prefix = Configuration::HuggleConfiguration->ProjectConfig_NSMediaWikiTalk;
            break;
        case MediaWikiNS_Portal:
            prefix = Configuration::HuggleConfiguration->ProjectConfig_NSPortalTalk;
            break;
        case MediaWikiNS_Project:
            prefix = Configuration::HuggleConfiguration->ProjectConfig_NSProjectTalk;
            break;
        case MediaWikiNS_User:
            prefix = Configuration::HuggleConfiguration->ProjectConfig_NSUserTalk;
            break;
        default:
            prefix = "";
            break;
    }
    if (prefix == "")
    {
        return NULL;
    }
    return new WikiPage(prefix + this->RootName());;
}

QString WikiPage::RootName()
{
    QString sanitized = this->PageName;
    if (this->GetNS() != MediaWikiNS_Main)
    {
        // first we need to get a colon
        if (this->PageName.contains(":"))
        {
            sanitized = sanitized.mid(sanitized.indexOf(":") + 1);
        }
    }
    if (sanitized.contains("/"))
    {
        sanitized = sanitized.mid(0, sanitized.indexOf("/"));
    }
    return sanitized;
}

bool WikiPage::IsUserpage()
{
    if (this->GetNS() == MediaWikiNS_User)
    {
        return true;
    }
    return false;
}
