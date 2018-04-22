//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef GENERIC_HPP
#define GENERIC_HPP

#include "definitions.hpp"

#include <QAbstractSocket>
#include <QString>

#ifdef MessageBox
    // fix GCC for windows headers port
    #undef MessageBox
#endif

class QWidget;

namespace Huggle
{
    //! Generic C++ functions that are missing in standard libs
    namespace Generic
    {
        /*!
         * \brief Bool2String Convert a bool to string
         * \param b bool
         * \return string
         */
        HUGGLE_EX_CORE QString Bool2String(bool b);
        HUGGLE_EX_CORE QStringList CSV2QStringList(QString CSV, QChar separator = ',');
        HUGGLE_EX_CORE QString HtmlEncode(QString text);
        /*!
         * \brief Safely turn a QString into a bool value
         * \param value QString Text that needs to be changed to bool
         * \param defaultvalue bool What value should bool have in case that text can't be parsed, by default it's false
         */
        HUGGLE_EX_CORE bool SafeBool(QString value, bool defaultvalue = false);
        //! Display a user message before reporting a user based on user preferences
        HUGGLE_EX_CORE bool ReportPreFlightCheck();
        HUGGLE_EX_CORE QString SanitizePath(QString name);
        HUGGLE_EX_CORE QString MD5(QString data);
        //! Display a message box telling user that function is not allowed during developer mode
        HUGGLE_EX_CORE void DeveloperError();
        HUGGLE_EX_CORE QString IRCQuitDefaultMessage();
        HUGGLE_EX_CORE QString SocketError2Str(QAbstractSocket::SocketError error);
        /*!
         * \brief ShrinkText makes string fit to a size, if text is longer, the extra part is replaced with ".."
         * \param text Text which is about to be made smaller
         * \param size Desired size of string, this function will alter it only if string is longer, size must be longer than minimum size
         * \param html if true spaces are replaced with html specials
         * \param minimum size
         * \return new string
         */
        HUGGLE_EX_CORE QString ShrinkText(QString text, unsigned int size, bool html = true, unsigned int minimum = 2);
    }
}

#endif // GENERIC_HPP
