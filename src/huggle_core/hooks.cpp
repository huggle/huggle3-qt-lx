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

bool Huggle::Hooks::EditBeforeScore(Huggle::WikiEdit *edit)
{
    if (edit == nullptr)
        throw new Huggle::NullPointerException("Huggle::WikiEdit *Edit", BOOST_CURRENT_FUNCTION);

    bool result = true;
    foreach (Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
        {
            if (!extension->Hook_EditBeforeScore(reinterpret_cast<void*>(edit)))
                result = false;
        }
    }
    return result;
}

void Huggle::Hooks::EditAfterPreProcess(Huggle::WikiEdit *edit)
{
    Events::Global->on_QueryPoolFinishWEPreprocess(edit);
    if (edit == nullptr)
        throw new Huggle::NullPointerException("Huggle::WikiEdit *Edit", BOOST_CURRENT_FUNCTION);

    foreach(Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
            extension->Hook_EditPreProcess(reinterpret_cast<void*>(edit));
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            s->Hook_EditPreProcess(edit);
    }
}

void Huggle::Hooks::EditBeforePostProcess(Huggle::WikiEdit *edit)
{
    if (edit == nullptr)
        throw new Huggle::NullPointerException("Huggle::WikiEdit *Edit", BOOST_CURRENT_FUNCTION);

    foreach(Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
            extension->Hook_EditBeforePostProcessing(reinterpret_cast<void*>(edit));
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            s->Hook_EditBeforePostProcess(edit);
    }
}

bool Huggle::Hooks::RevertPreflight(Huggle::WikiEdit *edit)
{
    bool result = true;
    foreach(Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
        {
            if (!extension->Hook_RevertPreflight(reinterpret_cast<void*>(edit)))
                result = false;
        }
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            if (!s->Hook_OnRevertPreflight(edit))
                result = false;
    }
    return result;
}

void Huggle::Hooks::EditAfterPostProcess(Huggle::WikiEdit *edit)
{
    Events::Global->on_QueryPoolFinishWEPostprocess(edit);
    if (edit == nullptr)
        throw new NullPointerException("Huggle::WikiEdit *Edit", BOOST_CURRENT_FUNCTION);

    foreach(Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
            extension->Hook_EditPostProcess(reinterpret_cast<void*>(edit));
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            s->Hook_EditPostProcess(edit);
    }
}

bool Huggle::Hooks::OnEditLoadToQueue(Huggle::WikiEdit *edit)
{
    foreach(Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
            if (!extension->Hook_OnEditLoadToQueue(reinterpret_cast<void*>(edit)))
                return false;
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            if (!s->Hook_EditLoadToQueue(edit))
                return false;
    }
    return true;
}

void Huggle::Hooks::OnGood(Huggle::WikiEdit *edit)
{
    Events::Global->on_WEGood(edit);
    foreach(Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
            e->Hook_GoodEdit(reinterpret_cast<void*>(edit));
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            s->Hook_OnGood(edit);
    }
}

void Huggle::Hooks::OnRevert(Huggle::WikiEdit *edit)
{
    Events::Global->on_WERevert(edit);
    foreach(Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
            e->Hook_OnRevert(reinterpret_cast<void*>(edit));
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            s->Hook_OnRevert(edit);
    }
}

bool Huggle::Hooks::EditCheckIfReady(Huggle::WikiEdit *edit)
{
    bool result = true;
    foreach(Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking() && !e->Hook_EditIsReady(reinterpret_cast<void*>(edit)))
            result = false;
    }
    return result;
}

void Huggle::Hooks::WikiEdit_ScoreJS(Huggle::WikiEdit *edit)
{
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
        {
            int score = s->Hook_EditRescore(edit);
            if (!score)
                continue;
            edit->Score += static_cast<long>(score);
            edit->PropertyBag.insert("score_js_" + s->GetName(), score);
        }
    }
}

void Huggle::Hooks::OnWarning(Huggle::WikiUser *user)
{
    Events::Global->on_WEWarningSent(user, user->GetWarningLevel());
}

void Huggle::Hooks::Suspicious(Huggle::WikiEdit *edit)
{
    Events::Global->on_WESuspicious(edit);
    foreach(Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
            e->Hook_OnSuspicious(reinterpret_cast<void*>(edit));
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            s->Hook_OnSuspicious(edit);
    }
}

void Huggle::Hooks::BadnessScore(Huggle::WikiUser *user, int score)
{
    if (user == nullptr)
        throw new NullPointerException("Huggle::WikiUser *User", BOOST_CURRENT_FUNCTION);

    foreach(Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
            extension->Hook_BadnessScore(reinterpret_cast<void*>(user), score);
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

void Huggle::Hooks::FeedProvidersOnInit(Huggle::WikiSite *site)
{
    foreach(Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
            extension->Hook_FeedProvidersOnInit(reinterpret_cast<void*>(site));
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            s->Hook_FeedProvidersOnInit(site);
    }
}

bool Huggle::Hooks::HAN_Suspicious(Huggle::WikiEdit *edit, QString nick, QString ident, QString host)
{
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking() && !s->Hook_HAN_Suspicious(edit, nick, ident, host))
            return false;
    }
    foreach (Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking() && !e->Hook_HAN_Suspicious(reinterpret_cast<void*>(edit), nick, ident, host))
            return false;
    }
    return true;
}

bool Huggle::Hooks::HAN_Revert(Huggle::WikiEdit *edit, QString nick, QString ident, QString host)
{
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking() && !s->Hook_HAN_Revert(edit, nick, ident, host))
            return false;
    }
    foreach (Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking() && !e->Hook_HAN_Revert(reinterpret_cast<void*>(edit), nick, ident, host))
            return false;
    }
    return true;
}

bool Huggle::Hooks::HAN_Good(Huggle::WikiEdit *edit, QString nick, QString ident, QString host)
{
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking() && !s->Hook_HAN_Good(edit, nick, ident, host))
            return false;
    }
    foreach (Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking() && !e->Hook_HAN_Good(reinterpret_cast<void*>(edit), nick, ident, host))
            return false;
    }
    return true;
}

bool Huggle::Hooks::HAN_Rescore(Huggle::WikiEdit *edit, long score, QString nick, QString ident, QString host)
{
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking() && !s->Hook_HAN_Rescore(edit, score, nick, ident, host))
            return false;
    }
    foreach (Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking() && !e->Hook_HAN_Rescore(reinterpret_cast<void*>(edit), score, nick, ident, host))
            return false;
    }
    return true;
}

bool Huggle::Hooks::HAN_Message(Huggle::WikiSite *site, QString message, QString nick, QString ident, QString host)
{
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking() && !s->Hook_HAN_Message(site, message, nick, ident, host))
            return false;
    }
    foreach (Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking() && !e->Hook_HAN_Message(reinterpret_cast<void*>(site), message, nick, ident, host))
            return false;
    }
    return true;
}

Huggle::Message *Huggle::Hooks::MessageUser(Huggle::WikiUser *user, QString text, QString title, QString summary, bool insert_section,
                                            Query *dependency, bool no_suffix, bool section_keep, bool autoremove,
                                            QString base_timestamp, bool create_only, bool fresh_only)
{
    foreach(Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
        {
            void *result = e->Hook_MessageUser(reinterpret_cast<void*>(user), text, title, summary, insert_section,
                                               reinterpret_cast<void*>(dependency), no_suffix,
                                               section_keep, autoremove, base_timestamp, create_only, fresh_only);
            if (result != nullptr)
                return reinterpret_cast<Huggle::Message*>(result);
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

void Huggle::Hooks::WarningFinished(Huggle::WikiEdit *edit)
{
    foreach(Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
            extension->Hook_WarningFinished(reinterpret_cast<void*>(edit));
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            s->Hook_WarningFinished(edit);
    }
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

void Huggle::Hooks::OnLocalConfigWrite()
{
    foreach(Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
            e->Hook_OnLocalConfigWrite();
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            s->Hook_OnLocalConfigWrite();
    }
}

void Huggle::Hooks::OnLocalConfigRead()
{
    foreach(Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
            e->Hook_OnLocalConfigRead();
    }
    foreach (Script *s, Script::GetScripts())
    {
        if (s->IsWorking())
            s->Hook_OnLocalConfigRead();
    }
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

