//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef UIHOOKS_HPP
#define UIHOOKS_HPP

#include <huggle_core/definitions.hpp>

namespace Huggle
{
    class WikiUser;
    class WikiEdit;
    class SpeedyForm;
    class UserinfoForm;
    class MainWindow;
    class Shortcut;
    namespace UiHooks
    {
        /*!
        * \brief Triggered when history of user is requested, usually right upon load of an edit in Main Window,
        *        these contributions are either loaded automatically, or manually, but always AFTER edit is loaded.
        *        They are retrieved using extra API query and this hook is triggered before the query is executed.
        * \param Edit in question, the history is retrieved for user who made this edit
        * \return If hook returns false, contrib query is not executed, use this if you want to provide own history
        */
       HUGGLE_EX_UI bool ContribBoxBeforeQuery(WikiUser *user, UserinfoForm *user_info);
       /*!
       * \brief Triggered when history of user is requested, usually right upon load of an edit in Main Window,
       *        these contributions are either loaded automatically, or manually, but always AFTER edit is loaded.
       *        They are retrieved using extra API query and this hook is triggered before the query is executed.
       * \param Edit in question, the history is retrieved for user who made this edit
       */
       HUGGLE_EX_UI void ContribBoxAfterQuery(WikiUser *user, UserinfoForm *user_info);
       HUGGLE_EX_UI void MainWindow_OnLoad(MainWindow *window);
       HUGGLE_EX_UI void MainWindow_OnRender();
       HUGGLE_EX_UI bool MainWindow_ReloadShortcut(Shortcut *shortcut);
       HUGGLE_EX_UI bool Speedy_BeforeOK(WikiEdit *edit, SpeedyForm *form);
       HUGGLE_EX_UI void Speedy_Finished(WikiEdit *edit, QString tags, bool success);
    }
}

#endif // UIHOOKS_HPP
