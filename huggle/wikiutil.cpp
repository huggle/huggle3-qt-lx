//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikiutil.hpp"
#include "apiquery.hpp"
#include "apiqueryresult.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "editquery.hpp"
#include "syslog.hpp"
#include "querypool.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"

using namespace Huggle;

bool WikiUtil::IsRevert(QString Summary)
{
    if (Summary.size() > 0)
    {
        int xx = 0;
        while (xx < Configuration::HuggleConfiguration->ProjectConfig->_revertPatterns.count())
        {
            if (Summary.contains(Configuration::HuggleConfiguration->ProjectConfig->_revertPatterns.at(xx)))
            {
                return true;
            }
            xx++;
        }
    }
    return false;
}

QString WikiUtil::MonthText(int n)
{
    if (n < 1 || n > 12)
    {
        throw new Huggle::Exception("Month must be between 1 and 12", BOOST_CURRENT_FUNCTION);
    }
    n--;
    return Configuration::HuggleConfiguration->ProjectConfig->Months.at(n);
}

Collectable_SmartPtr<RevertQuery> WikiUtil::RevertEdit(WikiEdit *_e, QString summary, bool minor, bool rollback)
{
    if (_e == nullptr)
        throw new Huggle::NullPointerException("NULL edit in RevertEdit(WikiEdit *_e, QString summary, bool minor, bool rollback, bool keep) is not a valid edit", BOOST_CURRENT_FUNCTION);
    if (_e->User == nullptr)
        throw new Huggle::NullPointerException("Object user was NULL in Core::Revert", BOOST_CURRENT_FUNCTION);
    if (_e->Page == nullptr)
        throw new Huggle::NullPointerException("Object page was NULL", BOOST_CURRENT_FUNCTION);

    Collectable_SmartPtr<RevertQuery> query = new RevertQuery(_e, _e->GetSite());
    if (summary.length())
        query->Summary = summary;
    query->MinorEdit = minor;
    QueryPool::HugglePool->AppendQuery(query);
    if (hcfg->UserConfig->EnforceSoftwareRollback())
        query->SetUsingSR(true);
    else
        query->SetUsingSR(!rollback);
    return query;
}

Message *WikiUtil::MessageUser(WikiUser *User, QString Text, QString Title, QString Summary, bool InsertSection,
                              Query *Dependency, bool NoSuffix, bool SectionKeep, bool autoremove,
                              QString BaseTimestamp, bool CreateOnly_, bool FreshOnly_)
{
    if (User == nullptr)
    {
        Huggle::Syslog::HuggleLogs->Log("Cowardly refusing to message NULL user");
        return nullptr;
    }

    if (Title.isEmpty())
    {
        InsertSection = false;
        SectionKeep = false;
    }

    Message *m = new Message(User, Text, Summary);
    m->Title = Title;
    m->Dependency = Dependency;
    m->CreateInNewSection = InsertSection;
    m->BaseTimestamp = BaseTimestamp;
    m->SectionKeep = SectionKeep;
    m->RequireFresh = FreshOnly_;
    m->CreateOnly = CreateOnly_;
    m->Suffix = !NoSuffix;
    QueryPool::HugglePool->Messages.append(m);
    m->RegisterConsumer(HUGGLECONSUMER_CORE);
    if (!autoremove)
    {
        m->RegisterConsumer(HUGGLECONSUMER_CORE_MESSAGE);
    }
    m->Send();
    HUGGLE_DEBUG("Sending message to user " + User->Username, 1);
    return m;
}

void WikiUtil::FinalizeMessages()
{
    if (QueryPool::HugglePool->Messages.count() < 1)
    {
        return;
    }
    int x=0;
    QList<Message*> list;
    while (x<QueryPool::HugglePool->Messages.count())
    {
        if (QueryPool::HugglePool->Messages.at(x)->IsFinished())
        {
            list.append(QueryPool::HugglePool->Messages.at(x));
        }
        x++;
    }
    x=0;
    while (x<list.count())
    {
        Message *message = list.at(x);
        message->UnregisterConsumer(HUGGLECONSUMER_CORE);
        QueryPool::HugglePool->Messages.removeOne(message);
        x++;
    }
}

Collectable_SmartPtr<EditQuery> WikiUtil::AppendTextToPage(QString page, QString text, QString summary, bool minor, WikiSite *site)
{
    if (!site)
        site = hcfg->Project;
    Collectable_SmartPtr<EditQuery> eq = new EditQuery();
    eq->Page = new WikiPage(page);
    eq->Page->Site = site;
    eq->text = text;
    summary = Configuration::GenerateSuffix(summary, eq->Page->GetSite()->GetProjectConfig());
    eq->Summary = summary;
    eq->Minor = minor;
    eq->Append = true;
    eq->RegisterConsumer(HUGGLECONSUMER_QP_MODS);
    QueryPool::HugglePool->PendingMods.append(eq);
    eq->Process();
    return eq;
}

Collectable_SmartPtr<EditQuery> WikiUtil::EditPage(QString page, QString text, QString summary, bool minor, QString BaseTimestamp, unsigned int section)
{
    WikiPage tp(page);
    return EditPage(&tp, text, summary, minor, BaseTimestamp, section);
}

Collectable_SmartPtr<EditQuery> WikiUtil::EditPage(WikiPage *page, QString text, QString summary, bool minor, QString BaseTimestamp, unsigned int section)
{
    if (page == nullptr)
    {
        throw Huggle::NullPointerException("WikiPage *page", BOOST_CURRENT_FUNCTION);
    }
    Collectable_SmartPtr<EditQuery> eq = new EditQuery();
    summary = Configuration::GenerateSuffix(summary, page->GetSite()->GetProjectConfig());
    eq->RegisterConsumer(HUGGLECONSUMER_QP_MODS);
    eq->Page = new WikiPage(page);
    eq->BaseTimestamp = BaseTimestamp;
    QueryPool::HugglePool->PendingMods.append(eq);
    eq->text = text;
    eq->Section = section;
    eq->Summary = summary;
    eq->Minor = minor;
    eq->Process();
    return eq;
}

QString WikiUtil::SanitizeUser(QString username)
{
    // ensure we don't modify the original string
    if (username.contains(" "))
    {
        return QString(username).replace(" ", "_");
    }
    return username;
}

Collectable_SmartPtr<ApiQuery> WikiUtil::Unwatchlist(WikiPage *page)
{
    ApiQuery *wt = new ApiQuery(ActionUnwatch, page->GetSite());
    wt->RegisterConsumer(HUGGLECONSUMER_QP_WATCHLIST);
    wt->UsingPOST = true;
    wt->Target = page->PageName;
    // first of all we need to check if current watchlist token is valid or not
    if (page->GetSite()->GetProjectConfig()->Token_Watch.isEmpty())
    {
        wt->Result = new QueryResult(true);
        wt->Result->SetError("No watchlist token");
        wt->Status = StatusInError;
        return wt;
    }
    wt->Parameters = "titles=" + page->EncodedName() + "&unwatch=1&token=" + QUrl::toPercentEncoding(page->GetSite()->GetProjectConfig()->Token_Watch);
    QueryPool::HugglePool->PendingWatches.append(wt);
    wt->Process();
    return wt;
}

Collectable_SmartPtr<ApiQuery> WikiUtil::Watchlist(WikiPage *page)
{
    ApiQuery *wt = new ApiQuery(ActionWatch, page->GetSite());
    wt->RegisterConsumer(HUGGLECONSUMER_QP_WATCHLIST);
    wt->UsingPOST = true;
    wt->Target = page->PageName;
    // first of all we need to check if current watchlist token is valid or not
    if (page->GetSite()->GetProjectConfig()->Token_Watch.isEmpty())
    {
        wt->Result = new QueryResult(true);
        wt->Result->SetError("No watchlist token");
        wt->Status = StatusInError;
        return wt;
    }
    wt->Parameters = "titles=" + page->EncodedName() + "&token=" + QUrl::toPercentEncoding(page->GetSite()->GetProjectConfig()->Token_Watch);
    QueryPool::HugglePool->PendingWatches.append(wt);
    wt->Process();
    return wt;
}


Collectable_SmartPtr<EditQuery> WikiUtil::PrependTextToPage(QString page, QString text, QString summary, bool minor, WikiSite *site)
{
    if (!site)
        site = hcfg->Project;
    Collectable_SmartPtr<EditQuery> eq = new EditQuery();
    eq->Page = new WikiPage(page);
    eq->Page->Site = site;
    eq->text = text;
    summary = Configuration::GenerateSuffix(summary, eq->Page->GetSite()->GetProjectConfig());
    eq->Summary = summary;
    eq->Minor = minor;
    eq->Prepend = true;
    eq->RegisterConsumer(HUGGLECONSUMER_QP_MODS);
    QueryPool::HugglePool->PendingMods.append(eq);
    eq->Process();
    return eq;
}

Collectable_SmartPtr<EditQuery> WikiUtil::PrependTextToPage(WikiPage *page, QString text, QString summary, bool minor)
{
    return WikiUtil::PrependTextToPage(page->PageName, text, summary, minor, page->GetSite());
}

Collectable_SmartPtr<EditQuery> WikiUtil::AppendTextToPage(WikiPage *page, QString text, QString summary, bool minor)
{
    return WikiUtil::AppendTextToPage(page->PageName, text, summary, minor, page->GetSite());
}

static void FinishTokens(Query *token)
{
    ApiQuery *q = (ApiQuery*) token;
    WikiSite *site = (WikiSite*) token->CallbackOwner;
    ApiQueryResultNode *tokens = q->GetApiQueryResult()->GetNode("tokens");
    if (tokens != nullptr)
    {
        if (tokens->Attributes.contains("rollbacktoken"))
        {
            site->GetProjectConfig()->Token_Rollback = tokens->GetAttribute("rollbacktoken");
            HUGGLE_DEBUG("Token for " + site->Name + " rollback " + site->GetProjectConfig()->Token_Rollback, 2);
        } else
        {
            HUGGLE_DEBUG1("No rollback for " + site->Name + " result: " + q->Result->Data);
        }
        if (tokens->Attributes.contains("csrftoken"))
        {
            site->GetProjectConfig()->Token_Csrf = tokens->GetAttribute("csrftoken");
            HUGGLE_DEBUG("Token for " + site->Name + " csrf " + site->GetProjectConfig()->Token_Csrf, 2);
        } else
        {
            HUGGLE_DEBUG1("No csrf for " + site->Name + " result: " + q->Result->Data);
        }
        if (tokens->Attributes.contains("watchtoken"))
        {
            site->GetProjectConfig()->Token_Watch = tokens->GetAttribute("watchtoken");
            HUGGLE_DEBUG("Token for " + site->Name + " watch " + site->GetProjectConfig()->Token_Watch, 2);
        } else
        {
            HUGGLE_DEBUG1("No watch for " + site->Name + " result: " + q->Result->Data);
        }
    }
    token->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
    token->DecRef();
}

static void FailureTokens(Query *token)
{
    Syslog::HuggleLogs->ErrorLog("Failed to process: " + token->GetFailureReason());
    token->DecRef();
}

void WikiUtil::RetrieveTokens(WikiSite *wiki_site)
{
    ApiQuery *qr = new ApiQuery(ActionQuery, wiki_site);
    qr->IncRef();
    qr->Parameters = "meta=tokens&type=" + QUrl::toPercentEncoding("csrf|patrol|rollback|watch");
    qr->Target = "Tokens";
    qr->CallbackOwner = wiki_site;
    qr->FailureCallback = (Callback)FailureTokens;
    qr->callback = (Callback)FinishTokens;
    qr->Process();
}
