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
#include "mainwindow.hpp"
#include "syslog.hpp"
#include "exception.hpp"

void Huggle::Hooks::EditPreProcess(Huggle::WikiEdit *Edit)
{
    if (Edit == nullptr)
    {
        throw new Huggle::Exception("Huggle::WikiEdit *Edit must not be nullptr", "void Huggle::Hooks::EditPreProcess(Huggle::WikiEdit *Edit)");
    }
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

void Huggle::Hooks::EditPostProcess(Huggle::WikiEdit *Edit)
{
    if (Edit == nullptr)
    {
        throw new Exception("Huggle::WikiEdit *Edit must not be nullptr", "void Huggle::Hooks::EditPreProcess(Huggle::WikiEdit *Edit)");
    }
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
}

void Huggle::Hooks::OnRevert(Huggle::WikiEdit *Edit)
{
    Core::HuggleCore->Main->VandalDock->Rollback(Edit);
}

void Huggle::Hooks::OnWarning(Huggle::WikiUser *User)
{
    Core::HuggleCore->Main->VandalDock->WarningSent(User, User->WarningLevel);
}

void Huggle::Hooks::Suspicious(Huggle::WikiEdit *Edit)
{
    Core::HuggleCore->Main->VandalDock->SuspiciousWikiEdit(Edit);
}

void Huggle::Hooks::BadnessScore(Huggle::WikiUser *User, int Score)
{
    if (User == nullptr)
    {
        throw new Exception("Huggle::WikiUser *User must not be nullptr", "void Huggle::Hooks::BadnessScore(Huggle::WikiUser "\
                            "*User, int Score)");
    }
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

void Huggle::Hooks::MainWindowIsLoaded(Huggle::MainWindow *window)
{
    int extension = 0;
    while (extension < Huggle::Core::HuggleCore->Extensions.count())
    {
        Huggle::iExtension *e = Huggle::Core::HuggleCore->Extensions.at(extension);
        if (e->IsWorking())
        {
            e->Hook_MainWindowOnLoad((void*)window);
        }
        extension++;
    }
#ifdef PYTHONENGINE
    Huggle::Core::HuggleCore->Python->Hook_MainWindowIsLoaded();
#endif
}
