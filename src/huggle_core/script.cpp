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
#include "configuration.hpp"
#include "localization.hpp"
#include "resources.hpp"
#include "generic.hpp"
#include "syslog.hpp"
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

Script *Script::GetScriptByEngine(QScriptEngine *e)
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

QScriptValue Script::ExecuteFunction(QString function, QScriptValueList parameters)
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

void Script::Hook_Shutdown()
{
    this->executeFunction("ext_hook_on_shutdown");
}

void Script::OnError(QScriptValue e)
{
    HUGGLE_ERROR("Script error (" + this->scriptName + "): " + e.toString());
}

bool Script::loadSource(QString source, QString *error)
{
    QScriptEngine syntax_check;
    QScriptSyntaxCheckResult s = syntax_check.checkSyntax(source);
    if (s.state() != QScriptSyntaxCheckResult::Valid)
    {
        *error = "Unable to load script, syntax error at line " + QString::number(s.errorLineNumber()) + " column " + QString::number(s.errorColumnNumber()) + ": " + s.errorMessage();
        this->isWorking = false;
        return false;
    }

    // Prepend the built-in libs
    source = Resources::GetResource("/huggle/text/Resources/ecma/help.js") + source;
    source = Resources::GetResource("/huggle/text/Resources/ecma/huggle.js") + source;
    source = Resources::GetResource("/huggle/text/Resources/ecma/types.js") + source;

    this->sourceCode = source;
    this->engine = new QScriptEngine();
    connect(this->engine, SIGNAL(signalHandlerException(QScriptValue)), this, SLOT(OnError(QScriptValue)));

    this->script_ptr = this->engine->evaluate(this->sourceCode);
    this->isLoaded = true;
    this->registerFunctions();
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

bool Script::executeFunctionAsBool(QString function, QScriptValueList parameters)
{
    return this->executeFunction(function, parameters).toBool();
}

bool Script::executeFunctionAsBool(QString function)
{
    return this->executeFunctionAsBool(function, QScriptValueList());
}

QString Script::executeFunctionAsString(QString function)
{
    return this->executeFunction(function, QScriptValueList()).toString();
}

QString Script::executeFunctionAsString(QString function, QScriptValueList parameters)
{
    return this->executeFunction(function, parameters).toString();
}

QScriptValue Script::executeFunction(QString function, QScriptValueList parameters)
{
    if (!this->isLoaded)
        throw new ScriptException("Call to script function of extension that isn't loaded", BOOST_CURRENT_FUNCTION, this);

    QScriptValue fc = this->engine->globalObject().property(function);
    // If function doesn't exist, invalid value is returned. For most of hooks this is normal, since extensions don't use all of them,
    // so this issue shouldn't be logged anywhere here. Let's just pass the invalid result for callee to handle it themselves.
    if (!fc.isValid())
        return fc;
    if (!fc.isFunction())
    {
        HUGGLE_ERROR("JS error (" + this->GetName() + "): " + function + " is not a function");
        return fc;
    }
    QScriptValue result = fc.call(QScriptValue(), parameters);
    if (result.isError())
    {
        // There was some error during execution
        qint32 line = result.property("lineNumber").toInt32();
        HUGGLE_ERROR("JS error, line " + QString::number(line) + " (" + this->GetName() + "): " + result.toString());
    }
    return result;
}

QScriptValue Script::executeFunction(QString function)
{
    return this->executeFunction(function, QScriptValueList());
}

void Script::registerFunction(QString name, QScriptEngine::FunctionSignature function_signature, int parameters, QString help, bool is_unsafe)
{
    // Check if this script is allowed to access unsafe functions
    if (is_unsafe && !this->isUnsafe)
        return;

    this->functionsExported.append(name);
    this->functionsHelp.insert(name, help);
    this->engine->globalObject().setProperty(name, this->engine->newFunction(function_signature, parameters));
}

void Script::registerHook(QString name, int parameters, QString help, bool is_unsafe)
{
    this->hooksExported.append(name);
    this->functionsHelp.insert(name, help);
}

///////////////////////////////////////////////////////////////////////////////////////////
/// Function definitions
///////////////////////////////////////////////////////////////////////////////////////////

static QScriptValue get_hook_list(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);
    Script *extension = Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);

    return qScriptValueFromSequence(engine, extension->GetHooks());
}

static QScriptValue is_unsafe(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);
    Script *extension = Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);

    return QScriptValue(engine, extension->IsUnsafe());
}

static QScriptValue get_cfg(QScriptContext *context, QScriptEngine *engine)
{
    Script *extension = Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        HUGGLE_ERROR(extension->GetName() + ": get_cfg(key): requires 1 parameter");
        return QScriptValue(engine, false);
    }
    QScriptValue default_value;
    if (context->argumentCount() > 1)
        default_value = context->argument(1);
    QString key = context->argument(0).toString();
    QString prefix = "script_" + extension->GetName();

    if (!hcfg->ExtensionConfigContainsKey(prefix, key))
        return default_value;

    QString value = hcfg->GetExtensionConfig(prefix, key, default_value.toString());
    QScriptValue result = QScriptValue(engine, value);
    return result;
}

static QScriptValue set_cfg(QScriptContext *context, QScriptEngine *engine)
{
    Script *extension = Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);

    if (context->argumentCount() < 2)
    {
        HUGGLE_ERROR(extension->GetName() + ": set_cfg(key, value): requires 2 parameters");
        return QScriptValue(engine, false);
    }
    QString key = context->argument(0).toString();
    QVariant value = context->argument(1).toVariant();

    hcfg->SetExtensionConfig("script_" + extension->GetName(), key, value.toString());
    return QScriptValue();
}

static QScriptValue get_version(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);
    Script *extension = Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);

    int major = HUGGLE_BYTE_VERSION_MAJOR;
    int minor = HUGGLE_BYTE_VERSION_MINOR;
    int revision = HUGGLE_BYTE_VERSION_RELEASE;

    // Marshalling
    QScriptValue version = engine->newObject();
    version.setProperty("Major", QScriptValue(engine, major));
    version.setProperty("Minor", QScriptValue(engine, minor));
    version.setProperty("Revision", QScriptValue(engine, revision));
    version.setProperty("String", hcfg->HuggleVersion);
    return version;
}

static QScriptValue get_context(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);
    Script *extension = Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);

    return QScriptValue(engine, extension->GetContext());
}

static QScriptValue has_function(QScriptContext *context, QScriptEngine *engine)
{
    Script *extension = Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        HUGGLE_ERROR(extension->GetName() + ": has_function(function): requires 1 parameter");
        return QScriptValue(engine, false);
    }
    QString fx = context->argument(0).toString();
    return QScriptValue(engine, extension->SupportFunction(fx));
}

static QScriptValue get_function_help(QScriptContext *context, QScriptEngine *engine)
{
    Script *extension = Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        HUGGLE_ERROR(extension->GetName() + ": get_function_help(function): requires 1 parameter");
        return QScriptValue(engine, QString("Function not found"));
    }
    QString fx = context->argument(0).toString();
    return QScriptValue(engine, extension->GetHelpForFunc(fx));
}

static QScriptValue get_function_list(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);
    Script *extension = Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);

    return qScriptValueFromSequence(engine, extension->GetFunctions());
}

static QScriptValue error_log(QScriptContext *context, QScriptEngine *engine)
{
    Script *extension = Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        HUGGLE_ERROR(extension->GetName() + ": error_log(text): requires 1 parameter");
        return QScriptValue(engine, false);
    }
    QString text = context->argument(0).toString();
    HUGGLE_ERROR(extension->GetName() + ": " + text);
    return QScriptValue(engine, true);
}

static QScriptValue debug_log(QScriptContext *context, QScriptEngine *engine)
{
    Script *extension = Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 2)
    {
        // Wrong number of parameters
        HUGGLE_ERROR(extension->GetName() + ": debug_log(text, verbosity): requires 2 parameters");
        return QScriptValue(engine, false);
    }
    QString text = context->argument(0).toString();
    int verbosity = context->argument(1).toInt32();
    HUGGLE_DEBUG(extension->GetName() + ": " + text, verbosity);
    return QScriptValue(engine, true);
}

static QScriptValue warning_log(QScriptContext *context, QScriptEngine *engine)
{
    Script *extension = Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        HUGGLE_ERROR(extension->GetName() + ": warning_log(text): requires 1 parameter");
        return QScriptValue(engine, false);
    }
    QString text = context->argument(0).toString();
    HUGGLE_WARNING(extension->GetName() + ": " + text);
    return QScriptValue(engine, true);
}

static QScriptValue log(QScriptContext *context, QScriptEngine *engine)
{
    Script *extension = Script::GetScriptByEngine(engine);
    if (!extension)
        return QScriptValue(engine, false);
    if (context->argumentCount() < 1)
    {
        // Wrong number of parameters
        HUGGLE_ERROR(extension->GetName() + ": log(text): requires 1 parameter");
        return QScriptValue(engine, false);
    }
    QString text = context->argument(0).toString();
    HUGGLE_LOG(text);
    return QScriptValue(engine, true);
}

void Script::registerFunctions()
{
    this->registerFunction("huggle_get_function_help", get_function_help, 1, "(string function_name): give you help for a function, returns string");
    this->registerFunction("huggle_get_function_list", get_function_list, 0, "(): returns array with list of functions");
    this->registerFunction("huggle_get_hook_list", get_hook_list, 0, "(): returns a list of all hooks");
    this->registerFunction("huggle_get_version", get_version, 0, "(): returns version object with properties: Major, Minor, Revision, String");
    this->registerFunction("huggle_is_unsafe", is_unsafe, 0, "(): returns true if script has access to unsafe functions");
    this->registerFunction("huggle_set_cfg", set_cfg, 2, "(string key, string value): stores value as key in settings");
    this->registerFunction("huggle_get_cfg", get_cfg, 2, "(string key, string default): returns stored value from ini file");
    this->registerFunction("huggle_has_function", has_function, 1, "(string function_name): return true or false whether function is present");
    this->registerFunction("huggle_get_context", get_context, 0, "(): return execution context, either core or GrumpyChat (core doesn't have ui functions and hooks)");
    this->registerFunction("huggle_debug_log", debug_log, 2, "(string text, int verbosity): prints to debug log");
    this->registerFunction("huggle_error_log", error_log, 1, "(string text): prints to system error log");
    this->registerFunction("huggle_warning_log", warning_log, 1, "(string text): prints to warning log");
    this->registerFunction("huggle_log", log, 1, "(string text): prints to log");

    this->registerHook("ext_init", 0, "(): called on start, must return true, otherwise load of extension is considered as failure");
    this->registerHook("ext_on_shutdown", 0, "(): called on exit");
    this->registerHook("ext_get_name", 0, "(): should return a name of this extension");
    this->registerHook("ext_get_desc", 0, "(): should return description");
    this->registerHook("ext_get_author", 0, "(): should contain name of creator");
    this->registerHook("ext_desc_version", 0, "(): should return version");
    this->registerHook("ext_unload", 0, "(): called when extension is being unloaded from system");
    this->registerHook("ext_is_working", 0, "(): must exist and must return true, if returns false, extension is considered crashed");
}

ScriptException::ScriptException(QString text, QString source, Script *scr, bool is_recoverable) : Exception(text, source, is_recoverable)
{
    this->s = scr;
}
