//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// This code is mostly taken from GrumpyChat project

// Copyright (c) Petr Bena 2018

#ifndef SCRIPT_HPP
#define SCRIPT_HPP

#include "../definitions.hpp"
#include "../exception.hpp"
#include <QJSEngine>

// This is here for performance reasons only, we could have a list of attached hooks as list of strings
// but strings usually perform way worse than hash of integers, so we just use integers instead
// If you add anything here don't forget to edit the GetHookID functions

// If you are working in different context than core, you can define your own IDs, but each context is prefixed
// with some number so that you never get a conflicting number
#define HUGGLE_SCRIPT_HOOK_SHUTDOWN                             0
#define HUGGLE_SCRIPT_HOOK_EDIT_PRE_PROCESS                     1
#define HUGGLE_SCRIPT_HOOK_EDIT_BEFORE_POST_PROCESS             2
#define HUGGLE_SCRIPT_HOOK_EDIT_POST_PROCESS                    3
#define HUGGLE_SCRIPT_HOOK_EDIT_LOAD_TO_QUEUE                   4
#define HUGGLE_SCRIPT_HOOK_FEED_PROVIDERS_ON_INIT               5
#define HUGGLE_SCRIPT_HOOK_EDIT_ON_REVERT                       6
#define HUGGLE_SCRIPT_HOOK_EDIT_ON_GOOD                         7
#define HUGGLE_SCRIPT_HOOK_EDIT_ON_SUSPICIOUS                   8
#define HUGGLE_SCRIPT_HOOK_EDIT_RESCORE                         9
#define HUGGLE_SCRIPT_HOOK_WARNING_FINISHED                     10

namespace Huggle
{
    class GenericJSClass;
    class Script;
    class WikiEdit;
    class WikiSite;
    class WikiUser;
    class WikiPage;
    class HUGGLE_EX_CORE ScriptException : public Exception
    {
        public:
            ScriptException(QString text, QString source, Script *scr, bool is_recoverable = true);
        private:
            Script *s;
    };

    class HUGGLE_EX_CORE Script : public QObject
    {
            Q_OBJECT
        public:
            static Script *GetScriptByPath(QString path);
            static Script *GetScriptByEngine(QJSEngine *e);
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
            QJSValue ExecuteFunction(QString function);
            QJSValue ExecuteFunction(QString function, QJSValueList parameters);
            virtual unsigned int GetContextID();
            virtual QString GetContext();
            bool IsUnsafe();
            bool SupportFunction(QString name);
            QString GetHelpForFunc(QString name);
            QList<QString> GetHooks();
            QList<QString> GetFunctions();
            QJSEngine *GetEngine();
            // HOOKS
            void Hook_Shutdown();
            void Hook_EditPreProcess(WikiEdit *edit);
            void Hook_EditBeforePostProcess(WikiEdit *edit);
            void Hook_EditPostProcess(WikiEdit *edit);
            bool Hook_EditLoadToQueue(WikiEdit *edit);
            void Hook_FeedProvidersOnInit(WikiSite *site);
            void Hook_OnRevert(WikiEdit *edit);
            void Hook_OnGood(WikiEdit *edit);
            void Hook_OnSuspicious(WikiEdit *edit);
            int Hook_EditRescore(WikiEdit *edit);
            void Hook_WarningFinished(WikiEdit *edit);
            void SubscribeHook(int hook, QString function_name);
            void UnsubscribeHook(int hook);
            bool HookSubscribed(int hook);
            virtual int GetHookID(QString hook);
        protected:
            static QList<QString> loadedPaths;
            static QHash<QString, Script*> scripts;
            bool loadSource(QString source, QString *error);
            bool executeFunctionAsBool(QString function, QJSValueList parameters);
            bool executeFunctionAsBool(QString function);
            QString executeFunctionAsString(QString function);
            QString executeFunctionAsString(QString function, QJSValueList parameters);
            QJSValue executeFunction(QString function, QJSValueList parameters);
            QJSValue executeFunction(QString function);
            virtual void registerFunction(QString name, QString help = "", bool is_unsafe = false);
            virtual void registerClass(QString name, GenericJSClass *c);
            virtual void registerClasses();
            virtual void registerHook(QString name, int parameters, QString help = "", bool is_unsafe = false);
            //! Makes all functions available to ECMA
            virtual void registerFunctions();
            QJSEngine *engine;
            QJSValue script_ptr;
            QList<QString> hooksExported;
            QList<QString> functionsExported;
            QHash<QString, QString> functionsHelp;
            QString sourceCode;
            QString scriptPath;
            QString scriptName;
            QString scriptDesc;
            QString scriptAuthor;
            QString scriptVers;
            QList<GenericJSClass*> classes;
            bool isWorking;
            bool isLoaded;
            bool isUnsafe;
            QHash<int, QString> attachedHooks;
    };
}

#endif // SCRIPT_HPP
