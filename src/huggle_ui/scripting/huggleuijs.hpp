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

namespace Huggle
{
    class HuggleUIJS : public GenericJSClass
    {
            Q_OBJECT
        public:
            HuggleUIJS(Script *s);
    };
}

#endif // HUGGLEUIJS_HPP
