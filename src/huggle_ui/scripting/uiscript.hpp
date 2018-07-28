//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef UISCRIPT_HPP
#define UISCRIPT_HPP

#include <huggle_core/definitions.hpp>
#include <huggle_core/scripting/script.hpp>

class QAction;
class QMenu;

#define HUGGLE_SCRIPT_HOOK_LOGIN_OPEN                   20000
#define HUGGLE_SCRIPT_HOOK_MAIN_OPEN                    20001
#define HUGGLE_SCRIPT_HOOK_SPEEDY_FINISHED              20010
#define HUGGLE_SCRIPT_HOOK_ON_RENDER                    20011
#define HUGGLE_SCRIPT_HOOK_ON_STATUSBAR_UPDATE          20012

namespace Huggle
{
    class UiScript;

    class HUGGLE_EX_UI ScriptMenu
    {
        public:
            ScriptMenu(UiScript *s, QMenu *parent, QString text, QString fc, bool checkable, QAction *before = nullptr);
            ~ScriptMenu();
            QAction *GetAction();
            void SetChecked(bool checked);
            QString GetCallback();
            QMenu *GetParent();

        private:
            QString title;
            QString callback;
            QMenu *parentMenu = nullptr;
            UiScript *script;
            QAction *item = nullptr;
    };

    class HUGGLE_EX_UI UiScript : public Script
    {
            Q_OBJECT
        public:
            static QList<UiScript*> GetAllUiScripts();
            static void Autostart();

            UiScript();
            ~UiScript();
            QString GetContext();
            unsigned int GetContextID();
            int RegisterMenu(QMenu *parent, QString title, QString fc, bool checkable);
            void UnregisterMenu(int menu);
            void ToggleMenuCheckState(int menu, bool checked);
            bool OwnMenu(int menu_id);
            void Hook_OnMain();
            void Hook_OnLogin();
            void Hook_OnRender();
            void Hook_OnSpeedyFinished(WikiEdit *edit, QString tags, bool success);
            QString Hook_OnMainStatusbarUpdate(QString text);
            int GetHookID(QString hook);
        public slots:
            void MenuClicked();
        private:
            static QList<UiScript*> uiScripts;
            void registerClasses();
            void registerFunctions();
            int lastMenu = 0;
            QHash<QAction*, ScriptMenu*> scriptMenusByAction;
            QHash<int, ScriptMenu*> scriptMenus;
    };
}

#endif // UISCRIPT_HPP
