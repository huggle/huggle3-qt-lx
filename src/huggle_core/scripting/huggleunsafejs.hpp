//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef HUGGLEUNSAFEJS_HPP
#define HUGGLEUNSAFEJS_HPP

#include "../definitions.hpp"

#include "genericjsclass.hpp"
#include <QVariant>
#include <QHash>
#include <QObject>
#include <QString>
#include <QJSEngine>

namespace Huggle
{
    class HuggleUnsafeJS : public GenericJSClass
    {
            Q_OBJECT
        public:
            HuggleUnsafeJS(Script *s);
            QHash<QString, QString> GetFunctions();
            Q_INVOKABLE bool append_string_to_file(QString text, QString file_path);
            Q_INVOKABLE QString read_file_to_string(QString file_path);
            Q_INVOKABLE QList<QString> read_file_to_list(QString file_path);
            Q_INVOKABLE bool write_string_to_file(QString text, QString file_path);
        private:
            QHash<QString, QString> function_help;
    };
}

#endif // HUGGLEUNSAFEJS_HPP
