//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef GENERICJSCLASS_HPP
#define GENERICJSCLASS_HPP

#include "../definitions.hpp"
#include <QObject>
#include <QString>
#include <QList>

namespace Huggle
{
    class Script;
    class GenericJSClass : public QObject
    {
            Q_OBJECT
        public:
            GenericJSClass(Script *s);
            virtual ~GenericJSClass();
            virtual Script *GetScript();

        protected:
            Script *script;
    };
}

#endif // GENERICJSCLASS_HPP
