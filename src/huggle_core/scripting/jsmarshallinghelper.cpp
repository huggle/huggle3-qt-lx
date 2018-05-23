//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "jsmarshallinghelper.hpp"
#include "../generic.hpp"
#include "../version.hpp"
#include "../wikiedit.hpp"
#include "../wikipage.hpp"
#include "../wikiuser.hpp"
#include "../wikisite.hpp"
#include "../apiquery.hpp"
#include "../apiqueryresult.hpp"

using namespace Huggle;

QJSValue JSMarshallingHelper::FromSite(WikiSite *site, QJSEngine *engine)
{
    if (!site)
        return QJSValue(false);
    QJSValue o = engine->newObject();
    o.setProperty("Name", QJSValue(site->Name));
    o.setProperty("ForceSSL", QJSValue(site->ForceSSL));
    o.setProperty("HANChannel", QJSValue(site->HANChannel));
    o.setProperty("IRCChannel", QJSValue(site->IRCChannel));
    o.setProperty("IsRightToLeft", QJSValue(site->IsRightToLeft));
    o.setProperty("LongPath", QJSValue(site->LongPath));
    o.setProperty("MediawikiVersion", FromVersion(&site->MediawikiVersion, engine));
    o.setProperty("ScriptPath", QJSValue(site->ScriptPath));
    o.setProperty("SupportHttps", QJSValue(site->SupportHttps));
    o.setProperty("URL", QJSValue(site->URL));
    return o;
}

QJSValue JSMarshallingHelper::FromUser(WikiUser *user, QJSEngine *engine)
{
    if (!user)
        return QJSValue(false);
    QJSValue o = engine->newObject();
    o.setProperty("EditCount", QJSValue(static_cast<int>(user->EditCount)));
    o.setProperty("Username", QJSValue(user->Username));
    // CPU expensive
    // o.setProperty("Flags", QScriptValue(engine, user->Flags()));
    o.setProperty("BadnessScore", QJSValue(static_cast<int>(user->GetBadnessScore())));
    o.setProperty("Site", FromSite(user->GetSite(), engine));
    o.setProperty("TalkPageName", QJSValue(user->GetTalk()));
    o.setProperty("UserPageName", QJSValue(user->GetUserPage()));
    o.setProperty("WarningLevel", QJSValue(static_cast<int>(user->GetWarningLevel())));
    o.setProperty("IsBlocked", QJSValue(user->IsBlocked));
    o.setProperty("IsBot", QJSValue(user->IsBot()));
    o.setProperty("IsIP", QJSValue(user->IsIP()));
    o.setProperty("IsReported", QJSValue(user->IsReported));
    o.setProperty("IsWhitelisted", QJSValue(user->IsWhitelisted()));
    o.setProperty("RegistrationDate", QJSValue(user->RegistrationDate));
    o.setProperty("Site", FromSite(user->Site, engine));
    o.setProperty("TalkPageContents", QJSValue(user->TalkPage_GetContents()));
    o.setProperty("TalkPage_RetrievalTime", QJSValue(user->TalkPage_RetrievalTime().toString()));
    return o;
}

QJSValue JSMarshallingHelper::FromEdit(WikiEdit *edit, QJSEngine *engine)
{
    if (!edit)
        return QJSValue(false);
    QJSValue o = engine->newObject();
    o.setProperty("Bot", QJSValue(edit->Bot));
    o.setProperty("CurrentUserWarningLevel", QJSValue(static_cast<int>(edit->CurrentUserWarningLevel)));
    o.setProperty("Diff", QJSValue(static_cast<int>(edit->Diff)));
    o.setProperty("DiffText", QJSValue(edit->DiffText));
    o.setProperty("DiffText_IsSplit", QJSValue(edit->DiffText_IsSplit));
    o.setProperty("DiffText_New", QJSValue(edit->DiffText_New));
    o.setProperty("DiffText_Old", QJSValue(edit->DiffText_Old));
    o.setProperty("DiffTo", QJSValue(edit->DiffTo));
    o.setProperty("EditMadeByHuggle", QJSValue(edit->EditMadeByHuggle));
    o.setProperty("FullUrl", QJSValue(edit->GetFullUrl()));
    o.setProperty("SiteName", QJSValue(edit->GetSite()->Name));
    o.setProperty("Size", QJSValue(static_cast<int>(edit->GetSize())));
    o.setProperty("GoodfaithScore", QJSValue(static_cast<int>(edit->GoodfaithScore)));
    o.setProperty("IsRangeOfEdits", QJSValue(edit->IsRangeOfEdits()));
    o.setProperty("IsRevert", QJSValue(edit->IsRevert));
    o.setProperty("IsValid", QJSValue(edit->IsValid));
    o.setProperty("IsMinor", QJSValue(edit->IsMinor));
    o.setProperty("NewPage", QJSValue(edit->NewPage));
    o.setProperty("OwnEdit", QJSValue(edit->OwnEdit));
    o.setProperty("Page", FromPage(edit->Page, engine));
    o.setProperty("User", FromUser(edit->User, engine));
    return o;
}

QJSValue JSMarshallingHelper::FromVersion(Version *version, QJSEngine *engine)
{
    if (!version)
        return QJSValue(false);
    QJSValue o = engine->newObject();
    o.setProperty("Major", QJSValue(version->GetMajor()));
    o.setProperty("Minor", QJSValue(version->GetMinor()));
    o.setProperty("Revision", QJSValue(version->GetRevision()));
    o.setProperty("String", QJSValue(version->ToString()));
    return o;
}

QJSValue JSMarshallingHelper::FromPage(WikiPage *page, QJSEngine *engine)
{
    if (!page)
        return QJSValue(false);
    QJSValue o = engine->newObject();
    o.setProperty("ContentModel", QJSValue(page->ContentModel));
    o.setProperty("Contents", QJSValue(page->Contents));
    o.setProperty("EncodedName", QJSValue(page->EncodedName()));
    o.setProperty("FounderKnown", QJSValue(page->FounderKnown()));
    o.setProperty("Founder", QJSValue(page->GetFounder()));
    o.setProperty("NS", FromNS(page->GetNS(), engine));
    o.setProperty("Site", FromSite(page->GetSite(), engine));
    o.setProperty("IsTalk", QJSValue(page->IsTalk()));
    o.setProperty("IsUserpage", QJSValue(page->IsUserpage()));
    o.setProperty("IsWatched", QJSValue(page->IsWatched()));
    o.setProperty("PageName", QJSValue(page->PageName));
    o.setProperty("RootName", QJSValue(page->RootName()));
    o.setProperty("SanitizedName", QJSValue(page->SanitizedName()));
    return o;
}

QJSValue JSMarshallingHelper::FromNS(WikiPageNS *ns, QJSEngine *engine)
{
    if (!ns)
        return QJSValue(false);
    QJSValue o = engine->newObject();
    o.setProperty("CanonicalName", QJSValue(ns->GetCanonicalName()));
    o.setProperty("ID", QJSValue(ns->GetID()));
    o.setProperty("Name", QJSValue(ns->GetName()));
    o.setProperty("IsTalkPage", QJSValue(ns->IsTalkPage()));
    return o;
}

QJSValue JSMarshallingHelper::FromQuery(Query *query, QJSEngine *engine)
{
    QJSValue o = engine->newObject();
    o.setProperty("FailureReason", query->GetFailureReason());
    o.setProperty("ExecutionTime", static_cast<int>(query->ExecutionTime()));
    o.setProperty("CustomStatus", query->CustomStatus);
    o.setProperty("HiddenQuery", query->HiddenQuery);
    o.setProperty("IsFailed", query->IsFailed());
    return o;
}

QJSValue JSMarshallingHelper::FromApiQuery(ApiQuery *query, QJSEngine *engine)
{
    QJSValue o = FromQuery(query, engine);

    return o;
}

QJSValue JSMarshallingHelper::FromApiQueryResult(ApiQueryResult *res, QJSEngine *engine)
{
    QJSValue o = engine->newObject();
    o.setProperty("Data", res->Data);
    o.setProperty("ErrorCode", res->ErrorCode);
    o.setProperty("ErrorMessage", res->ErrorMessage);
    o.setProperty("HasErrors", res->HasErrors);
    o.setProperty("IsFailed", res->IsFailed());
    o.setProperty("Warning", res->Warning);
    return o;
}
