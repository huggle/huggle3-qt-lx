//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikiutil.hpp"
#include "configuration.hpp"
#include "exception.hpp"
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
        throw new Huggle::Exception("Month must be between 1 and 12");
    }
    n--;
    return Configuration::HuggleConfiguration->ProjectConfig->Months.at(n);
}

Collectable_SmartPtr<RevertQuery> WikiUtil::RevertEdit(WikiEdit *_e, QString summary, bool minor, bool rollback)
{
    if (_e == nullptr)
        throw new Huggle::Exception("NULL edit in RevertEdit(WikiEdit *_e, QString summary, bool minor, bool rollback, bool keep) is not a valid edit");
    if (_e->User == nullptr)
        throw new Huggle::Exception("Object user was NULL in Core::Revert");
    if (_e->Page == nullptr)
        throw new Huggle::Exception("Object page was NULL");

    Collectable_SmartPtr<RevertQuery> query = new RevertQuery(_e, _e->GetSite());
    if (summary.length())
        query->Summary = summary;
    query->MinorEdit = minor;
    QueryPool::HugglePool->AppendQuery(query);
    if (hcfg->UserConfig->EnforceManualSoftwareRollback)
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

Collectable_SmartPtr<EditQuery> WikiUtil::AppendTextToPage(QString page, QString text, QString summary, bool minor)
{
    ///! \todo assumes main project?
    Collectable_SmartPtr<EditQuery> eq = new EditQuery();
    eq->Page = new WikiPage(page);
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
        throw Huggle::Exception("Invalid page (NULL)", "EditQuery *WikiUtil::EditPage(WikiPage *page, QString text, QString"\
                                " summary, bool minor, QString BaseTimestamp)");
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

ApiQuery *WikiUtil::Unwatchlist(WikiPage *page)
{
    ApiQuery *wt = new ApiQuery(ActionUnwatch, page->GetSite());
    wt->RegisterConsumer(HUGGLECONSUMER_QP_WATCHLIST);
    wt->UsingPOST = true;
    wt->Target = page->PageName;
    // first of all we need to check if current watchlist token is valid or not
    if (page->GetSite()->GetProjectConfig()->WatchlistToken.isEmpty())
    {
        // we need to append this query to watchlist queries that just wait for token
        ApiQuery *token = new ApiQuery(ActionQuery, page->GetSite());
        token->Parameters = "prop=info&intoken=watch&titles=" + page->EncodedName();
        token->Target = "Watchlist token";
        token->IncRef();
        wt->Dependency = token;
        QueryPool::HugglePool->PendingWatches.append(wt);
        QueryPool::HugglePool->AppendQuery(token);
        token->Process();
        Syslog::HuggleLogs->Log("There is no watchlist token, retrieving some");
        return wt;
    }
    wt->Parameters = "titles=" + page->EncodedName() + "&unwatch=1&token=" + QUrl::toPercentEncoding(page->GetSite()->GetProjectConfig()->WatchlistToken);
    QueryPool::HugglePool->PendingWatches.append(wt);
    wt->Process();
    return wt;
}

ApiQuery *WikiUtil::Watchlist(WikiPage *page)
{
    ApiQuery *wt = new ApiQuery(ActionWatch, page->GetSite());
    wt->RegisterConsumer(HUGGLECONSUMER_QP_WATCHLIST);
    wt->UsingPOST = true;
    wt->Target = page->PageName;
    // first of all we need to check if current watchlist token is valid or not
    if (page->GetSite()->GetProjectConfig()->WatchlistToken.isEmpty())
    {
        // we need to append this query to watchlist queries that just wait for token
        ApiQuery *token = new ApiQuery(ActionQuery, page->GetSite());
        token->Parameters = "prop=info&intoken=watch&titles=" + page->EncodedName();
        token->Target = "Watchlist token";
        token->IncRef();
        wt->Dependency = token;
        QueryPool::HugglePool->PendingWatches.append(wt);
        QueryPool::HugglePool->AppendQuery(token);
        token->Process();
        Syslog::HuggleLogs->Log("There is no watchlist token, retrieving some");
        return wt;
    }
    wt->Parameters = "titles=" + page->EncodedName() + "&token=" + QUrl::toPercentEncoding(page->GetSite()->GetProjectConfig()->WatchlistToken);
    QueryPool::HugglePool->PendingWatches.append(wt);
    wt->Process();
    return wt;
}
