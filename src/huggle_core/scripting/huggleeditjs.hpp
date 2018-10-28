//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef HUGGLEEDITJS_HPP
#define HUGGLEEDITJS_HPP

#include "../definitions.hpp"
#include "genericjsclass.hpp"
#include <QVariant>
#include <QHash>
#include <QString>
#include <QJSEngine>

namespace Huggle
{
    class WikiEdit;
    class HuggleEditJS : public GenericJSClass
    {
            Q_OBJECT
        public:
            HuggleEditJS(Script *s);
            QHash<QString, QString> GetFunctions();
            Q_INVOKABLE QJSValue get_edit_property_bag(QJSValue edit);
            Q_INVOKABLE QJSValue get_edit_meta_data(QJSValue edit);
            Q_INVOKABLE bool record_score(QJSValue edit, QString name, int score);
        private:
            WikiEdit *getEdit(QString fc, QJSValue edit);
    };
}

#endif // HUGGLEEDITJS_HPP
