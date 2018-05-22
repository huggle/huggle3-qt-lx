//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "huggleuijs.hpp"
#include "uiscript.hpp"
#include "../uigeneric.hpp"
#include "../mainwindow.hpp"
#include <huggle_core/scripting/jsmarshallinghelper.hpp>
#include <huggle_core/syslog.hpp>

using namespace Huggle;

HuggleUIJS::HuggleUIJS(Script *s) : GenericJSClass(s)
{
    this->ui_script = (UiScript*) s;
    this->function_help.insert("render_html", "(string html, [bool lock_page]): Renders html in current tab");
    this->function_help.insert("mainwindow_is_loaded", "(): Returns true if main window is loaded");
    this->function_help.insert("menu_item_set_checked", "(int menu_id, bool state): toggles menu checked state");
    this->function_help.insert("get_current_wiki_edit", "(): returns a copy of currently displayed edit");
    this->function_help.insert("delete_menu_item", "(int menu_id): remove a menu that was created by this script");
    this->function_help.insert("create_menu_item", "(int parent, string name, string function_name, [bool checkable = false]): Creates a new menu item in main window, "\
                                                                    "this function works only if main window is loaded");
    this->function_help.insert("message_box", "(string title, string text, [int type], [enforce_stop]): Show a message box");
}

int HuggleUIJS::create_menu_item(int parent, QString name, QString function, bool checkable)
{
    if (!Huggle::MainWindow::HuggleMain)
    {
        HUGGLE_ERROR(this->script->GetName() + ": create_menu(parent, name, function_name): mainwindow is not loaded yet");
        return -1;
    }
    QMenu *parentMenu = nullptr;
    if (parent < 0)
    {
        // Built-in menus
        int menu_id = -parent;
        parentMenu = MainWindow::HuggleMain->GetMenu(menu_id);
        if (!parentMenu)
        {
            HUGGLE_ERROR(this->ui_script->GetName() + ": create_menu(parent, name, function_name): invalid parent menu");
            return -1;
        }
    } else if (parent > 0)
    {
        HUGGLE_ERROR(this->ui_script->GetName() + ": create_menu(parent, name, function_name): non-builtin menu not implemented yet");
        return -1;
    }

    int menu = this->ui_script->RegisterMenu(parentMenu, name, function, checkable);
    return menu;
}

bool HuggleUIJS::delete_menu_item(int menu_id)
{
    if (!Huggle::MainWindow::HuggleMain)
    {
        HUGGLE_ERROR(this->ui_script->GetName() + ": delete_menu(id): mainwindow is not loaded yet");
        return false;
    }

    if (!this->ui_script->OwnMenu(menu_id))
    {
        HUGGLE_ERROR(this->ui_script->GetName() + ": delete_menu(id): request to delete menu that is not owned by this script");
        return false;
    }

    this->ui_script->UnregisterMenu(menu_id);
    return true;
}

bool HuggleUIJS::menu_item_set_checked(int menu, bool checked)
{
    if (!Huggle::MainWindow::HuggleMain)
    {
        HUGGLE_ERROR(this->ui_script->GetName() + ": menu_item_set_checked(menu, checked): mainwindow is not loaded yet");
        return false;
    }

    if (!this->ui_script->OwnMenu(menu))
    {
        HUGGLE_ERROR(this->ui_script->GetName() + ": menu_item_set_checked(menu, checked): request to toggle state of menu that is not owned by this script");
        return false;
    }

    this->ui_script->ToggleMenuCheckState(menu, checked);
    return true;
}

bool HuggleUIJS::mainwindow_is_loaded()
{
    return Huggle::MainWindow::HuggleMain != nullptr;
}

int HuggleUIJS::message_box(QString title, QString text, int messagebox_type, bool pause)
{
    if (messagebox_type < 0 || messagebox_type > 4)
    {
        HUGGLE_ERROR(this->script->GetName() + ": message_box(title, text, type): invalid message box type");
        return -1;
    }
    MessageBoxStyle type = (MessageBoxStyle) messagebox_type;
    // Pass the value
    return UiGeneric::MessageBox(title, text, type, pause);
}

bool HuggleUIJS::show_tray_message(QString heading, QString message)
{
    if (!MainWindow::HuggleMain)
        return false;
    MainWindow::HuggleMain->TrayMessage(heading, message);
    return true;
}

bool HuggleUIJS::show_tooltip_message(QString message)
{
    if (!MainWindow::HuggleMain)
        return false;

    // Show tooltip
    MainWindow::HuggleMain->ShowToolTip(message);
    return true;
}

bool HuggleUIJS::render_html(QString html, bool lock_page)
{
    if (!Huggle::MainWindow::HuggleMain)
    {
        HUGGLE_ERROR(this->script->GetName() + ": render_html(html): mainwindow is not loaded yet");
        return false;
    }

    MainWindow::HuggleMain->RenderHtml(html);

    if (lock_page)
        MainWindow::HuggleMain->LockPage();

    return true;
}

bool HuggleUIJS::navigate_next()
{
    if (!Huggle::MainWindow::HuggleMain)
    {
        HUGGLE_ERROR(this->script->GetName() + ": navigate_next(): mainwindow is not loaded yet");
        return false;
    }
    Huggle::MainWindow::HuggleMain->DisplayNext();
    return true;
}

bool HuggleUIJS::navigate_forward()
{
    if (!Huggle::MainWindow::HuggleMain)
    {
        HUGGLE_ERROR(this->script->GetName() + ": navigate_forward(): mainwindow is not loaded yet");
        return false;
    }
    Huggle::MainWindow::HuggleMain->GoForward();
    return true;
}

bool HuggleUIJS::navigate_backward()
{
    if (!Huggle::MainWindow::HuggleMain)
    {
        HUGGLE_ERROR(this->script->GetName() + ": navigate_backward(): mainwindow is not loaded yet");
        return false;
    }
    Huggle::MainWindow::HuggleMain->GoBackward();
    return true;
}

QJSValue HuggleUIJS::get_current_wiki_edit()
{
    if (!Huggle::MainWindow::HuggleMain)
    {
        HUGGLE_ERROR(this->script->GetName() + ": render_html(html): mainwindow is not loaded yet");
        return QJSValue(false);
    }

    WikiEdit *edit = Huggle::MainWindow::HuggleMain->GetCurrentWikiEdit();
    if (!edit)
        return QJSValue(QJSValue::SpecialValue::NullValue);

    return JSMarshallingHelper::FromEdit(edit, this->GetScript()->GetEngine());
}

QHash<QString, QString> HuggleUIJS::GetFunctions()
{
    return this->function_help;
}
