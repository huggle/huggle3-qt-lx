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
#include "../core.hpp"
#include "../generic.hpp"
#include "../syslog.hpp"
#include "../localization.hpp"
#include "../wikisite.hpp"
#include "../resources.hpp"
#include <QJSValueIterator>
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
    this->functions.insert("get_startup_date_time", "(): returns time of Huggle startup");
    this->functions.insert("stop_timer", "(uint timer");
    this->functions.insert("get_sites", "(): return string list of sites this user is logged to");
    this->functions.insert("get_site_by_name", "(string site_name): returns a site info for site with this name");
    this->functions.insert("localize", "(string key): return localized key");
    this->functions.insert("dump_obj", "(object): dumps an object to string");
    this->functions.insert("play_file", "(string file): play internal file");
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

QString HuggleJS::get_username()
{
    return hcfg->SystemConfig_Username;
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

QJSValue HuggleJS::get_site_by_name(QString site)
{
    foreach (WikiSite *wiki, hcfg->Projects)
    {
        if (wiki->Name == site)
            return JSMarshallingHelper::FromSite(wiki, this->script->GetEngine());
    }
    return QJSValue(QJSValue::SpecialValue::NullValue);
}

QList<QString> HuggleJS::get_sites()
{
    // List of sites
    QList<QString> sl;
    foreach (WikiSite *wiki, hcfg->Projects)
        sl.append(wiki->Name);

    return sl;
}

QString HuggleJS::localize(QString id)
{
    return _l(id);
}

QDateTime HuggleJS::get_startup_date_time()
{
    return Core::HuggleCore->StartupTime;
}

void HuggleJS::play_file(QString name)
{
    Resources::PlayEmbeddedSoundFile(name);
}

QString HuggleJS::dump_obj(QJSValue object, unsigned int indent)
{
    QString indent_prefix = "";
    unsigned int pref = indent;
    while (pref-- > 0)
        indent_prefix += " ";
    QString object_desc = indent_prefix;
    if (object.isArray())
    {
        object_desc += "array [ \n";
        int length = object.property("length").toInt();
        int i = 0;
        while (i < length)
        {
            object_desc += "    " + indent_prefix + dump_obj(object.property(i++), indent + 4) + "\n";
        }
        object_desc += indent_prefix + " ]";
    } else if (object.isBool())
    {
        object_desc += "bool (" + Generic::Bool2String(object.toBool()) + ")";
    } else if (object.isCallable())
    {
        object_desc += "callable()";
    } else if (object.isDate())
    {
        object_desc += "datetime (" + object.toDateTime().toString() + ")";
    } else if (object.isError())
    {
        object_desc += "error type";
    } else if (object.isNull())
    {
        object_desc += "null type";
    } else if (object.isNumber())
    {
        object_desc += "int (" + QString::number(object.toInt()) + ")";
    } else if (object.isUndefined())
    {
        object_desc += "undefined type";
    } else if (object.isVariant())
    {
        object_desc += "variant";
    } else if (object.isString())
    {
        object_desc += "string (" + object.toString() + ")";
    } else if (object.isQObject())
    {
        object_desc += "qobject";
    } else if (object.isObject())
    {
        object_desc += "object { \n";
        QJSValueIterator it(object);
        while (it.hasNext())
        {
            it.next();
            object_desc += "    " + indent_prefix + it.name() + ": " + dump_obj(it.value(), indent + 4) + "\n";
        }
        object_desc += indent_prefix + " }";
    } else
    {
        object_desc += "unknown type";
    }

    return object_desc;
}

void HuggleJS::OnTime()
{
    QTimer *timer = (QTimer*) QObject::sender();
    if (!this->timerFunctions.contains(timer))
        return;
    this->GetScript()->ExecuteFunction(this->timerFunctions[timer], QJSValueList());
}

WikiSite *HuggleJS::getSiteByName(QString name)
{
    foreach (WikiSite *site, hcfg->Projects)
    {
        if (site->Name == name)
            return site;
    }
    return nullptr;
}
