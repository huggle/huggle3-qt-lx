//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikipage.hpp"
#include <QUrl>
#include "exception.hpp"
#include "wikisite.hpp"
#include "localization.hpp"
using namespace Huggle;

WikiPage::WikiPage(WikiSite *site) : MediaWikiObject(site)
{
    this->PageName = _l("page-unknown");
    this->Contents = "";
    if (!this->Site)
        throw new Huggle::NullPointerException("local Site", BOOST_CURRENT_FUNCTION);
    this->NS = this->Site->Unknown;
}

WikiPage::WikiPage(const QString &name, WikiSite *site) : MediaWikiObject(site)
{
    this->PageName = name;
    this->Contents = "";
    if (!this->Site)
        throw new Huggle::NullPointerException("local Site", BOOST_CURRENT_FUNCTION);
    this->NS = this->Site->RetrieveNSFromTitle(this->PageName);
}

WikiPage::WikiPage(WikiPage *page) : MediaWikiObject(page)
{
    this->PageName = page->PageName;
    this->Contents = page->Contents;
    this->founder = page->founder;
    this->founderKnown = page->founderKnown;
    this->hasContent = page->hasContent;
    this->NS = page->NS;
}

WikiPage::WikiPage(const WikiPage &page) : MediaWikiObject(page)
{
    this->PageName = page.PageName;
    this->Contents = page.Contents;
    this->founder = page.founder;
    this->founderKnown = page.founderKnown;
    this->NS = page.NS;
}

WikiPage *WikiPage::RetrieveTalk()
{
    if (this->IsTalk())
    {
        return nullptr;
    }
    // now we need to get a talk namespace for this ns
    QString prefix = "Talk";
    if (!this->NS->GetName().isEmpty())
    {
        prefix = this->Site->RetrieveNSByCanonicalName(this->NS->GetCanonicalName() + " talk")->GetName();
        if (!prefix.size())
        {
            prefix = "Talk";
        }
    }
    prefix += ":";
    return new WikiPage(prefix + this->RootName(), this->Site);
}

bool WikiPage::FounderKnown()
{
    return this->founderKnown;
}

QString WikiPage::GetFounder()
{
    return this->founder;
}

bool WikiPage::EqualTo(WikiPage *page)
{
    return this->Site == page->Site && this->SanitizedName() == page->SanitizedName();
}

void WikiPage::SetFounder(QString name)
{
    this->founderKnown = true;
    this->founder = name;
}

QString WikiPage::RootName()
{
    QString sanitized = this->PageName;
    if (!this->GetNS()->GetName().isEmpty())
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

QString WikiPage::EncodedName()
{
    return QUrl::toPercentEncoding(this->PageName);
}

QString WikiPage::GetContent()
{
    return this->Contents;
}

void WikiPage::SetContent(QString content)
{
    this->hasContent = true;
    this->Contents = content;
}

bool WikiPage::HasContent()
{
    return this->hasContent;
}

bool WikiPage::IsTalk()
{
    return this->NS->IsTalkPage();
}

bool WikiPage::IsUserpage()
{
    return this->GetNS()->GetCanonicalName() == "User";
}


