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
#include "mediawiki.hpp"
#include "syslog.hpp"
#include "querypool.hpp"
#include "wikisite.hpp"
#include "wikiedit.hpp"
#include "wikiuser.hpp"
#include <QUrl>

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

QString WikiUtil::MonthText(int n, WikiSite *site)
{
    if (n < 1 || n > 12)
    {
        throw new Huggle::Exception("Month must be between 1 and 12", BOOST_CURRENT_FUNCTION);
    }
    if (!site)
        site = hcfg->Project;
    n--;
    return site->GetProjectConfig()->Months.at(n);
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

Collectable_SmartPtr<EditQuery> WikiUtil::EditPage(QString page, QString text, QString summary, bool minor, QString BaseTimestamp, unsigned int section, WikiSite *site)
{
    WikiPage tp(page);
    if (site != nullptr)
        tp.Site = site;
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
    foreach (Collectable_SmartPtr<Query> qx, Query::PendingRestart)
    {
        qx->Restart();
    }
    Query::PendingRestart.clear();
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

/////////////////////////////////////////////////////////////////
///                     RetrieveEditByRevid                   ///
/////////////////////////////////////////////////////////////////

class RetrieveEditByRevid_SourceInfo
{
    public:
        void *source;
        WikiEdit *edit;
        WikiUtil::RetrieveEditByRevid_Callback success;
        WikiUtil::RetrieveEditByRevid_Callback error;
};

static void RetrieveEditByRevid_Page_OK(Query *query)
{
    ApiQuery *result = (ApiQuery*) query;
    RetrieveEditByRevid_SourceInfo *x = (RetrieveEditByRevid_SourceInfo*)query->CallbackOwner;
    ApiQueryResultNode *page = result->GetApiQueryResult()->GetNode("page");
    ApiQueryResultNode *revision_data = result->GetApiQueryResult()->GetNode("rev");
    ApiQueryResultNode *diff_text = result->GetApiQueryResult()->GetNode("diff");
    if (diff_text == nullptr)
    {
        x->error(x->edit, x->source, "No diff was returned for query");
    }
    if (revision_data == nullptr)
    {
        x->error(x->edit, x->source, "No revision was returned by query");
        goto exit;
    }
    if (page == nullptr)
    {
        // whoa, this is an error!
        x->error(x->edit, x->source, "No page info was returned by query");
        goto exit;
    }
    x->edit->Page = new WikiPage(page->GetAttribute("title"));
    x->edit->Page->Site = result->GetSite();
    x->edit->SetSize(revision_data->GetAttribute("size", "0").toLong());
    x->edit->Summary = revision_data->GetAttribute("comment");
    x->edit->Time = MediaWiki::FromMWTimestamp(revision_data->GetAttribute("timestamp"));
    x->edit->User = new WikiUser(revision_data->GetAttribute("user"));
    x->edit->User->Site = result->GetSite();
    // pre process the edit
    QueryPool::HugglePool->PreProcessEdit(x->edit);
    // \bug now put the diff into the diff store, keep in mind that edit is still not postprocessed so many things are probably not going to be evaluated
    // we need to wait for post processing to finish here, it's just that there isn't really any simple way to accomplish that
    x->edit->DiffText = diff_text->Value;
    // this is true hack as it's async call, but we don't really need to have the edit post processed for it
    // to be rendered, let's just call it to be safe, as having unprocessed edits in buffer is a bad thing
    QueryPool::HugglePool->PostProcessEdit(x->edit);
    x->success(x->edit, x->source, "");
exit:
    delete x;
    query->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
}

static void RetrieveEditByRevid_Page_ER(Query *query)
{
    // The query has failed, let's call the failure callback :(
    RetrieveEditByRevid_SourceInfo *x = (RetrieveEditByRevid_SourceInfo*)query->CallbackOwner;
    x->error(x->edit, x->source, query->GetFailureReason());
    delete x;
    query->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
}

void WikiUtil::RetrieveEditByRevid(revid_ht revid, WikiSite *site, void *source,
                                   WikiUtil::RetrieveEditByRevid_Callback callback_success,
                                   WikiUtil::RetrieveEditByRevid_Callback callback_er)
{
    if (callback_er == nullptr)
        throw new Huggle::NullPointerException("callback_er", BOOST_CURRENT_FUNCTION);
    if (callback_success == nullptr)
        throw new Huggle::NullPointerException("callback_success", BOOST_CURRENT_FUNCTION);

    // let's create a new edit
    WikiEdit *edit = new WikiEdit();
    edit->Diff = revid;
    edit->RevID = revid;
    // let's get the information about the page now, once that is finished, we get information about the user
    ApiQuery *qPage = new ApiQuery(ActionQuery, site);
    RetrieveEditByRevid_SourceInfo *i = new RetrieveEditByRevid_SourceInfo();
    i->edit = edit;
    i->source = source;
    i->error = callback_er;
    i->success = callback_success;
    qPage->CallbackOwner = i;
    qPage->callback = (Callback) RetrieveEditByRevid_Page_OK;
    qPage->FailureCallback = (Callback) RetrieveEditByRevid_Page_ER;
    qPage->Parameters = "prop=revisions&revids=" + QString::number(revid) + "&rvprop=" +
                          QUrl::toPercentEncoding("ids|flags|timestamp|user|contentmodel|comment|size") + "&rvdiffto=prev";
    qPage->Process();
}

/////////////////////////////////////////////////////////////////
