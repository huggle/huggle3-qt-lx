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
#include "uigeneric.hpp"
#include "mainwindow.hpp"
#include <QDir>
#include <huggle_core/configuration.hpp>
#include <huggle_core/generic.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/exception.hpp>

using namespace Huggle;

QList<UiScript*> UiScript::uiScripts;

QList<UiScript*> UiScript::GetAllUiScripts()
{
    return uiScripts;
}

void UiScript::Autostart()
{
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
                Huggle::Syslog::HuggleLogs->Log("Loaded JS script: " + name);
                UiScript::uiScripts.append(script);
            } else
            {
                Huggle::Syslog::HuggleLogs->Log("Failed to load a JS script: " + er);
                delete script;
            }
        }
    }
}

UiScript::UiScript()
{

}

UiScript::~UiScript()
{

}

QString UiScript::GetContext()
{
    return "huggle_ui";
}

unsigned int UiScript::GetContextID()
{
    return HUGGLE_SCRIPT_CONTEXT_UI;
}

int UiScript::RegisterMenu(QMenu *parent, QString title, QString fc)
{
    int id = this->lastMenu++;
    ScriptMenu *menu = new ScriptMenu(this, parent, title, fc);
    this->scriptMenus.insert(id, menu);
    return id;
}

static QScriptValue create_menu(QScriptContext *context, QScriptEngine *engine)
{
    UiScript *extension = (UiScript*)Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (!Huggle::MainWindow::HuggleMain)
    {
        HUGGLE_ERROR(extension->GetName() + ": create_menu(parent, name, function_name): mainwindow is not loaded yet");
        return QScriptValue(engine, false);
    }
    if (context->argumentCount() < 3)
    {
        // Wrong number of parameters
        HUGGLE_ERROR(extension->GetName() + ": create_menu(parent, name, function_name): requires 3 parameters");
        return QScriptValue(engine, false);
    }

    int parent = context->argument(1).toInt32();
    QString name = context->argument(2).toString();
    QString callback = context->argument(3).toString();
    QMenu *parentMenu = nullptr;
    if (parent < 0)
    {
        // Built-in menus
        int menu_id = -parent;
        parentMenu = MainWindow::HuggleMain->GetMenu(menu_id);
        if (!parentMenu)
        {
            HUGGLE_ERROR(extension->GetName() + ": create_menu(parent, name, function_name): invalid parent menu");
            return QScriptValue(engine, false);
        }
    } else if (parent > 0)
    {
        //if ()
    }

    int menu = extension->RegisterMenu(parentMenu, name, callback);
    return QScriptValue(engine, menu);
}

static QScriptValue message_box(QScriptContext *context, QScriptEngine *engine)
{
    Script *extension = Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 2)
    {
        // Wrong number of parameters
        HUGGLE_ERROR(extension->GetName() + ": message_box(title, text, type): requires 2 parameters");
        return QScriptValue(engine, false);
    }
    MessageBoxStyle type = MessageBoxStyleNormal;
    if (context->argumentCount() > 2)
        type = (MessageBoxStyle)context->argument(3).toInt32();
    if (type < 0 || type > 4)
    {
        HUGGLE_ERROR(extension->GetName() + ": message_box(title, text, type): invalid message box type");
        return QScriptValue(engine, false);
    }
    bool enforce_stop = false;
    if (context->argumentCount() > 3)
    {
        enforce_stop = context->argument(4).toBool();
    }
    QString title = context->argument(1).toString();
    QString text = context->argument(2).toString();
    UiGeneric::MessageBox(title, text, type, enforce_stop);
    return QScriptValue(engine, true);
}

void UiScript::registerFunctions()
{
    this->registerFunction("huggle_ui_create_menu", create_menu, 3, "(parent, name, function_name): Creates a new menu item");
    this->registerFunction("huggle_ui_message_box", message_box, 2, "(title, text, [type], [enforce_stop]): Show a message box");
    this->registerHook("ext_on_login_open", 0, "Called when login form is loaded");
    this->registerHook("ext_on_main_open", 0, "Called when main window is loaded");
    Script::registerFunctions();
}

ScriptMenu::ScriptMenu(UiScript *s, QMenu *parent, QString text, QString fc)
{
    this->callback = fc;
    this->script = s;
    this->parentMenu = parent;
    this->title = text;
}
