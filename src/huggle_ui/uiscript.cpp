//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "uiscript.hpp"
#include <QDir>
#include <huggle_core/configuration.hpp>
#include <huggle_core/generic.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/exception.hpp>

using namespace Huggle;

QList<UiScript*> UiScript::uiScripts;

QList<UiScript*> UiScript::GetAllUiScripts()
{
    return uiScripts;
}

void UiScript::Autostart()
{
    QStringList extensions, files;;
    QString script_path = Configuration::GetExtensionsRootPath();
    if (QDir().exists(script_path))
    {
        QDir d(script_path);
        files = d.entryList();
        foreach (QString e_, files)
        {
            // we need to prefix the files here so that we can track the full path
            extensions << script_path + e_;
        }
    }
#ifdef HUGGLE_GLOBAL_EXTENSION_PATH
    QString global_path = Generic::SanitizePath(QString(HUGGLE_GLOBAL_EXTENSION_PATH) + "/");
    if (QDir().exists(global_path))
    {
        QDir g(global_path);
        files = g.entryList();
        foreach (QString e_, files)
        {
            // we need to prefix the files here so that we can track the full path
            extensions << global_path + e_;
        }
    }
#endif
    foreach (QString name, extensions)
    {
        QString lower_name = name.toLower();
        if (lower_name.endsWith(".jsu"))
        {
            QString er;
            UiScript *script = new UiScript();
            if (script->Load(name, &er))
            {
                Huggle::Syslog::HuggleLogs->Log("Loaded JS script: " + name);
                UiScript::uiScripts.append(script);
            } else
            {
                Huggle::Syslog::HuggleLogs->Log("Failed to load a JS script: " + er);
                delete script;
            }
        }
    }
}

UiScript::UiScript()
{

}

UiScript::~UiScript()
{

}

QString UiScript::GetContext()
{
    return "huggle_ui";
}

unsigned int UiScript::GetContextID()
{
    return 1;
}

void UiScript::registerFunctions()
{
    this->registerHook("ext_on_login_open", 0, "Called when login form is loaded");
    this->registerHook("ext_on_main_open", 0, "Called when main window is loaded");
    Script::registerFunctions();
}

ScriptMenu::ScriptMenu(UiScript *s)
{
    this->script = s;
}
