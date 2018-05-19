//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "uihooks.hpp"
#include "speedyform.hpp"
#include "userinfoform.hpp"
#include "scripting/uiscript.hpp"
#include <huggle_core/iextension.hpp>
#include <huggle_core/core.hpp>

using namespace Huggle;

bool UiHooks::ContribBoxBeforeQuery(WikiUser *user, UserinfoForm *user_info)
{
    bool result = true;
    foreach (Huggle::iExtension *extension, Huggle::Core::HuggleCore->Extensions)
    {
        if (extension->IsWorking())
        {
            if (!extension->Hook_ContribBoxBeforeQuery((void*)user, (void*)user_info))
                result = false;
        }
    }
    return result;
}

void UiHooks::ContribBoxAfterQuery(WikiUser *user, UserinfoForm *user_info)
{

}

void UiHooks::MainWindow_OnRender()
{
    foreach(Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
            e->Hook_MainWindowOnRender();
    }
}

void UiHooks::MainWindow_OnLoad(Huggle::MainWindow *window)
{
    foreach (Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
            e->Hook_MainWindowOnLoad((void*)window);
    }
    foreach (UiScript *sc, UiScript::GetAllUiScripts())
    {
        if (sc->IsWorking())
            sc->Hook_OnMain();
    }
}

bool UiHooks::MainWindow_ReloadShortcut(Huggle::Shortcut *shortcut)
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

void UiHooks::Speedy_Finished(WikiEdit *edit, QString tags, bool success)
{
    foreach (Huggle::iExtension *e, Huggle::Core::HuggleCore->Extensions)
    {
        if (e->IsWorking())
            e->Hook_SpeedyFinished((void*)edit, tags, success);
    }
    foreach (UiScript *sc, UiScript::GetAllUiScripts())
    {
        if (sc->IsWorking())
            sc->Hook_OnSpeedyFinished(edit, tags, success);
    }
}

bool UiHooks::Speedy_BeforeOK(WikiEdit *edit, SpeedyForm *form)
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

void UiHooks::LoginForm_OnLoad(LoginForm *login_form)
{
    foreach (UiScript *sc, UiScript::GetAllUiScripts())
    {
        sc->Hook_OnLogin();
    }
}
