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
    class UiScript;
    class HuggleUIJS : public GenericJSClass
    {
            Q_OBJECT
        public:
            HuggleUIJS(Script *s);
            Q_INVOKABLE int create_menu_item(int parent, QString name, QString function, bool checkable = false);
            Q_INVOKABLE bool delete_menu_item(int menu_id);
            Q_INVOKABLE bool menu_item_set_checked(int menu, bool checked);
            Q_INVOKABLE bool mainwindow_is_loaded();
            Q_INVOKABLE int message_box(QString title, QString text, int messagebox_type = 0, bool pause = false);
            Q_INVOKABLE bool render_html(QString html, bool lock_page = false);
            Q_INVOKABLE QJSValue get_current_wiki_edit();
            QHash<QString, QString> GetFunctions();
        private:
            QHash<QString, QString> function_help;
            UiScript *ui_script;
    };
}

#endif // HUGGLEUIJS_HPP
