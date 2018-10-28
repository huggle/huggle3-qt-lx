//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "huggleeditjs.hpp"
#include "jsmarshallinghelper.hpp"
#include "script.hpp"
#include "../apiquery.hpp"
#include "../configuration.hpp"
#include "../syslog.hpp"
#include "../querypool.hpp"
#include "../wikiedit.hpp"
#include "../wikipage.hpp"
#include "../wikiutil.hpp"
#include "../wikisite.hpp"

using namespace Huggle;

HuggleEditJS::HuggleEditJS(Script *s) : GenericJSClass(s)
{

}

QHash<QString, QString> HuggleEditJS::GetFunctions()
{
    QHash<QString, QString> function_help;
    function_help.insert("get_edit_property_bag", "(WikiEdit edit): returns a property bag for a given edit");
    function_help.insert("get_edit_meta_data", "(edit): returns meta data of edit");
    function_help.insert("record_score", "(edit, name, score): record custom score for edit");
    return function_help;
}

QJSValue HuggleEditJS::get_edit_property_bag(QJSValue edit)
{
    WikiEdit *we = getEdit("get_edit_property_bag(edit)", edit);
    if (!we)
        return false;
    return JSMarshallingHelper::FromQVariantHash(we->PropertyBag, this->GetScript()->GetEngine());
}

QJSValue HuggleEditJS::get_edit_meta_data(QJSValue edit)
{
    WikiEdit *we = getEdit("get_edit_property_bag(edit)", edit);
    if (!we)
        return false;
    return JSMarshallingHelper::FromQStringHash(we->MetaLabels, this->GetScript()->GetEngine());
}

bool HuggleEditJS::record_score(QJSValue edit, QString name, int score)
{
    WikiEdit *we = getEdit("record_score(edit, name, score)", edit);
    if (!we)
        return false;
    we->RecordScore("ext_" + this->script->GetName() + "_" + name, (score_ht)score);
    return true;
}

WikiEdit *HuggleEditJS::getEdit(QString fc, QJSValue edit)
{
    if (!edit.hasProperty("_ptr"))
    {
        HUGGLE_ERROR(this->script->GetName() + ": " + fc + ": edit structure is missing _ptr");
        return NULL;
    }

    int pool_id = edit.property("_ptr").toInt();
    WikiEdit *ex = this->script->GetMemPool()->GetEdit(pool_id);
    if (!ex)
    {
        HUGGLE_ERROR(this->script->GetName() + ": " + fc + ": null edit _ptr");
        return NULL;
    }
    return ex;
}
