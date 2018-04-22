//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef UIGENERIC_HPP
#define UIGENERIC_HPP

#include <huggle_core/definitions.hpp>

class QWidget;
namespace Huggle
{
    enum MessageBoxStyle
    {
        MessageBoxStyleNormal, // OK
        MessageBoxStyleQuestion, // Yes No
        MessageBoxStyleQuestionAbort, // Yes No Cancel
        MessageBoxStyleWarning, // OK (warning)
        MessageBoxStyleError // OK (error)
    };

    class WikiUser;
    namespace UiGeneric
    {
        /*!
         * \brief MessageBox Display a message box
         * \param title Title of message box
         * \param text What is displayed in a message
         */
        HUGGLE_EX int pMessageBox(QWidget *parent, QString title, QString text, MessageBoxStyle st = MessageBoxStyleNormal, bool enforce_stop = false);
        HUGGLE_EX int MessageBox(QString title, QString text, MessageBoxStyle st = MessageBoxStyleNormal, bool enforce_stop = false, QWidget *parent = nullptr);
        HUGGLE_EX void DisplayContributionBrowser(WikiUser *User, QWidget *parent = nullptr);
    }
}

#endif // UIGENERIC_HPP
