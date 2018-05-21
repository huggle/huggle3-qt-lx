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
#include <QTimer>

using namespace Huggle;

HuggleJS::HuggleJS(Script *s) : GenericJSClass(s)
{
    this->functions.insert("get_function_help", "(string function_name): give you help for a function, returns string");
    this->functions.insert("get_function_list", "(): returns array with list of functions");
    this->functions.insert("get_hook_list", "(): returns a list of all hooks");
    this->functions.insert("get_version", "(): returns version object with properties: Major, Minor, Revision, String");
    this->functions.insert("is_unsafe", "(): returns true if script has access to unsafe functions");
    this->functions.insert("set_cfg", "(string key, string value): stores value as key in settings");
    this->functions.insert("get_cfg", "(string key, string default): returns stored value from ini file");
    this->functions.insert("get_script_path", "(): returns a path / URL of this script");
    this->functions.insert("has_function", "(string function_name): return true or false whether function is present");
    this->functions.insert("get_context", "(): return execution context, either core or GrumpyChat (core doesn't have ui functions and hooks)");
    this->functions.insert("debug_log", "(string text, int verbosity): prints to debug log");
    this->functions.insert("error_log", "(string text): prints to system error log");
    this->functions.insert("warning_log", "(string text): prints to warning log");
    this->functions.insert("log", "(string text): prints to log");
    this->functions.insert("register_hook", "(string hook, string function_id): creates a hook");
    this->functions.insert("unregister_hook", "(string hook): removes hook");
    this->functions.insert("create_timer", "(int interval, string function, [bool start = true]): creates a timer");
    this->functions.insert("start_timer", "(uint timer)");
    this->functions.insert("destroy_timer", "(uint timer)");
    this->functions.insert("stop_timer", "(uint timer");
}

HuggleJS::~HuggleJS()
{
    qDeleteAll(this->timers);
    this->timerFunctions.clear();
}

QHash<QString, QString> HuggleJS::GetFunctions()
{
    return this->functions;
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

bool HuggleJS::register_hook(QString hook, QString function_name)
{
    int hook_id = this->script->GetHookID(hook);
    if (hook_id < 0)
    {
        HUGGLE_ERROR(this->script->GetName() + ": register_hook(hook, fc): unknown hook: " + hook);
        return false;
    }
    this->script->SubscribeHook(hook_id, function_name);
    return true;
}

void HuggleJS::unregister_hook(QString hook)
{
    int hook_id = this->script->GetHookID(hook);
    if (hook_id < 0)
    {
        HUGGLE_ERROR(this->script->GetName() + ": unregister_hook(h): unknown hook: " + hook);
        return;
    }
    this->script->UnsubscribeHook(hook_id);
}

QString HuggleJS::get_script_path()
{
    return this->script->GetPath();
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
    HUGGLE_DEBUG(this->script->GetName() + ": " + text, static_cast<unsigned int>(verbosity));
}

unsigned int HuggleJS::create_timer(int interval, QString function, bool start)
{
    unsigned int timer_id = this->lastTimer++;
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(OnTime()));
    this->timers.insert(timer_id, timer);
    this->timerFunctions.insert(timer, function);
    if (start)
        timer->start(interval);
    return timer_id;
}

bool HuggleJS::destroy_timer(unsigned int timer)
{
    if (!this->timers.contains(timer))
        return false;
    delete this->timers[timer];
    this->timerFunctions.remove(this->timers[timer]);
    this->timers.remove(timer);
    return true;
}

bool HuggleJS::start_timer(unsigned int timer, int interval)
{
    if (!this->timers.contains(timer))
        return false;
    this->timers[timer]->start(interval);

    return true;
}

bool HuggleJS::stop_timer(unsigned int timer)
{
    if (!this->timers.contains(timer))
        return false;

    this->timers[timer]->stop();
    return true;
}

void HuggleJS::OnTime()
{
    QTimer *timer = (QTimer*) QObject::sender();
    if (!this->timerFunctions.contains(timer))
        return;
    this->GetScript()->ExecuteFunction(this->timerFunctions[timer], QJSValueList());
}
