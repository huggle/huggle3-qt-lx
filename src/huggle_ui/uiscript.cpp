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
#include <QAction>
#include <QMenu>
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
    UiScript::uiScripts.append(this);
}

UiScript::~UiScript()
{
    if (UiScript::uiScripts.contains(this))
        UiScript::uiScripts.removeAll(this);
    foreach (ScriptMenu *menu, this->scriptMenus.values())
        delete menu;
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

int UiScript::RegisterMenu(QMenu *parent, QString title, QString fc)
{
    int id = this->lastMenu++;
    ScriptMenu *menu = new ScriptMenu(this, parent, title, fc);
    this->scriptMenusByAction.insert(menu->GetAction(), menu);
    this->scriptMenus.insert(id, menu);
    return id;
}

void UiScript::Hook_OnMain()
{
    this->executeFunction("ext_on_main_open");
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

    int parent = context->argument(0).toInt32();
    QString name = context->argument(1).toString();
    QString callback = context->argument(2).toString();
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
        type = (MessageBoxStyle)context->argument(2).toInt32();
    if (type < 0 || type > 4)
    {
        HUGGLE_ERROR(extension->GetName() + ": message_box(title, text, type): invalid message box type");
        return QScriptValue(engine, false);
    }
    bool enforce_stop = false;
    if (context->argumentCount() > 3)
    {
        enforce_stop = context->argument(3).toBool();
    }
    QString title = context->argument(0).toString();
    QString text = context->argument(1).toString();
    UiGeneric::MessageBox(title, text, type, enforce_stop);
    return QScriptValue(engine, true);
}

static QScriptValue render_html(QScriptContext *context, QScriptEngine *engine)
{
    Script *extension = Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        HUGGLE_ERROR(extension->GetName() + ": render_html(html [, lock_page]): requires 1 parameters");
        return QScriptValue(engine, false);
    }
    if (!Huggle::MainWindow::HuggleMain)
    {
        HUGGLE_ERROR(extension->GetName() + ": render_html(html): mainwindow is not loaded yet");
        return QScriptValue(engine, false);
    }
    QString source = context->argument(0).toString();
    bool disable_interface = false;

    if (context->argumentCount() > 1)
        disable_interface = context->argument(1).toBool();

    MainWindow::HuggleMain->RenderHtml(source);

    if (disable_interface)
        MainWindow::HuggleMain->LockPage();

    return QScriptValue(engine, true);
}

void UiScript::registerFunctions()
{
    this->registerFunction("huggle_ui_render_html", render_html, 1, "(string html, [bool lock_page]): Renders html in current tab");
    this->registerFunction("huggle_ui_create_menu", create_menu, 3, "(int parent, string name, string function_name): Creates a new menu item");
    this->registerFunction("huggle_ui_message_box", message_box, 2, "(string title, string text, [int type], [enforce_stop]): Show a message box");
    this->registerHook("ext_on_login_open", 0, "Called when login form is loaded");
    this->registerHook("ext_on_main_open", 0, "Called when main window is loaded");
    Script::registerFunctions();
}

ScriptMenu::ScriptMenu(UiScript *s, QMenu *parent, QString text, QString fc, QAction *before)
{
    this->callback = fc;
    this->script = s;
    this->parentMenu = parent;
    this->title = text;
    this->item = new QAction(text, (QObject*)parent);
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

QString ScriptMenu::GetCallback()
{
    return this->callback;
}
