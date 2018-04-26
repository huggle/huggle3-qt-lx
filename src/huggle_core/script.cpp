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
QList<Script*> Script::scripts;

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
    return this->scripts.values();
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
    return "huggle3";
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
    source = Resources::GetResource("huggle/ecma/huggle_core.js") + source;

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

    if (this->extensions.contains(this->scriptName))
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
    return this->executeFunction(function, QScriptValueList()).toBool();
}

bool Script::executeFunctionAsBool(QString function)
{

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
    return this->executeFunction(function, QScriptValueList());
}

QScriptValue Script::executeFunction(QString function)
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

void Script::registerFunction(QString name, QScriptEngine::FunctionSignature function_signature, int parameters, QString help, bool is_unsafe)
{

}

void Script::registerHook(QString name, int parameters, QString help, bool is_unsafe)
{

}

void Script::registerFunctions()
{

}

ScriptException::ScriptException(QString text, QString source, Script *scr, bool is_recoverable) : Exception(text, source, is_recoverable)
{

}
