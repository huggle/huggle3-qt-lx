//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikiutil.hpp"

using namespace Huggle;

bool WikiUtil::IsRevert(QString Summary)
{
    if (Summary != "")
    {
        int xx = 0;
        while (xx < Configuration::HuggleConfiguration->RevertPatterns.count())
        {
            if (Summary.contains(Configuration::HuggleConfiguration->RevertPatterns.at(xx)))
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
    return Configuration::HuggleConfiguration->Months.at(n);
}

RevertQuery *WikiUtil::RevertEdit(WikiEdit *_e, QString summary, bool minor, bool rollback, bool keep)
{
    if (_e == NULL)
    {
        throw new Exception("NULL edit in RevertEdit(WikiEdit *_e, QString summary, bool minor, bool rollback, bool keep) is not a valid edit");
    }
    if (_e->User == NULL)
    {
        throw new Exception("Object user was NULL in Core::Revert");
    }
    _e->RegisterConsumer("Core::RevertEdit");
    if (_e->Page == NULL)
    {
        throw new Exception("Object page was NULL");
    }

    RevertQuery *query = new RevertQuery(_e);
    if (summary != "")
    {
        query->Summary = summary;
    }
    query->MinorEdit = minor;
    QueryPool::HugglePool->AppendQuery(query);
    if (Configuration::HuggleConfiguration->EnforceManualSoftwareRollback)
    {
        query->UsingSR = true;
    } else
    {
        query->UsingSR = !rollback;
    }
    query->Process();

    if (keep)
    {
        query->RegisterConsumer("keep");
    }

    return query;
}

Message *WikiUtil::MessageUser(WikiUser *User, QString Text, QString Title, QString Summary, bool InsertSection,
                              Query *Dependency, bool NoSuffix, bool SectionKeep, bool autoremove,
                              QString BaseTimestamp, bool CreateOnly_, bool FreshOnly_)
{
    if (User == NULL)
    {
        Huggle::Syslog::HuggleLogs->Log("Cowardly refusing to message NULL user");
        return NULL;
    }

    if (Title == "")
    {
        InsertSection = false;
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
    Huggle::Syslog::HuggleLogs->DebugLog("Sending message to user " + User->Username);
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

EditQuery *WikiUtil::EditPage(QString page, QString text, QString summary, bool minor, QString BaseTimestamp, unsigned int section)
{
    // retrieve a token
    EditQuery *eq = new EditQuery();
    eq->IncRef();
    if (!summary.endsWith(Configuration::HuggleConfiguration->ProjectConfig_EditSuffixOfHuggle))
    {
        summary = summary + " " + Configuration::HuggleConfiguration->ProjectConfig_EditSuffixOfHuggle;
    }
    eq->RegisterConsumer(HUGGLECONSUMER_QP_MODS);
    eq->Page = page;
    eq->BaseTimestamp = BaseTimestamp;
    QueryPool::HugglePool->PendingMods.append(eq);
    eq->text = text;
    eq->Section = section;
    eq->Summary = summary;
    eq->Minor = minor;
    eq->Process();
    return eq;
}

EditQuery *WikiUtil::EditPage(WikiPage *page, QString text, QString summary, bool minor, QString BaseTimestamp)
{
    if (page == NULL)
    {
        throw Huggle::Exception("Invalid page (NULL)", "EditQuery *WikiUtil::EditPage(WikiPage *page, QString text, QString"\
                                " summary, bool minor, QString BaseTimestamp)");
    }
    return EditPage(page->PageName, text, summary, minor, BaseTimestamp);
}
