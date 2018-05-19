//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef HUGGLEEDITINGJS_HPP
#define HUGGLEEDITINGJS_HPP

#include "../definitions.hpp"

#include "genericjsclass.hpp"
#include <QVariant>
#include <QHash>
#include <QObject>
#include <QString>
#include <QJSEngine>

// Since QJSEngine doesn't have newFunction yet https://bugreports.qt.io/browse/QTBUG-45246
// we need to use this workaround: https://forum.qt.io/topic/64407/newfunction-in-qjsengine-missing/6

namespace Huggle
{
    /*!
     * \brief The HuggleJS class has C++ exports for JS engine (maps C++ functions to JS)
     *        it implements all functions that are available in JS script
     *        in "huggle" namespace
     */
    class HuggleEditingJS : public GenericJSClass
    {
            Q_OBJECT
        public:
            HuggleEditingJS(Script *s);
            QHash<QString, QString> GetFunctions();
            Q_INVOKABLE void append_text(QString page_name, QString text, QString summary, bool minor = false);

        private:
            QHash<QString, QString> functions;
    };
}

#endif // HUGGLEEDITINGJS_HPP
