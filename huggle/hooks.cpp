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
#include "mainwindow.hpp"
#include "vandalnw.hpp"
#include "iextension.hpp"
#include "wikiuser.hpp"
#include "wikiedit.hpp"
#include "syslog.hpp"
#include "exception.hpp"
#include "wikipage.hpp"

void Huggle::Hooks::EditPreProcess(Huggle::WikiEdit *Edit)
{
    if (Edit == nullptr)
        throw new Huggle::NullPointerException("Huggle::WikiEdit *Edit", BOOST_CURRENT_FUNCTION);

    int extension = 0;
    while (extension < Huggle::Core::HuggleCore->Extensions.count())
    {
        Huggle::iExtension *e = Huggle::Core::HuggleCore->Extensions.at(extension);
        if (e->IsWorking())
        {
            e->Hook_EditPreProcess((void*)Edit);
        }
        extension++;
    }
}

bool Huggle::Hooks::RevertPreflight(Huggle::WikiEdit *Edit)
{
    bool result = true;
    int extension = 0;
    while (extension < Huggle::Core::HuggleCore->Extensions.count())
    {
        Huggle::iExtension *e = Huggle::Core::HuggleCore->Extensions.at(extension++);
        if (e->IsWorking())
        {
            if (!e->Hook_RevertPreflight((void*)Edit))
                result = false;
        }
    }
    return result;
}

void Huggle::Hooks::EditPostProcess(Huggle::WikiEdit *Edit)
{
    if (Edit == nullptr)
        throw new NullPointerException("Huggle::WikiEdit *Edit", BOOST_CURRENT_FUNCTION);

    int extension = 0;
    while (extension < Huggle::Core::HuggleCore->Extensions.count())
    {
        Huggle::iExtension *e = Huggle::Core::HuggleCore->Extensions.at(extension);
        if (e->IsWorking())
        {
            e->Hook_EditPostProcess((void*)Edit);
        }
        extension++;
    }
}

void Huggle::Hooks::OnGood(Huggle::WikiEdit *Edit)
{
    Core::HuggleCore->Main->VandalDock->Good(Edit);
    foreach(Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
            e->Hook_GoodEdit((void*)Edit);
    }
}

void Huggle::Hooks::OnRevert(Huggle::WikiEdit *Edit)
{
    Core::HuggleCore->Main->VandalDock->Rollback(Edit);
}

void Huggle::Hooks::OnWarning(Huggle::WikiUser *User)
{
    Core::HuggleCore->Main->VandalDock->WarningSent(User, User->GetWarningLevel());
}

void Huggle::Hooks::Suspicious(Huggle::WikiEdit *Edit)
{
    Core::HuggleCore->Main->VandalDock->SuspiciousWikiEdit(Edit);
}

void Huggle::Hooks::BadnessScore(Huggle::WikiUser *User, int Score)
{
    if (User == nullptr)
        throw new NullPointerException("Huggle::WikiUser *User", BOOST_CURRENT_FUNCTION);

    int extension = 0;
    while (extension < Huggle::Core::HuggleCore->Extensions.count())
    {
        Huggle::iExtension *e = Huggle::Core::HuggleCore->Extensions.at(extension);
        if (e->IsWorking())
        {
            e->Hook_BadnessScore((void*)User, Score);
        }
        extension++;
    }
}

void Huggle::Hooks::Speedy_Finished(Huggle::WikiEdit *edit, QString tags, bool success)
{
    foreach (Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
            e->Hook_SpeedyFinished((void*)edit, tags, success);
    }
#ifdef HUGGLE_PYTHON
    Huggle::Core::HuggleCore->Python->Hook_SpeedyFinished(edit, tags, success);
#endif
}

void Huggle::Hooks::MainWindow_OnLoad(Huggle::MainWindow *window)
{
    foreach (Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
            e->Hook_MainWindowOnLoad((void*)window);
    }
#ifdef HUGGLE_PYTHON
    Huggle::Core::HuggleCore->Python->Hook_MainWindowIsLoaded();
#endif
}

void Huggle::Hooks::Shutdown()
{
    foreach (Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if ( e->IsWorking() )
           e->Hook_Shutdown();
    }
#ifdef HUGGLE_PYTHON
    Huggle::Core::HuggleCore->Python->Hook_HuggleShutdown();
#endif
}

bool Huggle::Hooks::Speedy_BeforeOK(Huggle::WikiEdit *edit, Huggle::SpeedyForm *form)
{
    bool result = true;
    foreach (Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
        {
           if (!e->Hook_SpeedyBeforeOK((void*)edit, (void*)form))
           {
               result = false;
           }
        }
    }
    return result;
}

bool Huggle::Hooks::MainWindow_ReloadShortcut(Huggle::Shortcut *shortcut)
{
    bool result = true;
    foreach(Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
        {
            if (!e->Hook_MainWindowReloadShortcut((void*)shortcut))
            {
                result = false;
            }
        }
    }
    return result;
}

