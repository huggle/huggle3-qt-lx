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
#ifdef HUGGLE_PYTHON
    Huggle::Core::HuggleCore->Python->Hook_OnEditPreProcess(Edit);
#endif
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
#ifdef HUGGLE_PYTHON
    //Huggle::Core::HuggleCore->Python->Hook_OnEditPreProcess(Edit);
#endif
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
#ifdef HUGGLE_PYTHON
    Huggle::Core::HuggleCore->Python->Hook_OnEditPostProcess(Edit);
#endif
}

void Huggle::Hooks::OnGood(Huggle::WikiEdit *Edit)
{
    MainWindow::HuggleMain->VandalDock->Good(Edit);
    foreach(Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
            e->Hook_GoodEdit((void*)Edit);
    }
#ifdef HUGGLE_PYTHON
    Huggle::Core::HuggleCore->Python->Hook_GoodEdit(Edit);
#endif
}

void Huggle::Hooks::OnRevert(Huggle::WikiEdit *Edit)
{
    MainWindow::HuggleMain->VandalDock->Rollback(Edit);
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
    MainWindow::HuggleMain->VandalDock->WarningSent(User, User->GetWarningLevel());
}

void Huggle::Hooks::Suspicious(Huggle::WikiEdit *Edit)
{
    MainWindow::HuggleMain->VandalDock->SuspiciousWikiEdit(Edit);
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

void Huggle::Hooks::MainWindow_OnRender()
{
    foreach(Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
            e->Hook_MainWindowOnRender();
    }
#ifdef HUGGLE_PYTHON
    //Huggle::Core::HuggleCore->Python->Hook_MainWindowOnRender();
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
               result = false;
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
                result = false;
        }
    }
    return result;
}

