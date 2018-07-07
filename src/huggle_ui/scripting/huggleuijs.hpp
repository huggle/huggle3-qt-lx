//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef HUGGLEUIJS_HPP
#define HUGGLEUIJS_HPP

#include <huggle_core/definitions.hpp>
#include <huggle_core/scripting/genericjsclass.hpp>
#include <QHash>
#include <QJSValue>

namespace Huggle
{
    class OverlayBox;
    class UiScript;
    class HuggleUIJS : public GenericJSClass
    {
            Q_OBJECT
        public:
            HuggleUIJS(Script *s);
            ~HuggleUIJS();
            Q_INVOKABLE int create_menu_item(int parent, QString name, QString function, bool checkable = false);
            Q_INVOKABLE bool delete_menu_item(int menu_id);
            Q_INVOKABLE bool menu_item_set_checked(int menu, bool checked);
            Q_INVOKABLE bool mainwindow_is_loaded();
            Q_INVOKABLE int message_box(QString title, QString text, int messagebox_type = 0, bool pause = false);
            Q_INVOKABLE bool show_tray_message(QString heading, QString message);
            Q_INVOKABLE bool show_overlay(QString text, int x = -1, int y = -1, int timeout = 10000, int width = -1, int height = -1, bool is_dismissable = false);
            Q_INVOKABLE bool set_overlay_text(int overlay_id, QString text);
            Q_INVOKABLE int show_persistent_overlay(QString text, int x = -1, int y = -1, int width = -1, int height = -1);
            Q_INVOKABLE bool destroy_persistent_overlay(int overlay);
            Q_INVOKABLE bool show_tooltip_message(QString message);
            Q_INVOKABLE bool render_html(QString html, bool lock_page = false);
            Q_INVOKABLE bool navigate_next();
            Q_INVOKABLE bool navigate_forward();
            Q_INVOKABLE bool navigate_backward();
            Q_INVOKABLE QJSValue get_current_wiki_edit();
            Q_INVOKABLE bool insert_edit_to_queue(QString site_name, int rev_id);
            Q_INVOKABLE bool highlight_text(QString text);
            Q_INVOKABLE QJSValue input_box(QString title, QString text, QString default_text);
            Q_INVOKABLE QJSValue filebox_open(QString title, QString mask = "All files (*)");
            Q_INVOKABLE QJSValue filebox_save(QString title, QString mask = "All files (*)");
            Q_INVOKABLE void external_link(QString link);
            Q_INVOKABLE bool internal_link(QString link, bool lock_page = false);
            QHash<QString, QString> GetFunctions();
        private slots:
            void OverlayClosed(QObject *ob);
        private:
            QHash<QString, QString> function_help;
            QHash<int, OverlayBox*> overlayBoxes;
            int lastOB = 0;
            UiScript *ui_script;
    };
}

#endif // HUGGLEUIJS_HPP
