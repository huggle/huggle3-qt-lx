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
#include "../genericbrowser.hpp"
#include "../overlaybox.hpp"
#include "../hugglequeue.hpp"
#include <huggle_core/scripting/jsmarshallinghelper.hpp>
#include <huggle_core/wikiutil.hpp>
#include <huggle_core/wikisite.hpp>
#include <huggle_core/syslog.hpp>
#include <QUrl>
#include <QDesktopServices>
#include <QFileDialog>
#include <QInputDialog>

using namespace Huggle;

HuggleUIJS::HuggleUIJS(Script *s) : GenericJSClass(s)
{
    this->ui_script = (UiScript*) s;
    this->function_help.insert("render_html", "(string html, [bool lock_page]): Renders html in current tab");
    this->function_help.insert("external_link", "(string link): (since HG 3.4.4) opens a link in external browser");
    this->function_help.insert("internal_link", "(string link, [bool lock_page]): (since HG 3.4.4) opens a link in huggle browser");
    this->function_help.insert("mainwindow_is_loaded", "(): Returns true if main window is loaded");
    this->function_help.insert("menu_item_set_checked", "(int menu_id, bool state): toggles menu checked state");
    this->function_help.insert("get_current_wiki_edit", "(): returns a copy of currently displayed edit");
    this->function_help.insert("delete_menu_item", "(int menu_id): remove a menu that was created by this script");
    this->function_help.insert("create_menu_item", "(int parent, string name, string function_name, [bool checkable = false]): Creates a new menu item in main window, "\
                                                                    "this function works only if main window is loaded");
    this->function_help.insert("show_tray_message", "(string title, string text): shows a message in tray");
    this->function_help.insert("show_tooltip_message", "(string message): shows a tooltip");
    this->function_help.insert("show_overlay", "(string text, [int x, int y, int timeout, int width, int height, bool dismissable]): (3.4.5) display overlay message, similar to tooltip but supports more features");
    this->function_help.insert("destroy_persistent_overlay", "(int overlay): (3.4.5) removes a persistent overlay, returns false on failure");
    this->function_help.insert("show_persistent_overlay", "(string text, [int x, int y, int width, int height]): (3.4.5) display a persistent overlay message, returns its ID that can be used to manipulate it");
    this->function_help.insert("message_box", "(string title, string text, [int type], [enforce_stop]): Show a message box");
    this->function_help.insert("navigate_next", "(): move to next edit in queue");
    this->function_help.insert("navigate_backward", "(): move to previous edit");
    this->function_help.insert("navigate_forward", "(): move to next edit in history");
    this->function_help.insert("insert_edit_to_queue", "(string site_name, int revision_id): (since HG 3.4.3) inserts an edit to queue, filters will apply, edit will be processed so this function is asynchronous and results will be not be visible immediately");
    this->function_help.insert("highlight_text", "(string text): find a text in current browser window");
    this->function_help.insert("input_box", "(string title, string text, string default): (since HG 3.4.4) get input from user");
    this->function_help.insert("filebox_open", "(string title, string mask): (since HG 3.4.4) file box for opening of a file");
    this->function_help.insert("filebox_save", "(string title, string mask): (since HG 3.4.4) file box for saving of a file");
}

HuggleUIJS::~HuggleUIJS()
{
    foreach (OverlayBox *ob, this->overlayBoxes.values())
        ob->Close();
    this->overlayBoxes.clear();
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

bool HuggleUIJS::show_overlay(QString text, int x, int y, int timeout, int width, int height, bool is_dismissable)
{
    if (!MainWindow::HuggleMain)
        return false;

    MainWindow::HuggleMain->ShowOverlay(text, x, y, timeout, width, height, is_dismissable);
    return true;
}

int HuggleUIJS::show_persistent_overlay(QString text, int x, int y, int width, int height)
{
    if (!MainWindow::HuggleMain)
        return -1;
    OverlayBox *ob = MainWindow::HuggleMain->ShowOverlay(text, x, y, 0, width, height, false);
    int id = this->lastOB++;
    ob->SetPersistent(true);
    connect(ob, SIGNAL(destroyed(QObject*)), this, SLOT(OverlayClosed(QObject*)));
    this->overlayBoxes.insert(id, ob);
    return id;
}

bool HuggleUIJS::destroy_persistent_overlay(int overlay)
{
    if (!this->overlayBoxes.contains(overlay))
        return false;
    disconnect(this->overlayBoxes[overlay], SIGNAL(destroyed(QObject*)), this, SLOT(OverlayClosed(QObject*)));
    this->overlayBoxes[overlay]->Close();
    this->overlayBoxes.remove(overlay);
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

bool HuggleUIJS::insert_edit_to_queue(QString site_name, int rev_id)
{
    if (!Huggle::MainWindow::HuggleMain)
    {
        HUGGLE_ERROR(this->script->GetName() + ": insert_edit_to_queue(...): mainwindow is not loaded yet");
        return false;
    }
    // Get the site
    WikiSite *site = WikiUtil::GetSiteByName(site_name);
    if (!site)
    {
        HUGGLE_ERROR(this->script->GetName() + ": insert_edit_to_queue(...): unknown site");
        return false;
    }
    MainWindow::HuggleMain->Queue1->AddUnprocessedEditFromRevID(static_cast<revid_ht>(rev_id), site);
    return true;
}

bool HuggleUIJS::highlight_text(QString text)
{
    if (!MainWindow::HuggleMain)
    {
        HUGGLE_ERROR(this->script->GetName() + ": insert_edit_to_queue(...): mainwindow is not loaded yet");
        return false;
    }
    MainWindow::HuggleMain->Browser->Find(text);
    return true;
}

QJSValue HuggleUIJS::input_box(QString title, QString text, QString default_text)
{
    bool ok;
    QString results = QInputDialog::getText(nullptr, title, text, QLineEdit::Normal, default_text, &ok);
    if (!ok)
        return QJSValue(false);
    return QJSValue(results);
}

QJSValue HuggleUIJS::filebox_open(QString title, QString mask)
{
    QFileDialog file_dialog;
    file_dialog.setNameFilter(mask);
    file_dialog.setWindowTitle(title);
    file_dialog.setFileMode(QFileDialog::FileMode::ExistingFile);
    if (file_dialog.exec() == QDialog::DialogCode::Rejected || file_dialog.selectedFiles().count() == 0)
        return QJSValue(false);

    return QJSValue(file_dialog.selectedFiles().at(0));
}

QJSValue HuggleUIJS::filebox_save(QString title, QString mask)
{
    QFileDialog file_dialog;
    file_dialog.setNameFilter(mask);
    file_dialog.setWindowTitle(title);
    file_dialog.setFileMode(QFileDialog::FileMode::AnyFile);
    if (file_dialog.exec() == QDialog::DialogCode::Rejected || file_dialog.selectedFiles().count() == 0)
        return QJSValue(false);

    return QJSValue(file_dialog.selectedFiles().at(0));
}

void HuggleUIJS::external_link(QString link)
{
    QDesktopServices::openUrl(QUrl(link));
}

bool HuggleUIJS::internal_link(QString link, bool lock_page)
{
    if (!MainWindow::HuggleMain)
    {
        HUGGLE_ERROR(this->script->GetName() + ": internal_link(...): mainwindow is not loaded yet");
        return false;
    }
    MainWindow::HuggleMain->DisplayURL(link);
    if (lock_page)
    {
        MainWindow::HuggleMain->LockPage();
    }
    return true;
}

QHash<QString, QString> HuggleUIJS::GetFunctions()
{
    return this->function_help;
}

void HuggleUIJS::OverlayClosed(QObject *ob)
{
    foreach (int i, this->overlayBoxes.keys())
    {
        if (this->overlayBoxes[i] == ob)
        {
            this->overlayBoxes.remove(i);
            return;
        }
    }
}
