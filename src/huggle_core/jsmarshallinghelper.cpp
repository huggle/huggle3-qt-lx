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
#include "generic.hpp"
#include "version.hpp"
#include "wikiedit.hpp"
#include "wikipage.hpp"
#include "wikiuser.hpp"
#include "wikisite.hpp"
#include <QtScript>

using namespace Huggle;

QScriptValue JSMarshallingHelper::FromSite(WikiSite *site, QScriptEngine *engine)
{
    if (!site)
        return QScriptValue(engine, false);
    QScriptValue o = engine->newObject();
    o.setProperty("Name", QScriptValue(engine, site->Name));
    o.setProperty("ForceSSL", QScriptValue(engine, site->ForceSSL));
    o.setProperty("HANChannel", QScriptValue(engine, site->HANChannel));
    o.setProperty("IRCChannel", QScriptValue(engine, site->IRCChannel));
    o.setProperty("IsRightToLeft", QScriptValue(engine, site->IsRightToLeft));
    o.setProperty("LongPath", QScriptValue(engine, site->LongPath));
    o.setProperty("MediawikiVersion", FromVersion(&site->MediawikiVersion, engine));
    o.setProperty("ScriptPath", QScriptValue(engine, site->ScriptPath));
    o.setProperty("SupportHttps", QScriptValue(engine, site->SupportHttps));
    o.setProperty("URL", QScriptValue(engine, site->URL));
    return o;
}

QScriptValue JSMarshallingHelper::FromUser(WikiUser *user, QScriptEngine *engine)
{
    if (!user)
        return QScriptValue(engine, false);
    QScriptValue o = engine->newObject();
    o.setProperty("EditCount", QScriptValue(engine, static_cast<int>(user->EditCount)));
    o.setProperty("Username", QScriptValue(engine, user->Username));
    // CPU expensive
    // o.setProperty("Flags", QScriptValue(engine, user->Flags()));
    o.setProperty("BadnessScore", QScriptValue(engine, static_cast<int>(user->GetBadnessScore())));
    o.setProperty("Site", FromSite(user->GetSite(), engine));
    o.setProperty("TalkPageName", QScriptValue(engine, user->GetTalk()));
    o.setProperty("UserPageName", QScriptValue(engine, user->GetUserPage()));
    o.setProperty("WarningLevel", QScriptValue(engine, static_cast<int>(user->GetWarningLevel())));
    o.setProperty("IsBlocked", QScriptValue(engine, user->IsBlocked));
    o.setProperty("IsBot", QScriptValue(engine, user->IsBot()));
    o.setProperty("IsIP", QScriptValue(engine, user->IsIP()));
    o.setProperty("IsReported", QScriptValue(engine, user->IsReported));
    o.setProperty("IsWhitelisted", QScriptValue(engine, user->IsWhitelisted()));
    o.setProperty("RegistrationDate", QScriptValue(engine, user->RegistrationDate));
    o.setProperty("Site", FromSite(user->Site, engine));
    o.setProperty("TalkPageContents", QScriptValue(engine, user->TalkPage_GetContents()));
    o.setProperty("TalkPage_RetrievalTime", QScriptValue(engine, user->TalkPage_RetrievalTime().toString()));
    return o;
}

QScriptValue JSMarshallingHelper::FromEdit(WikiEdit *edit, QScriptEngine *engine)
{
    if (!edit)
        return QScriptValue(engine, false);
    QScriptValue o = engine->newObject();
    o.setProperty("Bot", QScriptValue(engine, edit->Bot));
    o.setProperty("CurrentUserWarningLevel", QScriptValue(engine, static_cast<int>(edit->CurrentUserWarningLevel)));
    o.setProperty("Diff", QScriptValue(engine, static_cast<int>(edit->Diff)));
    o.setProperty("DiffText", QScriptValue(engine, edit->DiffText));
    o.setProperty("DiffText_IsSplit", QScriptValue(engine, edit->DiffText_IsSplit));
    o.setProperty("DiffText_New", QScriptValue(engine, edit->DiffText_New));
    o.setProperty("DiffText_Old", QScriptValue(engine, edit->DiffText_Old));
    o.setProperty("DiffTo", QScriptValue(engine, edit->DiffTo));
    return o;
}

QScriptValue JSMarshallingHelper::FromVersion(Version *version, QScriptEngine *engine)
{
    if (!version)
        return QScriptValue(engine, false);
    QScriptValue o = engine->newObject();
    o.setProperty("Major", QScriptValue(engine, version->GetMajor()));
    o.setProperty("Minor", QScriptValue(engine, version->GetMinor()));
    o.setProperty("Revision", QScriptValue(engine, version->GetRevision()));
    o.setProperty("String", QScriptValue(engine, version->ToString()));
    return o;
}

QScriptValue JSMarshallingHelper::FromPage(WikiPage *page, QScriptEngine *engine)
{
    if (!page)
        return QScriptValue(engine, false);
    QScriptValue o = engine->newObject();
    o.setProperty("ContentModel", QScriptValue(engine, page->ContentModel));
    o.setProperty("Contents", QScriptValue(engine, page->Contents));
    o.setProperty("EncodedName", QScriptValue(engine, page->EncodedName()));
    o.setProperty("FounderKnown", QScriptValue(engine, page->FounderKnown()));
    o.setProperty("Founder", QScriptValue(engine, page->GetFounder()));
    o.setProperty("NS", FromNS(page->GetNS(), engine));
    o.setProperty("Site", FromSite(page->GetSite(), engine));
    o.setProperty("IsTalk", QScriptValue(engine, page->IsTalk()));
    o.setProperty("IsUserpage", QScriptValue(engine, page->IsUserpage()));
    o.setProperty("IsWatched", QScriptValue(engine, page->IsWatched()));
    o.setProperty("PageName", QScriptValue(engine, page->PageName));
    o.setProperty("RootName", QScriptValue(engine, page->RootName()));
    o.setProperty("SanitizedName", QScriptValue(engine, page->SanitizedName()));
    return o;
}

QScriptValue JSMarshallingHelper::FromNS(WikiPageNS *ns, QScriptEngine *engine)
{
    if (!ns)
        return QScriptValue(engine, false);
    QScriptValue o = engine->newObject();
    o.setProperty("CanonicalName", QScriptValue(engine, ns->GetCanonicalName()));
    o.setProperty("ID", QScriptValue(engine, ns->GetID()));
    o.setProperty("Name", QScriptValue(engine, ns->GetName()));
    o.setProperty("IsTalkPage", QScriptValue(engine, ns->IsTalkPage()));
    return o;
}
