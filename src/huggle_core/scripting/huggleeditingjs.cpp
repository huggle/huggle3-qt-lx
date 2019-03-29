//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2019

#include "huggleeditingjs.hpp"
#include "script.hpp"
#include "scriptfunctionhelp.hpp"
#include <huggle_core/syslog.hpp>
#include <huggle_core/wikipage.hpp>
#include <huggle_core/configuration.hpp>
#include <huggle_core/editquery.hpp>
#include <huggle_core/querypool.hpp>
#include <huggle_core/wikiutil.hpp>

using namespace Huggle;

HuggleEditingJS::HuggleEditingJS(Script *s) : GenericJSClass(s)
{
    this->functions.insert("append_text", "(string page_name, string text, string summary, [bool minor = false]): appends text to a wiki page");
    this->functions.insert("patrol_edit", "(WikiEdit edit): (3.4.5) mark edit as patrolled");
}

QHash<QString, QString> HuggleEditingJS::GetFunctions()
{
    return this->functions;
}

QHash<QString, ScriptFunctionHelp> HuggleEditingJS::GetFunctionHelp()
{
    QHash <QString, ScriptFunctionHelp> help;
    help.insert("append_text", ScriptFunctionHelp("appends text to a wiki page", "string page_name, string text, string summary, [bool minor = false]"));
    help.insert("patrol_edit", ScriptFunctionHelp("mark edit as patrolled", "WikiEdit edit", "bool: True on success", "3.4.5"));
    return help;
}

void HuggleEditingJS::append_text(const QString& page_name, const QString& text, const QString &summary, bool minor)
{
    QueryPool::HugglePool->AppendQuery(WikiUtil::AppendTextToPage(page_name, text, summary, minor).GetPtr());
}

bool HuggleEditingJS::patrol_edit(const QJSValue& edit)
{
    if (!edit.hasProperty("_ptr"))
    {
        HUGGLE_ERROR(this->script->GetName() + ": patrol_edit(edit): edit structure is missing _ptr");
        return false;
    }

    int pool_id = edit.property("_ptr").toInt();
    WikiEdit *ex = this->script->GetMemPool()->GetEdit(pool_id);
    if (!ex)
    {
        HUGGLE_ERROR(this->script->GetName() + ": patrol_edit(edit): null edit _ptr");
        return false;
    }
    WikiUtil::PatrolEdit(ex);
    return true;
}
