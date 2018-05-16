//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hooks.hpp"
#include "core.hpp"
#include "configuration.hpp"
#include "events.hpp"
#include "message.hpp"
#include "iextension.hpp"
#include "wikiuser.hpp"
#include "wikiedit.hpp"
#include "syslog.hpp"
#include "scripting/script.hpp"
#include "exception.hpp"
#include "wikipage.hpp"

bool Huggle::Hooks::EditBeforeScore(Huggle::WikiEdit *Edit)
{
    if (Edit == nullptr)
        throw new Huggle::NullPointerException("Huggle::WikiEdit *Edit", BOOST_CURRENT_FUNCTION);

    bool result = true;
    foreach (Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
        {
            if (!extension->Hook_EditBeforeScore((void*)Edit))
                result = false;
        }
    }
    return result;
}

void Huggle::Hooks::EditPreProcess(Huggle::WikiEdit *Edit)
{
    if (Edit == nullptr)
        throw new Huggle::NullPointerException("Huggle::WikiEdit *Edit", BOOST_CURRENT_FUNCTION);

    foreach(Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
            extension->Hook_EditPreProcess((void*)Edit);
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            s->Hook_EditPreProcess(Edit);
    }
}

void Huggle::Hooks::EditBeforePostProcess(Huggle::WikiEdit *Edit)
{
    if (Edit == nullptr)
        throw new Huggle::NullPointerException("Huggle::WikiEdit *Edit", BOOST_CURRENT_FUNCTION);

    foreach(Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
            extension->Hook_EditBeforePostProcessing((void*)Edit);
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            s->Hook_EditBeforePostProcess(Edit);
    }
}

bool Huggle::Hooks::RevertPreflight(Huggle::WikiEdit *Edit)
{
    bool result = true;
    foreach(Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
        {
            if (!extension->Hook_RevertPreflight((void*)Edit))
                result = false;
        }
    }
    return result;
}

void Huggle::Hooks::EditPostProcess(Huggle::WikiEdit *Edit)
{
    if (Edit == nullptr)
        throw new NullPointerException("Huggle::WikiEdit *Edit", BOOST_CURRENT_FUNCTION);

    foreach(Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
            extension->Hook_EditPostProcess((void*)Edit);
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            s->Hook_EditPostProcess(Edit);
    }
}

bool Huggle::Hooks::OnEditLoadToQueue(Huggle::WikiEdit *Edit)
{
    foreach(Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
            if (!extension->Hook_OnEditLoadToQueue((void*)Edit))
                return false;
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            if (!s->Hook_EditLoadToQueue(Edit))
                return false;
    }
    return true;
}

void Huggle::Hooks::OnGood(Huggle::WikiEdit *Edit)
{
    Events::Global->on_WEGood(Edit);
    foreach(Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
            e->Hook_GoodEdit((void*)Edit);
    }
}

void Huggle::Hooks::OnRevert(Huggle::WikiEdit *Edit)
{
    Events::Global->on_WERevert(Edit);
}

bool Huggle::Hooks::EditCheckIfReady(Huggle::WikiEdit *Edit)
{
    bool result = true;
    foreach(Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking() && !e->Hook_EditIsReady((void*)Edit))
            result = false;
    }
    return result;
}

void Huggle::Hooks::OnWarning(Huggle::WikiUser *User)
{
    Events::Global->on_WEWarningSent(User, User->GetWarningLevel());
}

void Huggle::Hooks::Suspicious(Huggle::WikiEdit *Edit)
{
    Events::Global->on_WESuspicious(Edit);
}

void Huggle::Hooks::BadnessScore(Huggle::WikiUser *User, int Score)
{
    if (User == nullptr)
        throw new NullPointerException("Huggle::WikiUser *User", BOOST_CURRENT_FUNCTION);

    foreach(Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
            extension->Hook_BadnessScore((void*)User, Score);
    }
}

void Huggle::Hooks::Shutdown()
{
    foreach (Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if ( e->IsWorking() )
           e->Hook_Shutdown();
    }
}

Huggle::Message *Huggle::Hooks::MessageUser(Huggle::WikiUser *User, QString Text, QString Title, QString Summary, bool InsertSection, Query *Dependency, bool NoSuffix,
                                bool SectionKeep, bool Autoremove, QString BaseTimestamp, bool CreateOnly, bool FreshOnly)
{
    foreach(Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
        {
            void *result = e->Hook_MessageUser((void*)User, Text, Title, Summary, InsertSection, (void*)Dependency, NoSuffix, SectionKeep, Autoremove, BaseTimestamp, CreateOnly, FreshOnly);
            if (result != nullptr)
                return (Huggle::Message*)result;
        }
    }
    return nullptr;
}

void Huggle::Hooks::WikiUser_Updated(Huggle::WikiUser *user)
{
    if (!Events::Global)
        return;

    Events::Global->on_UpdateUser(user);
}

void Huggle::Hooks::WikiEdit_OnNewHistoryItem(Huggle::HistoryItem *history_item)
{
    if (!Events::Global)
        return;

    Events::Global->on_WENewHistoryItem(history_item);
}

void Huggle::Hooks::QueryPool_Remove(Huggle::Query *q)
{
    Events::Global->on_QueryPoolRemove(q);
}

void Huggle::Hooks::QueryPool_Update(Huggle::Query *q)
{
    Events::Global->on_QueryPoolUpdate(q);
}

void Huggle::Hooks::ReportUser(Huggle::WikiUser *u)
{
    Events::Global->on_Report(u);
}

void Huggle::Hooks::SilentReport(Huggle::WikiUser *u)
{
    Events::Global->on_SReport(u);
}

void Huggle::Hooks::ShowMessage(QString title, QString message)
{
    Events::Global->on_SMessage(title, message);
}

void Huggle::Hooks::ShowError(QString title, QString message)
{
    Events::Global->on_SError(title, message);
}

void Huggle::Hooks::ShowWarning(QString title, QString message)
{
    Events::Global->on_SWarning(title, message);
}

bool Huggle::Hooks::ShowYesNoQuestion(QString title, QString message, bool default_answer)
{
    return Events::Global->on_SYesNoQs(title, message, default_answer);
}

