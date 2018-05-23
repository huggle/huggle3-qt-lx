//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef HUGGLEJS_HPP
#define HUGGLEJS_HPP

#include "../definitions.hpp"

#include "genericjsclass.hpp"
#include <QVariant>
#include <QHash>
#include <QObject>
#include <QString>
#include <QJSEngine>

// Since QJSEngine doesn't have newFunction yet https://bugreports.qt.io/browse/QTBUG-45246
// we need to use this workaround: https://forum.qt.io/topic/64407/newfunction-in-qjsengine-missing/6

class QTimer;

namespace Huggle
{
    /*!
     * \brief The HuggleJS class has C++ exports for JS engine (maps C++ functions to JS)
     *        it implements all functions that are available in JS script
     *        in "huggle" namespace
     */
    class HuggleJS : public GenericJSClass
    {
            Q_OBJECT
        public:
            HuggleJS(Script *s);
            ~HuggleJS();
            QHash<QString, QString> GetFunctions();
            Q_INVOKABLE QString get_context();
            Q_INVOKABLE int get_context_id();
            Q_INVOKABLE QList<QString> get_hook_list();
            Q_INVOKABLE QList<QString> get_function_list();
            Q_INVOKABLE QString get_function_help(QString function_name);
            Q_INVOKABLE QJSValue get_version();
            Q_INVOKABLE QString get_username();
            Q_INVOKABLE bool is_unsafe();
            Q_INVOKABLE bool has_function(QString function_name);
            Q_INVOKABLE bool register_hook(QString hook, QString function_name);
            Q_INVOKABLE void unregister_hook(QString hook);
            Q_INVOKABLE QString get_script_path();
            // Cfg
            Q_INVOKABLE QString get_cfg(QString key, QVariant default_value = "");
            Q_INVOKABLE void set_cfg(QString key, QVariant value);
            // Logging
            Q_INVOKABLE void log(QString text);
            Q_INVOKABLE void warning_log(QString text);
            Q_INVOKABLE void error_log(QString text);
            Q_INVOKABLE void debug_log(QString text, int verbosity);
            Q_INVOKABLE unsigned int create_timer(int interval, QString function, bool start = true);
            Q_INVOKABLE bool destroy_timer(unsigned int timer);
            Q_INVOKABLE bool start_timer(unsigned int timer, int interval);
            Q_INVOKABLE bool stop_timer(unsigned int timer);
            Q_INVOKABLE QJSValue get_site_by_name(QString site);
            Q_INVOKABLE QList<QString> get_sites();
            Q_INVOKABLE QString localize(QString id);
        private slots:
            void OnTime();
        private:
            unsigned int lastTimer = 0;
            QHash<unsigned int, QTimer*> timers;
            QHash<QTimer*, QString> timerFunctions;
            QHash<QString, QString> functions;
    };
}

#endif // HUGGLEJS_HPP
