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
// now we need to ensure that python is included first, because it
// simply suck :P
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QString>
#include <QList>
#include "hugglequeuefilter.hpp"

namespace Huggle
{
    class Configuration;
    class HuggleQueueFilter;

    //! This namespace contains functions to parse various text, such as configuration keys
    namespace HuggleParser
    {
        //! \todo This function needs a unit test
        QString GetSummaryOfWarningTypeFromWarningKey(QString key);
        //! \todo This function needs a unit test
        QString GetNameOfWarningTypeFromWarningKey(QString key);
        //! \todo This function needs a unit test
        QString GetKeyOfWarningTypeFromWarningName(QString id);
        //! \todo This function needs a unit test
        /*!
         * \brief ConfigurationParse_QL Parses a QStringList of values for a given key
         * The list must be either separated by comma and newline or it can be a list of values separated
         * by comma only
         * \param key Key
         * \param content Text to parse key from
         * \param CS Whether the values are separated by comma only (if this is set to true there can be more items on a line)
         * \return List of values from text or empty list
         */
        QStringList ConfigurationParse_QL(QString key, QString content, bool CS = false);
        //! \todo This function needs a unit test
        QStringList ConfigurationParse_QL(QString key, QString content, QStringList list, bool CS = false);
        //! \todo This function needs a unit test
        QStringList ConfigurationParseTrimmed_QL(QString key, QString content, bool CS = false, bool RemoveNull = false);
        //! \todo This function needs a unit test
        QList<HuggleQueueFilter*> ConfigurationParseQueueList(QString content, bool locked = false);
        /*!
         * \brief GetIDOfMonth retrieve a month based on list of localized months in configuration file
         * \param month
         * \return If there is no such a month this function will return negative number
         */
        byte_ht GetIDOfMonth(QString month);
        //! \todo This function needs a unit test
        //! Parse a part patterns for score words
        void ParsePats(QString text);
        //! \todo This function needs a unit test
        void ParseWords(QString text);
        //! \todo This function needs a unit test
        QString GetValueFromKey(QString item);
        //! \todo This function needs a unit test
        QString GetKeyFromValue(QString item);
        /*!
         * \brief Process content of talk page in order to figure which user level they have
         * \param page The content of talk page
         * \return Level
         */
        byte_ht GetLevel(QString page, QDate bt);
    }
}

#endif // HUGGLEPARSER_HPP
