//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "scriptfunctionhelp.hpp"

using namespace Huggle;

ScriptFunctionHelp::ScriptFunctionHelp(const QString &function_help, const QString &function_params, const QString &function_return,
                                       const QString &min_version, const QString &detailed_help, const QString &function_examples)
{
    this->FunctionHelp = function_help;
    this->FunctionParams = function_params;
    this->FunctionReturn = function_return;
    this->MinVersion = min_version;
    if (!detailed_help.isEmpty())
        this->FunctionDocs = detailed_help;
    else
        this->FunctionDocs = function_help;
    this->FunctionExamples = function_examples;
}
