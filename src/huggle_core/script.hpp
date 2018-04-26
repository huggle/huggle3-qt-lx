//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// This code is larger taken over from GrumpyChat project

// Copyright (c) Petr Bena 2018

#ifndef SCRIPT_HPP
#define SCRIPT_HPP

#include "definitions.hpp"
#include "exception.hpp"
#include <QtScript>

namespace Huggle
{
    class Script;
    class ScriptException : public Exception
    {
        public:
            ScriptException(QString text, QString source, Script *scr, bool is_recoverable = true);
        private:
            Script *s;
    };

    class Script : public QObject
    {
            Q_OBJECT
        public:
            static Script *GetScriptByPath(QString path);
            static Script *GetScriptByEngine(QScriptEngine *e);
            static Script *GetScriptByName(QString name);
            static QList<Script*> GetScripts();

            Script();
            virtual ~Script();
            virtual bool Load(QString path, QString *error);
            virtual bool LoadSrc(QString unique_id, QString source, QString *error);
            virtual void Unload();
            QString GetDescription();
            QString GetName();
            QString GetVersion();
            QString GetPath();
            QString GetAuthor();
            bool IsWorking();
            QScriptValue ExecuteFunction(QString function, QScriptValueList parameters);
            virtual unsigned int GetContextID();
            virtual QString GetContext();
            bool IsUnsafe();
            bool SupportFunction(QString name);
            QString GetHelpForFunc(QString name);
            QList<QString> GetHooks();
            QList<QString> GetFunctions();
            void Hook_Shutdown();
        private slots:
            void OnError(QScriptValue e);
        protected:
            static QList<QString> loadedPaths;
            static QHash<QString, Script*> scripts;
            bool loadSource(QString source, QString *error);
            bool executeFunctionAsBool(QString function, QScriptValueList parameters);
            bool executeFunctionAsBool(QString function);
            QString executeFunctionAsString(QString function);
            QString executeFunctionAsString(QString function, QScriptValueList parameters);
            QScriptValue executeFunction(QString function, QScriptValueList parameters);
            QScriptValue executeFunction(QString function);
            virtual void registerFunction(QString name, QScriptEngine::FunctionSignature function_signature, int parameters, QString help = "", bool is_unsafe = false);
            virtual void registerHook(QString name, int parameters, QString help = "", bool is_unsafe = false);
            //! Makes all functions available to ECMA
            virtual void registerFunctions();
            QScriptEngine *engine;
            QScriptValue script_ptr;
            QList<QString> hooksExported;
            QList<QString> functionsExported;
            QHash<QString, QString> functionsHelp;
            QString sourceCode;
            QString scriptPath;
            QString scriptName;
            QString scriptDesc;
            QString scriptAuthor;
            QString scriptVers;
            bool isWorking;
            bool isLoaded;
            bool isUnsafe;
    };
}

#endif // SCRIPT_HPP
