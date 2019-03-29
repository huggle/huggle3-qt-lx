//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef HUGGLEFEEDJS_HPP
#define HUGGLEFEEDJS_HPP

#include "../definitions.hpp"

#include "genericjsclass.hpp"
#include "../collectable_smartptr.hpp"
#include <QVariant>
#include <QHash>
#include <QObject>
#include <QString>
#include <QJSEngine>

namespace Huggle
{
    class HuggleFeedJS : public GenericJSClass
    {
            Q_OBJECT
        public:
            HuggleFeedJS(Script *s);
            Q_INVOKABLE QJSValue get_all_bytes_sent();
            Q_INVOKABLE QJSValue get_all_bytes_received();
            QHash<QString, QString> GetFunctions() override;
        private:
            QHash<QString, QString> functions;
    };
}

#endif // HUGGLEFEEDJS_HPP
