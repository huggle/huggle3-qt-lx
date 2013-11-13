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

#include <QtPlugin>
#include <QList>
#include <QString>

#if _MSC_VER
#pragma warning ( push )
#pragma warning ( disable )
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

namespace Huggle
{
    //! Extension interface
    class iExtension
    {
        public:
            static QList<iExtension *> Extensions;
            iExtension();
            /*!
             * \brief IsWorking
             * \return if extension work
             */
            bool IsWorking();
            virtual ~iExtension();
            virtual bool Register() { return false; }
            /*!
             * \brief This is called when the extension is removed from system
             */
            virtual void Quit() { Working = false; }
            /*!
             * \brief Hook_EditPreProcess is called when edit is being pre processed
             * \param edit is a pointer to edit in question
             */
            virtual void Hook_EditPreProcess(void *edit) {}
            /*!
             * \brief Hook_EditScore is called after edit score is calculated
             * \param edit
             */
            virtual void Hook_EditScore(void *edit) {}
            virtual void Hook_EditPostProcess(void *edit) {}
            virtual bool Hook_EditBeforeScore(QString text, QString page, int* editscore, int userscore) { return true; }
            virtual void Hook_MainWindowOnLoad(void *window) {}
            //! Name of the extension
            QString ExtensionName;
            //! User who created this thing
            QString ExtensionAuthor;
            QString ExtensionVersion;
            QString ExtensionDescription;
        private:
            bool Working;
    };
}

#if _MSC_VER
#pragma warning ( pop )
#else
#pragma GCC diagnostic pop
#endif

Q_DECLARE_INTERFACE(Huggle::iExtension, "org.huggle.extension.qt")

#endif // IEXTENSION_H
