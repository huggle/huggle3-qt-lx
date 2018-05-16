//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "script.hpp"
#include "genericjsclass.hpp"
#include "hugglejs.hpp"
#include "../configuration.hpp"
#include "../localization.hpp"
#include "../resources.hpp"
#include "../generic.hpp"
#include "../syslog.hpp"
#include <QFile>
#include <QTimer>

using namespace Huggle;

QList<QString> Script::loadedPaths;
QHash<QString, Script*> Script::scripts;

Script *Script::GetScriptByPath(QString path)
{
    foreach (Script *s, Script::scripts)
    {
        if (s->scriptPath == path)
            return s;
    }

    return nullptr;
}

Script *Script::GetScriptByEngine(QJSEngine *e)
{
    foreach (Script *s, Script::scripts)
    {
        if (s->engine == e)
            return s;
    }

    return nullptr;
}

Script *Script::GetScriptByName(QString name)
{
    foreach (Script *s, Script::scripts)
    {
        if (s->scriptName == name)
            return s;
    }

    return nullptr;
}

QList<Script *> Script::GetScripts()
{
    return Script::scripts.values();
}

Script::Script()
{
    this->isUnsafe = true;
    this->isWorking = false;
    this->isLoaded = false;
    this->engine = nullptr;
}

Script::~Script()
{
    delete this->engine;
    if (!this->scriptPath.isEmpty())
        Script::loadedPaths.removeAll(this->scriptPath);
    if (this->isLoaded && Script::scripts.contains(this->GetName()))
        Script::scripts.remove(this->scriptName);
}

bool Script::Load(QString path, QString *error)
{
    if (this->engine || this->isLoaded)
    {
        *error = "You can't run Load multiple times";
        return false;
    }
    if (Script::loadedPaths.contains(path))
    {
        *error = "This script is already loaded";
        return false;
    }

    this->scriptPath = path;

    // Try to read the script
    QFile file(path);
    if (!file.open(QFile::ReadOnly))
    {
        *error = "Unable to read file: " + path;
        return false;
    }
    QTextStream stream(&file);
    QString sx = stream.readAll();
    file.close();
    return this->loadSource(sx, error);
}

bool Script::LoadSrc(QString unique_id, QString source, QString *error)
{
    if (this->engine || this->isLoaded)
    {
        *error = "You can't run Load multiple times";
        return false;
    }
    if (Script::loadedPaths.contains(unique_id))
    {
        *error = "This script is already loaded";
        return false;
    }

    this->scriptPath = unique_id;
    return this->loadSource(source, error);
}

void Script::Unload()
{
    if (this->IsWorking())
        this->executeFunction("ext_unload");
    this->isWorking = false;
}

QString Script::GetDescription()
{
    return this->scriptDesc;
}

QString Script::GetName()
{
    return this->scriptName;
}

QString Script::GetVersion()
{
    return this->scriptVers;
}

QString Script::GetPath()
{
    return this->scriptPath;
}

QString Script::GetAuthor()
{
    return this->scriptAuthor;
}

bool Script::IsWorking()
{
    if (!this->isWorking || !this->isLoaded)
        return false;

    return this->executeFunctionAsBool("ext_is_working");
}

QJSValue Script::ExecuteFunction(QString function, QJSValueList parameters)
{
    return this->executeFunction(function, parameters);
}

unsigned int Script::GetContextID()
{
    return 0;
}

QString Script::GetContext()
{
    return "huggle_core";
}

bool Script::IsUnsafe()
{
    return this->isUnsafe;
}

bool Script::SupportFunction(QString name)
{
    return this->functionsExported.contains(name);
}

QString Script::GetHelpForFunc(QString name)
{
    if (!this->functionsHelp.contains(name))
        return QString();
    return this->functionsHelp[name];
}

QList<QString> Script::GetHooks()
{
    return this->hooksExported;
}

QList<QString> Script::GetFunctions()
{
    return this->functionsExported;
}

QJSEngine *Script::GetEngine()
{
    return this->engine;
}

void Script::Hook_Shutdown()
{
    if (this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_SHUTDOWN))
        return;
    this->executeFunction(this->attachedHooks[HUGGLE_SCRIPT_HOOK_SHUTDOWN]);
}

void Script::SubscribeHook(int hook, QString function_name)
{
    if (this->attachedHooks.contains(hook))
        this->attachedHooks[hook] = function_name;
    else
        this->attachedHooks.insert(hook, function_name);
}

void Script::UnsubscribeHook(int hook)
{
    if (this->attachedHooks.contains(hook))
        this->attachedHooks.remove(hook);
}

bool Script::HookSubscribed(int hook)
{
    return this->attachedHooks.contains(hook);
}

int Script::GetHookID(QString hook)
{
    // Resolve hook ID from string ID
    // If doesn't exist, return -1
    if (hook == "shutdown")
        return HUGGLE_SCRIPT_HOOK_SHUTDOWN;
    return -1;
}

bool Script::loadSource(QString source, QString *error)
{
    // Prepend the built-in libs
    source = Resources::GetResource("/huggle/text/ecma/help.js") + source;
    source = Resources::GetResource("/huggle/text/ecma/huggle.js") + source;
    source = Resources::GetResource("/huggle/text/ecma/types.js") + source;

    this->sourceCode = source;
    this->engine = new QJSEngine();

    this->script_ptr = this->engine->evaluate(this->sourceCode);
    if (this->script_ptr.isError())
    {
        *error = "Unable to load script, syntax error at line " + QString::number(this->script_ptr.property("lineNumber").toInt()) + " column " +
                 QString::number(this->script_ptr.property("columnNumber").toInt()) + ": " + this->script_ptr.toString();
        this->isWorking = false;
        return false;
    }

    this->isLoaded = true;
    this->registerFunctions();
    this->registerClasses();
    this->isWorking = true;

    if (!this->IsWorking())
    {
        *error = "Unable to load script, ext_is_working() didn't return true";
        this->isWorking = false;
        return false;
    }

    this->scriptAuthor = this->executeFunctionAsString("ext_get_author");
    this->scriptDesc = this->executeFunctionAsString("ext_get_desc");
    this->scriptName = this->executeFunctionAsString("ext_get_name");
    this->scriptVers = this->executeFunctionAsString("ext_get_version");

    if (this->scriptName.isEmpty())
    {
        *error = "Unable to load script, ext_get_name returned nothing";
        this->isWorking = false;
        return false;
    }

    this->scriptName = this->scriptName.toLower();

    if (this->scripts.contains(this->scriptName))
    {
        *error = this->scriptName + " is already loaded";
        this->isWorking = false;
        return false;
    }

    // Loading is done, let's assume everything works
    Script::loadedPaths.append(this->scriptPath);
    Script::scripts.insert(this->GetName(), this);
    if (!this->executeFunctionAsBool("ext_init"))
    {
        *error = "Unable to load script, ext_init() didn't return true";
        this->isWorking = false;
        return false;
    }
    return true;
}

bool Script::executeFunctionAsBool(QString function, QJSValueList parameters)
{
    return this->executeFunction(function, parameters).toBool();
}

bool Script::executeFunctionAsBool(QString function)
{
    return this->executeFunctionAsBool(function, QJSValueList());
}

QString Script::executeFunctionAsString(QString function)
{
    return this->executeFunction(function, QJSValueList()).toString();
}

QString Script::executeFunctionAsString(QString function, QJSValueList parameters)
{
    return this->executeFunction(function, parameters).toString();
}

QJSValue Script::executeFunction(QString function, QJSValueList parameters)
{
    if (!this->isLoaded)
        throw new ScriptException("Call to script function of extension that isn't loaded", BOOST_CURRENT_FUNCTION, this);

    QJSValue fc = this->engine->globalObject().property(function);
    // If function doesn't exist, undefined value is returned. For most of hooks this is normal, since extensions don't use all of them,
    // so this issue shouldn't be logged anywhere here. Let's just pass the invalid result for callee to handle it themselves.
    if (fc.isUndefined())
        return fc;
    if (!fc.isCallable())
    {
        HUGGLE_ERROR("JS error (" + this->GetName() + "): " + function + " is not a function");
        return fc;
    }
    QJSValue result = fc.call(parameters);
    if (result.isError())
    {
        // There was some error during execution
        qint32 line = result.property("lineNumber").toInt();
        qint32 col = result.property("columnNumber").toInt();
        HUGGLE_ERROR("JS error, line " + QString::number(line) + " column " + QString::number(col) + " (" + this->GetName() + "): " + result.toString());
    }
    return result;
}

QJSValue Script::executeFunction(QString function)
{
    return this->executeFunction(function, QJSValueList());
}

void Script::registerClass(QString name, GenericJSClass *c)
{
    this->engine->globalObject().setProperty(name, engine->newQObject(c));
}

void Script::registerClasses()
{
    this->registerClass("huggle", new HuggleJS(this));
}

void Script::registerFunction(QString name, QString help, bool is_unsafe)
{
    // Check if this script is allowed to access unsafe functions
    if (is_unsafe && !this->isUnsafe)
        return;

    this->functionsExported.append(name);
    this->functionsHelp.insert(name, help);
}

void Script::registerHook(QString name, int parameters, QString help, bool is_unsafe)
{
    this->hooksExported.append(name);
    this->functionsHelp.insert(name, help);
}

void Script::registerFunctions()
{
    this->registerFunction("huggle.get_function_help", "(string function_name): give you help for a function, returns string");
    this->registerFunction("huggle.get_function_list", "(): returns array with list of functions");
    this->registerFunction("huggle.get_hook_list", "(): returns a list of all hooks");
    this->registerFunction("huggle.get_version", "(): returns version object with properties: Major, Minor, Revision, String");
    this->registerFunction("huggle.is_unsafe", "(): returns true if script has access to unsafe functions");
    this->registerFunction("huggle.set_cfg", "(string key, string value): stores value as key in settings");
    this->registerFunction("huggle.get_cfg", "(string key, string default): returns stored value from ini file");
    this->registerFunction("huggle.has_function", "(string function_name): return true or false whether function is present");
    this->registerFunction("huggle.get_context", "(): return execution context, either core or GrumpyChat (core doesn't have ui functions and hooks)");
    this->registerFunction("huggle.debug_log", "(string text, int verbosity): prints to debug log");
    this->registerFunction("huggle.error_log", "(string text): prints to system error log");
    this->registerFunction("huggle.warning_log", "(string text): prints to warning log");
    this->registerFunction("huggle.log", "(string text): prints to log");

    this->registerHook("ext_init", 0, "(): called on start, must return true, otherwise load of extension is considered as failure");
    this->registerHook("ext_get_name", 0, "(): should return a name of this extension");
    this->registerHook("ext_get_desc", 0, "(): should return description");
    this->registerHook("ext_get_author", 0, "(): should contain name of creator");
    this->registerHook("ext_desc_version", 0, "(): should return version");
    this->registerHook("ext_unload", 0, "(): called when extension is being unloaded from system");
    this->registerHook("ext_is_working", 0, "(): must exist and must return true, if returns false, extension is considered crashed");
    this->registerHook("shutdown", 0, "(): called on exit of Huggle");
}

ScriptException::ScriptException(QString text, QString source, Script *scr, bool is_recoverable) : Exception(text, source, is_recoverable)
{
    this->s = scr;
}
