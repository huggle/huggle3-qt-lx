//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef SCRIPTFUNCTIONHELP_HPP
#define SCRIPTFUNCTIONHELP_HPP

#include "../definitions.hpp"

namespace Huggle
{
    class HUGGLE_EX_CORE ScriptFunctionHelp
    {
        public:
            ScriptFunctionHelp()=default;
            ScriptFunctionHelp(const QString &function_help, const QString &function_params, const QString &function_return = "void",
                               const QString &min_version = "3.4.0", const QString &detailed_help = "", const QString &function_examples = "");
            QString FunctionName;
            QString FunctionHelp;
            QString FunctionParams;
            QString FunctionReturn;
            QString FunctionDocs;
            QString FunctionExamples;
            QString MinVersion;
    };
}

#endif // SCRIPTFUNCTIONHELP_HPP
