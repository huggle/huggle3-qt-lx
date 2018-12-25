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
#include <QUrl>

// This is here for performance reasons only, we could have a list of attached hooks as list of strings
// but strings usually perform way worse than hash of integers, so we just use integers instead
// If you add anything here don't forget to edit the GetHookID functions

// If you are working in different context than core, you can define your own IDs, but each context is prefixed
// with some number so that you never get a conflicting number
#define HUGGLE_SCRIPT_HOOK_SHUTDOWN                             0
#define HUGGLE_SCRIPT_HOOK_EDIT_BEFORE_PRE_PROCESS              19
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
#define HUGGLE_SCRIPT_HOOK_REVERT_PREFLIGHT                     11
#define HUGGLE_SCRIPT_HOOK_LOCALCONFIG_WRITE                    12
#define HUGGLE_SCRIPT_HOOK_LOCALCONFIG_READ                     13
#define HUGGLE_SCRIPT_HOOK_HAN_GOOD                             14
#define HUGGLE_SCRIPT_HOOK_HAN_REVERT                           15
#define HUGGLE_SCRIPT_HOOK_HAN_MESSAGE                          16
#define HUGGLE_SCRIPT_HOOK_HAN_RESCORE                          17
#define HUGGLE_SCRIPT_HOOK_HAN_SUSPICIOUS                       18
                                                                // 20 (19 already in use)

namespace Huggle
{
    class GenericJSClass;
    class Script;
    class ScriptFunctionHelp;
    class WikiEdit;
    class WikiSite;
    class WikiUser;
    class WikiPage;
    class HUGGLE_EX_CORE ScriptException : public Exception
    {
        public:
            ScriptException(const QString &text, const QString &source, Script *scr, bool is_recoverable = true);
        private:
            Script *s;
    };

    /*!
     * \brief The ScriptMemPool class is managing access to C++ objects from JS code
     *        this class is a pool of C++ pointers mapped to JS integers.
     *
     * Few words on how pointer marshalling works and why we need it:
     * Most of Huggle C++ functions require pointers to classes to work. So that if we wanted to call them
     * from JS code, which we do, we need to be somehow able to give these pointers to JS structures so that
     * we can use these pointers as arguments to JS functions that are calling the C++ functions.
     *
     * For that this class is used. It contains functions that allow us to temporarily store a pointer in a memory
     * hash and get a simple ID (also known as pool_id elsewhere in the code) which is just an integer that is passed
     * to JS code. This class allows safe registering of C++ pointers and conversion of pool_id's back to pointers.
     *
     * The lifespan of pool_id is typically very short, just for a period of hook execution.
     */
    class HUGGLE_EX_CORE ScriptMemPool
    {
        public:
            WikiEdit *GetEdit(int edit);
            int RegisterEdit(WikiEdit *edit);
            bool UnregisterEdit(WikiEdit *edit);
        private:
            int lastEdit = 0;
            QHash<WikiEdit*, int> editToIntMap;
            QHash<int, WikiEdit*> intToEditMap;
    };

    class HUGGLE_EX_CORE Script : public QObject
    {
            Q_OBJECT
        public:
            static Script *GetScriptByPath(const QString &path);
            static Script *GetScriptByEngine(QJSEngine *e);
            static Script *GetScriptByName(const QString& name);
            static QList<Script*> GetScripts();
            static QJSValue ProcessURL(const QUrl& url);

            Script();
             ~Script() override;
            virtual bool Load(const QString &path, QString *error);
            virtual bool LoadSrc(const QString &unique_id, const QString &source, QString *error);
            virtual void Unload();
            QString GetDescription();
            QString GetName();
            QString GetVersion();
            QString GetPath();
            QString GetAuthor();
            bool IsWorking();
            QJSValue ExecuteFunction(const QString &function);
            QJSValue ExecuteFunction(const QString &function, const QJSValueList &parameters);
            virtual unsigned int GetContextID();
            virtual QString GetContext();
            bool IsUnsafe();
            bool SupportFunction(const QString& name);
            QString GetHelpForFunc(const QString& name);
            QList<QString> GetHooks();
            QList<QString> GetFunctions();
            ScriptMemPool *GetMemPool();
            QJSEngine *GetEngine();
            // External callbacks exists for security reasons - each script can expose its own
            // callbacks for other scripts to execute, but other scripts are only allowed to
            // execute callbacks that were previously exported using these
            QJSValue ExternalCallback(const QString& callback, const QJSValueList& parameters);
            void RegisterExternalCallback(const QString& callback);
            void UnregisterExternalCallback(const QString& callback);
            bool HasExternalCallback(const QString& callback);
            // HOOKS
            void Hook_Shutdown();
            bool Hook_EditBeforePreProcess(WikiEdit *edit);
            void Hook_EditPreProcess(WikiEdit *edit);
            void Hook_EditBeforePostProcess(WikiEdit *edit);
            void Hook_EditPostProcess(WikiEdit *edit);
            bool Hook_EditLoadToQueue(WikiEdit *edit);
            void Hook_FeedProvidersOnInit(WikiSite *site);
            void Hook_OnRevert(WikiEdit *edit);
            void Hook_OnGood(WikiEdit *edit);
            void Hook_OnSuspicious(WikiEdit *edit);
            bool Hook_OnRevertPreflight(WikiEdit *edit);
            int Hook_EditRescore(WikiEdit *edit);
            void Hook_WarningFinished(WikiEdit *edit);
            void Hook_OnLocalConfigRead();
            void Hook_OnLocalConfigWrite();
            bool Hook_HAN_Good(WikiEdit *edit, const QString& nick, const QString& ident, const QString& host);
            bool Hook_HAN_Suspicious(WikiEdit *edit, const QString& nick, const QString& ident, const QString& host);
            bool Hook_HAN_Rescore(WikiEdit *edit, long score, const QString& nick, const QString& ident, const QString& host);
            bool Hook_HAN_Revert(WikiEdit *edit, const QString& nick, const QString& ident, const QString& host);
            bool Hook_HAN_Message(WikiSite *w, const QString& message, const QString& nick, const QString& ident, const QString& host);
            void SubscribeHook(int hook, const QString& function_name);
            void UnsubscribeHook(int hook);
            bool HookSubscribed(int hook);
            virtual int GetHookID(const QString &hook);
        protected:
            static QList<QString> loadedPaths;
            static QHash<QString, Script*> scripts;
            bool loadSource(QString source, QString *error);
            bool executeFunctionAsBool(const QString &function, const QJSValueList &parameters);
            bool executeFunctionAsBool(const QString &function);
            QString executeFunctionAsString(const QString &function);
            QString executeFunctionAsString(const QString &function, const QJSValueList &parameters);
            QJSValue executeFunction(const QString &function, const QJSValueList &parameters);
            QJSValue executeFunction(const QString& function);
            virtual void registerFunction(const QString &name, const QString &help = "", bool is_unsafe = false);
            virtual void registerClass(const QString &name, GenericJSClass *c);
            virtual void registerClasses();
            virtual void registerHook(const QString &name, int parameters, const QString &help = "", bool is_unsafe = false);
            //! Makes all functions available to ECMA
            virtual void registerFunctions();
            QJSEngine *engine;
            ScriptMemPool *memPool;
            QJSValue script_ptr;
            QList<QString> hooksExported;
            QList<QString> functionsExported;
            QHash<QString, QString> functionsHelp;
            QHash<QString, ScriptFunctionHelp> functionsDocs;
            QString sourceCode;
            QString scriptPath;
            QString scriptName;
            QString scriptDesc;
            QString scriptAuthor;
            QString scriptVers;
            QList<QString> externalCallbacks;
            QList<GenericJSClass*> classes;
            bool isWorking;
            bool isLoaded;
            bool isUnsafe;
            QHash<int, QString> attachedHooks;
    };
}

#endif // SCRIPT_HPP
