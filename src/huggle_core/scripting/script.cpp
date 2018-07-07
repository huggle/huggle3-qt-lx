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
#include "jsmarshallinghelper.hpp"
#include "huggleunsafejs.hpp"
#include "hugglequeryjs.hpp"
#include "hugglejs.hpp"
#include "huggleeditingjs.hpp"
#include "../configuration.hpp"
#include "../localization.hpp"
#include "../resources.hpp"
#include "../generic.hpp"
#include "../syslog.hpp"
#include "../version.hpp"
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

QJSValue Script::ProcessURL(QUrl url)
{
    if (url.scheme() != "hgjs")
        return QJSValue::NullValue;

    Script *script = GetScriptByName(url.host());
    if (!script)
        return QJSValue::UndefinedValue;
    QString function_name;
    QJSValueList parameters;
    function_name = url.path().replace("/", "");
    if (url.hasQuery())
    {
        foreach (QString parameter, url.query().split("&"))
        {
            parameters.append(QJSValue(parameter));
        }
    }
    return script->ExternalCallback(function_name, parameters);
}

Script::Script()
{
    this->isUnsafe = hcfg->SystemConfig_UnsafeExts;
    this->isWorking = false;
    this->isLoaded = false;
    this->engine = nullptr;
}

Script::~Script()
{
    if (!this->scriptPath.isEmpty())
        Script::loadedPaths.removeAll(this->scriptPath);
    if (this->isLoaded && Script::scripts.contains(this->GetName()))
        Script::scripts.remove(this->scriptName);
    qDeleteAll(this->classes);
    delete this->engine;
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

QJSValue Script::ExecuteFunction(QString function)
{
    return this->executeFunction(function);
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

QJSValue Script::ExternalCallback(QString callback, QJSValueList parameters)
{
    if (!this->externalCallbacks.contains(callback))
        return QJSValue::NullValue;
    return this->executeFunction(callback, parameters);
}

void Script::RegisterExternalCallback(QString callback)
{
    this->externalCallbacks.append(callback);
}

void Script::UnregisterExternalCallback(QString callback)
{
    this->externalCallbacks.removeAll(callback);
}

bool Script::HasExternalCallback(QString callback)
{
    return this->externalCallbacks.contains(callback);
}

void Script::Hook_Shutdown()
{
    if (!this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_SHUTDOWN))
        return;
    this->executeFunction(this->attachedHooks[HUGGLE_SCRIPT_HOOK_SHUTDOWN]);
}

void Script::Hook_EditPreProcess(WikiEdit *edit)
{
    if (!this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_EDIT_PRE_PROCESS))
        return;

    QJSValueList parameters;
    parameters.append(JSMarshallingHelper::FromEdit(edit, this->engine));
    this->executeFunction(this->attachedHooks[HUGGLE_SCRIPT_HOOK_EDIT_PRE_PROCESS], parameters);
}

void Script::Hook_EditBeforePostProcess(WikiEdit *edit)
{
    if (!this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_EDIT_BEFORE_POST_PROCESS))
        return;

    QJSValueList parameters;
    parameters.append(JSMarshallingHelper::FromEdit(edit, this->engine));
    this->executeFunction(this->attachedHooks[HUGGLE_SCRIPT_HOOK_EDIT_BEFORE_POST_PROCESS], parameters);
}

void Script::Hook_EditPostProcess(WikiEdit *edit)
{
    if (!this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_EDIT_POST_PROCESS))
        return;

    QJSValueList parameters;
    parameters.append(JSMarshallingHelper::FromEdit(edit, this->engine));
    this->executeFunction(this->attachedHooks[HUGGLE_SCRIPT_HOOK_EDIT_POST_PROCESS], parameters);
}

bool Script::Hook_EditLoadToQueue(WikiEdit *edit)
{
    if (!this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_EDIT_LOAD_TO_QUEUE))
        return true;
    QJSValueList parameters;
    parameters.append(JSMarshallingHelper::FromEdit(edit, this->engine));
    return this->executeFunctionAsBool(this->attachedHooks[HUGGLE_SCRIPT_HOOK_EDIT_LOAD_TO_QUEUE], parameters);
}

void Script::Hook_FeedProvidersOnInit(WikiSite *site)
{
    if (!this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_FEED_PROVIDERS_ON_INIT))
        return;
    QJSValueList parameters;
    parameters.append(JSMarshallingHelper::FromSite(site, this->engine));
    this->executeFunctionAsBool(this->attachedHooks[HUGGLE_SCRIPT_HOOK_FEED_PROVIDERS_ON_INIT], parameters);
}

void Script::Hook_OnRevert(WikiEdit *edit)
{
    if (!this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_EDIT_ON_REVERT))
        return;

    QJSValueList parameters;
    parameters.append(JSMarshallingHelper::FromEdit(edit, this->engine));
    this->executeFunction(this->attachedHooks[HUGGLE_SCRIPT_HOOK_EDIT_ON_REVERT], parameters);
}

void Script::Hook_OnGood(WikiEdit *edit)
{
    if (!this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_EDIT_ON_GOOD))
        return;

    QJSValueList parameters;
    parameters.append(JSMarshallingHelper::FromEdit(edit, this->engine));
    this->executeFunction(this->attachedHooks[HUGGLE_SCRIPT_HOOK_EDIT_ON_GOOD], parameters);
}

void Script::Hook_OnSuspicious(WikiEdit *edit)
{
    if (!this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_EDIT_ON_SUSPICIOUS))
        return;

    QJSValueList parameters;
    parameters.append(JSMarshallingHelper::FromEdit(edit, this->engine));
    this->executeFunction(this->attachedHooks[HUGGLE_SCRIPT_HOOK_EDIT_ON_SUSPICIOUS], parameters);
}

bool Script::Hook_OnRevertPreflight(WikiEdit *edit)
{
    if (!this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_REVERT_PREFLIGHT))
        return true;
    QJSValueList parameters;
    parameters.append(JSMarshallingHelper::FromEdit(edit, this->engine));
    return this->executeFunctionAsBool(this->attachedHooks[HUGGLE_SCRIPT_HOOK_REVERT_PREFLIGHT], parameters);
}

int Script::Hook_EditRescore(WikiEdit *edit)
{
    if (!this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_EDIT_RESCORE))
        return 0;

    QJSValueList parameters;
    parameters.append(JSMarshallingHelper::FromEdit(edit, this->engine));
    QJSValue result = this->executeFunction(this->attachedHooks[HUGGLE_SCRIPT_HOOK_EDIT_RESCORE], parameters);
    if (!result.isNumber())
    {
        HUGGLE_ERROR("JS error (" + this->GetName() + "): edit_rescore must return a number");
        return 0;
    }
    return result.toInt();
}

void Script::Hook_WarningFinished(WikiEdit *edit)
{
    if (!this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_WARNING_FINISHED))
        return;

    QJSValueList parameters;
    parameters.append(JSMarshallingHelper::FromEdit(edit, this->engine));
    this->executeFunction(this->attachedHooks[HUGGLE_SCRIPT_HOOK_WARNING_FINISHED], parameters);
}

void Script::Hook_OnLocalConfigRead()
{
    if (!this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_LOCALCONFIG_READ))
        return;

    this->executeFunction(this->attachedHooks[HUGGLE_SCRIPT_HOOK_LOCALCONFIG_READ]);
}

void Script::Hook_OnLocalConfigWrite()
{
    if (!this->attachedHooks.contains(HUGGLE_SCRIPT_HOOK_LOCALCONFIG_WRITE))
        return;

    this->executeFunction(this->attachedHooks[HUGGLE_SCRIPT_HOOK_LOCALCONFIG_WRITE]);
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
    if (hook == "edit_after_pre_process")
        return HUGGLE_SCRIPT_HOOK_EDIT_PRE_PROCESS;
    if (hook == "edit_before_post_process")
        return HUGGLE_SCRIPT_HOOK_EDIT_BEFORE_POST_PROCESS;
    if (hook == "edit_load_to_queue")
        return HUGGLE_SCRIPT_HOOK_EDIT_LOAD_TO_QUEUE;
    if (hook == "edit_rescore")
        return HUGGLE_SCRIPT_HOOK_EDIT_RESCORE;
    if (hook == "edit_post_process")
        return HUGGLE_SCRIPT_HOOK_EDIT_POST_PROCESS;
    if (hook == "edit_on_revert")
        return HUGGLE_SCRIPT_HOOK_EDIT_ON_REVERT;
    if (hook == "edit_on_suspicious")
        return HUGGLE_SCRIPT_HOOK_EDIT_ON_SUSPICIOUS;
    if (hook == "edit_on_good")
        return HUGGLE_SCRIPT_HOOK_EDIT_ON_GOOD;
    if (hook == "warning_finished")
        return HUGGLE_SCRIPT_HOOK_WARNING_FINISHED;
    if (hook == "revert_preflight")
        return HUGGLE_SCRIPT_HOOK_REVERT_PREFLIGHT;
    if (hook == "localconfig_write")
        return HUGGLE_SCRIPT_HOOK_LOCALCONFIG_WRITE;
    if (hook == "localconfig_read")
        return HUGGLE_SCRIPT_HOOK_LOCALCONFIG_READ;
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

    QJSValue info = this->executeFunction("ext_get_info");
    if (info.isUndefined() || info.isNull())
    {
        *error = "Unable to load script, ext_get_info() didn't return valid object";
        this->isWorking = false;
        return false;
    }

    if (!info.hasProperty("author") || !info.hasProperty("description") || !info.hasProperty("version") || !info.hasProperty("name"))
    {
        *error = "Unable to load script, ext_get_info() didn't contain some of these properties: name, author, description, version";
        this->isWorking = false;
        return false;
    }

    this->scriptAuthor = info.property("author").toString();
    this->scriptDesc = info.property("description").toString();
    this->scriptName = info.property("name").toString();
    this->scriptVers = info.property("version").toString();

    if (this->scriptName.isEmpty())
    {
        *error = "Unable to load script, invalid extension name (name must be a string)";
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

    if (info.hasProperty("min_huggle_version"))
    {
        Version huggle_version(HUGGLE_VERSION);
        Version min_version(info.property("min_huggle_version").toString());
        if (!min_version.IsValid())
        {
            *error = "Unable to load script, min_huggle_version is invalid version string";
            this->isWorking = false;
            return false;
        }
        if (huggle_version < min_version)
        {
            *error = "Unable to load script, this extension requires Huggle version " + min_version.ToString() + " or newer";
            this->isWorking = false;
            return false;
        }
    }

    if (info.hasProperty("requires_unsafe") && info.property("requires_unsafe").toBool() == true && !this->IsUnsafe())
    {
        *error = "Unable to load script, access to unsafe methods is required by this script";
        this->isWorking = false;
        return false;
    }

    if (info.hasProperty("required_context") && info.property("required_context").toString() != this->GetContext())
    {
        *error = "Unable to load script, this extension doesn't work in this context of execution";
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
    QHash<QString, QString> functions = c->GetFunctions();
    foreach (QString function, functions.keys())
    {
        this->functionsExported.append(name + "." + function);
        this->functionsHelp.insert(name + "." + function, functions[function]);
    }
    // Register this class for later removal
    this->classes.append(c);
    this->engine->globalObject().setProperty(name, engine->newQObject(c));
}

void Script::registerClasses()
{
    // Check if extension should have access to extra functions
    if (this->IsUnsafe())
    {
        this->registerClass("huggle_unsafe", new HuggleUnsafeJS(this));
    }
    this->registerClass("huggle", new HuggleJS(this));
    this->registerClass("huggle_editing", new HuggleEditingJS(this));
    this->registerClass("huggle_query", new HuggleQueryJS(this));
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
    this->registerHook("ext_init", 0, "(): called on start, must return true, otherwise load of extension is considered as failure");
    this->registerHook("ext_get_info", 0, "(): should return a dictionary with properties name, author, description, version (all strings)");
    this->registerHook("ext_unload", 0, "(): called when extension is being unloaded from system");
    this->registerHook("ext_is_working", 0, "(): must exist and must return true, if returns false, extension is considered crashed");
    this->registerHook("shutdown", 0, "(): called on exit of Huggle");
    this->registerHook("edit_after_pre_process", 1, "(WikiEdit edit): called when edit is pre processed");
    this->registerHook("edit_before_post_process", 1, "(WikiEdit edit): called when edit is post processed");
    this->registerHook("bool edit_load_to_queue", 1, "(WikiEdit edit): called when edit is loaded to queue, if returns false, edit will be removed");
    this->registerHook("edit_on_suspicious", 1, "(WikiEdit edit): when suspicious edit is spotted");
    this->registerHook("edit_on_good", 1, "(WikiEdit edit): on good edit");
    this->registerHook("edit_on_revert", 1, "(WikiEdit edit): edit reverted");
    this->registerHook("int edit_rescore", 1, "(WikiEdit edit): called after post processing the edit, number returned will be added to final score (3.4.2)");
    this->registerHook("warning_finished", 1, "(WikiEdit edit): called when warning to user is sent");
    this->registerHook("bool revert_preflight", 1, "(WikiEdit edit): run before edit is reverted, if false is returned, revert is stopped");
}

ScriptException::ScriptException(QString text, QString source, Script *scr, bool is_recoverable) : Exception(text, source, is_recoverable)
{
    this->s = scr;
}
