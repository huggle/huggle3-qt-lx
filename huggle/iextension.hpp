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
    class iExtension
    {
        public:
            iExtension() {}
            /*!
             * \brief IsWorking
             * \return if extension work
             */
            virtual bool IsWorking() { return false; }
            virtual ~iExtension() {}
            virtual bool Register() { return false; }
            void huggle__internal_SetPath(QString path);
            HUGGLE_EX QString GetExtensionFullPath();
            virtual QString CompiledFor()
            {
                // version of huggle this extension was built for
                return QString(HUGGLE_VERSION);
            }
            /*!
             * \brief This is called when the extension is removed from system
             */
            virtual bool Quit() { return false; }
            /*!
             * \brief Hook_EditPreProcess is called when edit is being pre processed
             * \param edit is a pointer to edit in question
             */
            virtual void Hook_EditPreProcess(void *edit) {}
            virtual void Hook_SpeedyFinished(void *edit, QString tags, bool successfull) {}
            /*!
             * \brief Hook_SpeedyBeforeOK Called right after user request processing of speedy form
             * \param edit
             * \param form
             * \return if false is returned whole operation is refused
             */
            virtual bool Hook_SpeedyBeforeOK(void *edit, void *form) { return true; }
            virtual void Hook_Shutdown() {}
            /*!
             * \brief Hook_EditScore is called after edit score is calculated
             * \param edit
             */
            virtual void Hook_EditScore(void *edit) {}
            virtual void Hook_EditPostProcess(void *edit) {}
            virtual bool Hook_EditBeforeScore(QString text, QString page, int* editscore, int userscore) { return true; }
            virtual void Hook_MainWindowOnLoad(void *window) {}
            virtual void Hook_BadnessScore(void *user, int score) {}
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
            //! Pointer to huggle core
            void *HuggleCore;
            //! Pointer to global system configuration
            void *Configuration;
            void *Localization;
            QNetworkAccessManager *Networking;
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
