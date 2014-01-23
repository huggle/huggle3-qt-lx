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

#include <QString>
#include "configuration.hpp"

namespace Huggle
{
    class Configuration;

    //! This class is used to parse various text, such as configuration keys
    class HuggleParser
    {
        public:
            static QString GetSummaryOfWarningTypeFromWarningKey(QString key);
            static QString GetNameOfWarningTypeFromWarningKey(QString key);
            static QString GetKeyOfWarningTypeFromWarningName(QString id);
            //! Remove leading and finishing space of string
            static QString Trim(QString text);
            //! Parse a part patterns for score words
            static void ParsePats(QString text);
            static void ParseWords(QString text);
            static QString GetValueFromKey(QString item);
            static QString GetKeyFromValue(QString item);
            /*!
             * \brief Process content of talk page in order to figure which user level they have
             * \param page The content of talk page
             * \return Level
             */
            static int GetLevel(QString page);
    };
}

#endif // HUGGLEPARSER_HPP
