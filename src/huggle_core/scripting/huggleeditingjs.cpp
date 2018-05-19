//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "huggleeditingjs.hpp"
#include <huggle_core/syslog.hpp>
#include <huggle_core/wikipage.hpp>
#include <huggle_core/configuration.hpp>
#include <huggle_core/editquery.hpp>
#include <huggle_core/wikiutil.hpp>

using namespace Huggle;

HuggleEditingJS::HuggleEditingJS(Script *s) : GenericJSClass(s)
{
    this->functions.insert("append_text", "(string page_name, string text, string summary, [bool minor = false]): appends text to a wiki page");
}

QHash<QString, QString> HuggleEditingJS::GetFunctions()
{
    return this->functions;
}

void HuggleEditingJS::append_text(QString page_name, QString text, QString summary, bool minor)
{
    WikiUtil::AppendTextToPage(page_name, text, summary, minor);
}
