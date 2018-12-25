//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef LOCALIZATION_HPP
#define LOCALIZATION_HPP

#include "definitions.hpp"

// localization tool
#define _l Huggle::Localizations::HuggleLocalizations->Localize

#include <QStringList>
#include <QString>
#include <QList>
#include <QMap>

namespace Huggle
{
    /*!
     * \brief The Language class is used to store localization data
     */
    class HUGGLE_EX_CORE Language
    {
        public:
            //! Creates new instance of language
            //! param name Name of language
            Language(const QString& name);
            //! This is a short language name which is used by system
            QString LanguageName;
            //! Long identifier of language that is seen by user
            QString LanguageID;
            bool IsRTL = false;
            QMap<QString, QString> Messages;
    };

    //! This class is used to localize strings

    //! Huggle is using own localization system and usage of
    //! other 3rd localization system or libraries
    //! is strictly forbidden. Your commits that introduce
    //! those will be reverted mercillesly.
    class HUGGLE_EX_CORE Localizations
    {
        public:
            static int EnglishID;
            static Localizations *HuggleLocalizations;
            //! "qqx"-Language for outputting the used keys
            static const QString LANG_QQX;

            Localizations();
            ~Localizations();
            /*!
             * \brief Initializes a localization with given name
             *
             * This function will create a new localization object using built-in localization file
             * using Core::MakeLanguage() and insert that to language list
             * \param name Name of a localization that is a name of language without txt suffix in localization folder
             */
            void LocalInit(const QString& name, bool xml = true);
            QString Localize(const QString &key);
            QString Localize(const QString &key, const QStringList &parameters);
            QString Localize(const QString& key, const QString& parameter);
            QString Localize(const QString &key, const QString &par1, const QString &par2);
            //! Check if key exists by looking it up in default language hash, returns true even if key doesn't exist in preferred language
            bool KeyExists(const QString& key);
			
            //! Check whether the preferred language is RightToLeft language.
            bool IsRTL();
            //! Languages
            QList<Language*> LocalizationData;
            //! Language selected by user this is only a language of interface
            QString PreferredLanguage;
        private:
            static Language *MakeLanguage(const QString& text, const QString &name);
            static Language *MakeLanguageUsingXML(const QString& text, const QString& name);
    };
}

#endif // LOCALIZATION_HPP
