//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef IEXTENSION_H
#define IEXTENSION_H

#include "definitions.hpp"

#include <QtPlugin>
#include <QList>
#include <QNetworkAccessManager>
#include <QString>

#if _MSC_VER
#pragma warning ( push )
#pragma warning ( disable : 4100 )
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

namespace Huggle
{
    //! Extension interface

    //! Keep in mind that extensions are running in separate domain so that if you want to have access
    //! to resources like configuration or network, you need to explicitly request it.
    class HUGGLE_EX iExtension
    {
        public:
            iExtension() {}
            /*!
             * \brief IsWorking
             * \return if extension work
             */
            virtual ~iExtension() {}
            virtual bool IsWorking() { return false; }
            virtual bool Register() { return false; }
            void huggle__internal_SetPath(QString path);
            QString GetExtensionFullPath();
            virtual QString CompiledFor()
            {
                // version of huggle this extension was built for
                return QString(HUGGLE_VERSION);
            }
            virtual void Init();
            /*!
             * \brief This is called when the extension is removed from system
             */
            virtual bool Quit() { return false; }
            //! Name of the extension
            virtual QString GetExtensionName() { return "Unknown"; }
            //! User who created this thing
            virtual QString GetExtensionAuthor() { return ""; }
            virtual QString GetExtensionVersion() { return "1.0"; }
            virtual QString GetExtensionDescription() { return "No description"; }
            //! Whether this extension need access to huggle configs
            virtual bool RequestConfiguration() { return false; }
            //! Whether this extension need access to core
            virtual bool RequestCore() { return false; }
            virtual bool RequestNetwork() { return false; }
            /*!
             * \brief GetConfig Gets a value stored by extension from huggle configuration file
             * \param key Key
             * \param dv Default value
             * \return Value
             */
            virtual QString GetConfig(QString key, QString dv = "");
            virtual void SetConfig(QString key, QString value);
            /*!
             * \brief Hook_BadnessScore When the score of user is changed
             * \param user pointer to user whom score is changed
             * \param score New score of user
             */
            virtual void Hook_BadnessScore(void *user, int score) {}
            /*!
             * \brief Hook_EditScore is called after edit score is calculated
             * \param edit
             */
            virtual void Hook_EditScore(void *edit) {}
            /*!
             * \brief Hook_EditPostProcess Event that is called after edit is post processed by internal edit processor
             * \param edit Edit that was just post processed by huggle internal edit processor
             */
            virtual void Hook_EditPostProcess(void *edit) {}
            /*!
             * \brief Hook_EditBeforeScore This is called before internal edit scoring happens
             * \param edit Pointer to edit
             * \return If false is returned the internal scoring is skipped
             */
            virtual bool Hook_EditBeforeScore(void *edit) { return true;  }
            /*!
             * \brief Hook_EditBeforeScore This is called before internal edit scoring happens
             * \param text Text of edit
             * \param page Name of page
             * \param editscore Pointer to integer that contains current score of edit
             * \param userscore Score of user
             * \return If false is returned the internal scoring is skipped
             */
            virtual bool Hook_EditBeforeScore(QString text, QString page, int* editscore, int userscore) { return true; }
            virtual void Hook_EditBeforePostProcessing(void *edit) {}
            virtual bool Hook_OnEditLoadToQueue(void *edit) { return true; }
            /*!
             * \brief Hook_EditIsReady Event that checks if edit can be considered processed
             *
             * In case there are some extensions that add extra stuff to edit processing (such as extra queries) and need to wait
             * for them to finish, they can return false here in case they are still waiting for some query to finish
             * so that this edit is hold in a queue instead of being distributed to interface. This is useful in case you make
             * extension that needs to execute asynchronous jobs during the processing of each edit.
             * \param edit Pointer to WikiEdit
             * \return
             */
            virtual bool Hook_EditIsReady(void *edit) { return true; }
            /*!
             * \brief Hook_EditPreProcess is called when edit is being pre processed
             * \param edit is a pointer to edit in question
             */
            virtual void Hook_EditPreProcess(void *edit) {}
            virtual void Hook_GoodEdit(void *edit) {}
            /*!
             * \brief Hook_RevertPreflight is called before preflight check is executed and if
             * false is returned, the revert is cancelled with no warnings
             */
            virtual bool Hook_RevertPreflight(void *edit) { return true; }
            virtual void Hook_SpeedyFinished(void *edit, QString tags, bool successfull) {}
            /*!
             * \brief Hook_SpeedyBeforeOK Called right after user request processing of speedy form
             * \param edit
             * \param form
             * \return if false is returned whole operation is refused
             */
            virtual bool Hook_SpeedyBeforeOK(void *edit, void *form) { return true; }
            virtual void Hook_Shutdown() {}
            virtual void Hook_MainWindowOnLoad(void *window) {}
            virtual bool Hook_MainWindowReloadShortcut(void *shortcut) { return true; }
            virtual void Hook_MainWindowOnRender() {}
            //! Pointer to huggle core, set by extension loader
            void *HuggleCore = nullptr;
            //! Pointer to global system configuration, set by extension loader
            void *Configuration = nullptr;
            void *Localization = nullptr;
            QNetworkAccessManager *Networking = nullptr;
        private:
            QString huggle__internal_ExtensionPath;
    };

    class ExtensionHolder : public iExtension
    {
        public:
            QString GetExtensionName() { return this->Name; }
            QString Name;
    };
}

#if _MSC_VER
#pragma warning ( default: 4100 )
#else
#pragma GCC diagnostic pop
#endif

Q_DECLARE_INTERFACE(Huggle::iExtension, "org.huggle.extension.qt")

#endif // IEXTENSION_H
