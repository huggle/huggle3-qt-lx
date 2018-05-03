//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "jsmarshallinghelper.hpp"
#include "generic.hpp"
#include "wikiedit.hpp"
#include "wikiuser.hpp"
#include "wikisite.hpp"
#include <QtScript>

using namespace Huggle;

QScriptValue JSMarshallingHelper::FromSite(WikiSite *site, QScriptEngine *engine)
{
    QScriptValue o = engine->newObject();
    o.setProperty("Name", QScriptValue(engine, site->Name));
    o.setProperty("ForceSSL", QScriptValue(engine, site->ForceSSL));
    return o;
}

QScriptValue JSMarshallingHelper::FromUser(WikiUser *user, QScriptEngine *engine)
{
    QScriptValue o = engine->newObject();
    o.setProperty("EditCount", QScriptValue(engine, static_cast<int>(user->EditCount)));
    o.setProperty("IsIP", QScriptValue(engine, user->IsIP()));
    o.setProperty("Username", QScriptValue(engine, user->Username));
    return o;
}

QScriptValue JSMarshallingHelper::FromEdit(WikiEdit *edit, QScriptEngine *engine)
{
    QScriptValue o = engine->newObject();
    o.setProperty("Bot", QScriptValue(engine, edit->Bot));
    o.setProperty("CurrentUserWarningLevel", QScriptValue(engine, static_cast<int>(edit->CurrentUserWarningLevel)));
    o.setProperty("Diff", QScriptValue(engine, static_cast<int>(edit->Diff)));
    o.setProperty("DiffText", QScriptValue(engine, edit->DiffText));
    o.setProperty("DiffText_IsSplit", QScriptValue(engine, edit->DiffText_IsSplit));
    o.setProperty("DiffText_New", QScriptValue(engine, edit->DiffText_New));
    o.setProperty("DiffText_Old", QScriptValue(engine, edit->DiffText_Old));
    o.setProperty("DiffTo", QScriptValue(engine, edit->DiffTo));
    return o;
}
