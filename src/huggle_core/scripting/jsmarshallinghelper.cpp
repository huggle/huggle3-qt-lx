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
#include "../apiquery.hpp"
#include "../apiqueryresult.hpp"
#include "../editquery.hpp"
#include "../generic.hpp"
#include "../userconfiguration.hpp"
#include "../projectconfiguration.hpp"
#include "../version.hpp"
#include "../wlquery.hpp"
#include "../wikiedit.hpp"
#include "../wikipage.hpp"
#include "../wikiuser.hpp"
#include "../wikisite.hpp"

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
    o.setProperty("UserConfig", FromSiteUserConfig(site->GetUserConfig(), engine));
    o.setProperty("ProjectConfig", FromSiteProjectConfig(site->GetProjectConfig(), engine));
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
    o.setProperty("SiteName", user->GetSite()->Name);
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
    o.setProperty("SiteName", page->GetSite()->Name);
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
    o.setProperty("CanonicalName", ns->GetCanonicalName());
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
    o.setProperty("CustomStatus", QJSValue(query->CustomStatus));
    o.setProperty("EditingQuery", QJSValue(query->EditingQuery));
    o.setProperty("HiddenQuery", QJSValue(query->HiddenQuery));
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

QJSValue JSMarshallingHelper::FromEditQuery(EditQuery *eq, QJSEngine *engine)
{
    QJSValue o = FromQuery(eq, engine);
    o.setProperty("Append", QJSValue(eq->Append));
    o.setProperty("BaseTimestamp", QJSValue(eq->BaseTimestamp));
    o.setProperty("Page", JSMarshallingHelper::FromPage(eq->Page, engine));
    return o;
}

QJSValue JSMarshallingHelper::FromSiteProjectConfig(ProjectConfiguration *config, QJSEngine *engine)
{
    QJSValue o = engine->newObject();
    o.setProperty("AgfRevert", config->AgfRevert);
    o.setProperty("AIV", config->AIV);
    o.setProperty("AIVP", FromPage(config->AIVP, engine));
    o.setProperty("Approval", config->Approval);
    o.setProperty("ApprovalPage", config->ApprovalPage);
    o.setProperty("AutomaticallyResolveConflicts", config->AutomaticallyResolveConflicts);
    o.setProperty("ConfirmMultipleEdits", config->ConfirmMultipleEdits);
    o.setProperty("ConfirmOnSelfRevs", config->ConfirmOnSelfRevs);
    o.setProperty("ConfirmTalk", config->ConfirmTalk);
    o.setProperty("ConfirmWL", config->ConfirmWL);
    o.setProperty("DefaultSummary", config->DefaultSummary);
    o.setProperty("DefaultTemplate", config->DefaultTemplate);
    o.setProperty("EditSuffixOfHuggle", config->EditSuffixOfHuggle);
    o.setProperty("EnableAll", config->EnableAll);
    o.setProperty("IsLoggedIn", config->IsLoggedIn);
    o.setProperty("MinimalVersion", config->MinimalVersion);
    o.setProperty("Patrolling", config->Patrolling);
    o.setProperty("Token_Csrf", config->Token_Csrf);
    o.setProperty("Token_Patrol", config->Token_Patrol);
    o.setProperty("Token_Rollback", config->Token_Rollback);
    o.setProperty("Token_Watch", config->Token_Watch);
    return o;
}

QJSValue JSMarshallingHelper::FromSiteUserConfig(UserConfiguration *config, QJSEngine *engine)
{
    QJSValue o = engine->newObject();
    o.setProperty("AutomaticallyGroup", config->AutomaticallyGroup);
    o.setProperty("AutomaticallyResolveConflicts", config->AutomaticallyResolveConflicts);
    o.setProperty("AutomaticallyWatchlistWarnedUsers", config->AutomaticallyWatchlistWarnedUsers);
    o.setProperty("AutomaticRefresh", config->AutomaticRefresh);
    o.setProperty("AutomaticReports", config->AutomaticReports);
    o.setProperty("CheckTP", config->CheckTP);
    o.setProperty("DefaultSummary", config->DefaultSummary);
    o.setProperty("DeleteEditsAfterRevert", config->DeleteEditsAfterRevert);
    o.setProperty("DisplayTitle", config->DisplayTitle);
    o.setProperty("EnableMaxScore", config->EnableMaxScore);
    o.setProperty("EnableMinScore", config->EnableMinScore);
    return o;
}
