//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEPARSER_HPP
#define HUGGLEPARSER_HPP

#include "definitions.hpp"

#include <QString>
#include <QDateTime>
#include <QList>
#include "hugglequeuefilter.hpp"

namespace Huggle
{
    class Configuration;
    class ProjectConfiguration;
    class HuggleQueueFilter;
    class WikiSite;

    //! This namespace contains functions to parse various text, such as configuration keys
    namespace HuggleParser
    {
        //! Parse a string from configuration which has format used by huggle 2x
        /*!
         * \param key Key
         * \param content Text to parse from
         * \param missing Default value in case this key is missing in text
         * \return Value of key, in case there is no such a key content of missing is returned
         */
        HUGGLE_EX QString ConfigurationParse(QString key, QString content, QString missing = "");
        //! \todo This function needs a unit test
        HUGGLE_EX QString GetSummaryOfWarningTypeFromWarningKey(QString key, ProjectConfiguration *project_conf);
        //! \todo This function needs a unit test
        HUGGLE_EX QString GetNameOfWarningTypeFromWarningKey(QString key, ProjectConfiguration *project_conf);
        //! \todo This function needs a unit test
        HUGGLE_EX QString GetKeyOfWarningTypeFromWarningName(QString id, ProjectConfiguration *project_conf);
        //! \todo This function needs a unit test
        /*!
         * \brief ConfigurationParse_QL Parses a QStringList of values for a given key
         * The list must be either separated by comma and newline or it can be a list of values separated
         * by comma only, however if you want to have multiple items per line, you need to set CS to true
         * \param key Key
         * \param content Text to parse key from
         * \param CS Whether the values are separated by comma only (if this is set to true there can be more items on a line)
         * \return List of values from text or empty list
         */
        HUGGLE_EX QStringList ConfigurationParse_QL(QString key, QString content, bool CS = false);
        //! \todo This function needs a unit test
        HUGGLE_EX QStringList ConfigurationParse_QL(QString key, QString content, QStringList list, bool CS = false);
        //! \todo This function needs a unit test
        HUGGLE_EX QStringList ConfigurationParseTrimmed_QL(QString key, QString content, bool CS = false, bool RemoveNull = false);
        //! \todo This function needs a unit test
        HUGGLE_EX QList<HuggleQueueFilter*> ConfigurationParseQueueList(QString content, bool locked = false);
        /*!
         * \brief GetIDOfMonth retrieve a month based on list of localized months in configuration file
         * \param month
         * \return If there is no such a month this function will return negative number
         */
        HUGGLE_EX byte_ht GetIDOfMonth(QString month, WikiSite *site);
        //! \todo This function needs a unit test
        //! Parse a part patterns for score words
        HUGGLE_EX void ParsePats(QString text, WikiSite *site);
        //! \todo This function needs a unit test
        HUGGLE_EX void ParseWords(QString text, WikiSite *site);
        //! \todo This function needs a unit test
        HUGGLE_EX QString GetValueFromKey(QString item);
        //! \todo This function needs a unit test
        HUGGLE_EX QString GetKeyFromValue(QString item);
        /*!
         * \brief Process content of talk page in order to figure which user level they have
         * \param page The content of talk page
         * \return Level
         */
        HUGGLE_EX byte_ht GetLevel(QString page, QDate bt, Huggle::WikiSite *site = nullptr);
    }
}

#endif // HUGGLEPARSER_HPP
