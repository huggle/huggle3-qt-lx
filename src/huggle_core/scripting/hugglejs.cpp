//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "hugglejs.hpp"
#include "script.hpp"
#include "jsmarshallinghelper.hpp"
#include "../configuration.hpp"
#include "../syslog.hpp"

using namespace Huggle;

HuggleJS::HuggleJS(Script *s) : GenericJSClass(s)
{

}

QString HuggleJS::get_context()
{
    return this->script->GetContext();
}

int HuggleJS::get_context_id()
{
    return static_cast<int>(this->script->GetContextID());
}

QList<QString> HuggleJS::get_hook_list()
{
    return this->GetScript()->GetHooks();
}

QList<QString> HuggleJS::get_function_list()
{
    return this->script->GetFunctions();
}

QString HuggleJS::get_function_help(QString function_name)
{
    return this->script->GetHelpForFunc(function_name);
}

QJSValue HuggleJS::get_version()
{
    int major = HUGGLE_BYTE_VERSION_MAJOR;
    int minor = HUGGLE_BYTE_VERSION_MINOR;
    int revision = HUGGLE_BYTE_VERSION_RELEASE;

    // Marshalling
    QJSValue version = this->script->GetEngine()->newObject();
    version.setProperty("Major", QJSValue(major));
    version.setProperty("Minor", QJSValue(minor));
    version.setProperty("Revision", QJSValue(revision));
    version.setProperty("String", QJSValue(hcfg->HuggleVersion));
    return version;
}

bool HuggleJS::is_unsafe()
{
    return this->script->IsUnsafe();
}

bool HuggleJS::has_function(QString function_name)
{
    return this->script->SupportFunction(function_name);
}

QString HuggleJS::get_cfg(QString key, QVariant default_value)
{
    return hcfg->GetExtensionConfig("script_" + this->script->GetName(), key, default_value.toString());
}

void HuggleJS::set_cfg(QString key, QVariant value)
{
    hcfg->SetExtensionConfig("script_" + this->script->GetName(), key, value.toString());
}

void HuggleJS::log(QString text)
{
    HUGGLE_LOG(text);
}

void HuggleJS::warning_log(QString text)
{
    HUGGLE_WARNING(this->script->GetName() + ": " + text);
}

void HuggleJS::error_log(QString text)
{
    HUGGLE_ERROR(this->script->GetName() + ": " + text);
}

void HuggleJS::debug_log(QString text, int verbosity)
{
    HUGGLE_DEBUG(this->script->GetName() + ": " + text, verbosity);
}
