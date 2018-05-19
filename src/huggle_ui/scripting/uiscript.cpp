//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "uiscript.hpp"
#include "huggleuijs.hpp"
#include "../uigeneric.hpp"
#include "../mainwindow.hpp"
#include <QAction>
#include <QMenu>
#include <QDir>
#include <QCoreApplication>
#include <huggle_core/configuration.hpp>
#include <huggle_core/generic.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/exception.hpp>
#include <huggle_core/scripting/jsmarshallinghelper.hpp>

using namespace Huggle;

QList<UiScript*> UiScript::uiScripts;

QList<UiScript*> UiScript::GetAllUiScripts()
{
    return uiScripts;
}

void UiScript::Autostart()
{
    if (hcfg->SystemConfig_SafeMode)
        return;

    QStringList extensions, files;;
    QString script_path = Configuration::GetExtensionsRootPath();
    if (QDir().exists(script_path))
    {
        QDir d(script_path);
        files = d.entryList();
        foreach (QString e_, files)
        {
            // we need to prefix the files here so that we can track the full path
            extensions << script_path + e_;
        }
    }
#ifdef HUGGLE_GLOBAL_EXTENSION_PATH
    QString global_path = Generic::SanitizePath(QString(HUGGLE_GLOBAL_EXTENSION_PATH) + "/");
    if (QDir().exists(global_path))
    {
        QDir g(global_path);
        files = g.entryList();
        foreach (QString e_, files)
        {
            // we need to prefix the files here so that we can track the full path
            extensions << global_path + e_;
        }
    }
#endif
    foreach (QString name, extensions)
    {
        QString lower_name = name.toLower();
        if (lower_name.endsWith(".js"))
        {
            QString er;
            UiScript *script = new UiScript();
            if (script->Load(name, &er))
            {
                HUGGLE_LOG("Loaded JS script: " + name);
            } else
            {
                HUGGLE_ERROR("Failed to load a JS script: " + er);
                delete script;
            }
        }
    }
}

UiScript::UiScript()
{
    UiScript::uiScripts.append(this);
}

UiScript::~UiScript()
{
    if (UiScript::uiScripts.contains(this))
        UiScript::uiScripts.removeAll(this);
    while (!this->scriptMenus.isEmpty())
        this->UnregisterMenu(this->scriptMenus.keys().first());
    this->scriptMenusByAction.clear();
    this->scriptMenus.clear();
}

QString UiScript::GetContext()
{
    return "huggle_ui";
}

unsigned int UiScript::GetContextID()
{
    return HUGGLE_SCRIPT_CONTEXT_UI;
}

int UiScript::RegisterMenu(QMenu *parent, QString title, QString fc, bool checkable)
{
    int id = this->lastMenu++;
    ScriptMenu *menu = new ScriptMenu(this, parent, title, fc, checkable);
    this->scriptMenusByAction.insert(menu->GetAction(), menu);
    this->scriptMenus.insert(id, menu);
    return id;
}

void UiScript::UnregisterMenu(int menu)
{
    if (!this->scriptMenus.contains(menu))
        return;
    ScriptMenu *s = this->scriptMenus[menu];
    if (this->scriptMenusByAction.contains(s->GetAction()))
        this->scriptMenusByAction.remove(s->GetAction());
    this->scriptMenus.remove(menu);
    if (Huggle::MainWindow::HuggleMain)
    {
        if (s->GetParent())
            s->GetParent()->removeAction(s->GetAction());
    }
    delete s;
}

void UiScript::ToggleMenuCheckState(int menu, bool checked)
{
    if (!this->scriptMenus.contains(menu))
        return;
    this->scriptMenus[menu]->SetChecked(checked);
}

bool UiScript::OwnMenu(int menu_id)
{
    return this->scriptMenus.contains(menu_id);
}

void UiScript::Hook_OnMain()
{
    if (this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_MAIN_OPEN))
        this->executeFunction(this->attachedHooks[HUGGLE_SCRIPT_HOOK_MAIN_OPEN]);
}

void UiScript::Hook_OnLogin()
{
    if (this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_LOGIN_OPEN))
        this->executeFunction(this->attachedHooks[HUGGLE_SCRIPT_HOOK_LOGIN_OPEN]);
}

void UiScript::Hook_OnSpeedyFinished(WikiEdit *edit, QString tags, bool success)
{
    if (!this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_SPEEDY_FINISHED))
        return;

    QJSValueList parameters;
    parameters.append(JSMarshallingHelper::FromEdit(edit, this->engine));
    parameters.append(QJSValue(tags));
    parameters.append(QJSValue(success));
    this->executeFunction(this->attachedHooks[HUGGLE_SCRIPT_HOOK_SPEEDY_FINISHED], parameters);
}

int UiScript::GetHookID(QString hook)
{
    if (hook == "speedy_finished")
        return HUGGLE_SCRIPT_HOOK_SPEEDY_FINISHED;
    if (hook == "main_open")
        return HUGGLE_SCRIPT_HOOK_MAIN_OPEN;
    if (hook == "login_open")
        return HUGGLE_SCRIPT_HOOK_LOGIN_OPEN;

    return Script::GetHookID(hook);
}

void UiScript::MenuClicked()
{
    QAction *menu = (QAction*)QObject::sender();
    if (!this->scriptMenusByAction.contains(menu))
    {
        HUGGLE_ERROR(this->GetName() + ": invalid menu pointer: " + QString("0x%1").arg((quintptr)menu, QT_POINTER_SIZE * 2, 16, QChar('0')));
        return;
    }
    this->executeFunction(this->scriptMenusByAction[menu]->GetCallback());
}

void UiScript::registerClasses()
{
    this->registerClass("huggle_ui", new HuggleUIJS(this));
    Script::registerClasses();
}

void UiScript::registerFunctions()
{
    this->registerHook("speedy_finished", 0, "(WikiEdit edit, string tags, bool success): when speedy finish");
    this->registerHook("login_open", 0, "(): Called when login form is loaded");
    this->registerHook("main_open", 0, "(): Called when main window is loaded");

    Script::registerFunctions();
}

ScriptMenu::ScriptMenu(UiScript *s, QMenu *parent, QString text, QString fc, bool checkable, QAction *before)
{
    this->callback = fc;
    this->script = s;
    this->parentMenu = parent;
    this->title = text;
    this->item = new QAction(text, (QObject*)parent);
    this->item->setCheckable(checkable);
    if (parent)
        parent->insertAction(before, this->item);
    UiScript::connect(this->item, SIGNAL(triggered()), this->script, SLOT(MenuClicked()));
}

ScriptMenu::~ScriptMenu()
{
    delete this->item;
}

QAction *ScriptMenu::GetAction()
{
    return this->item;
}

void ScriptMenu::SetChecked(bool checked)
{
    this->item->setChecked(checked);
}

QString ScriptMenu::GetCallback()
{
    return this->callback;
}

QMenu *ScriptMenu::GetParent()
{
    return this->parentMenu;
}
