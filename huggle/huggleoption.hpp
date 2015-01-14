//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEOPTION_HPP
#define HUGGLEOPTION_HPP

#include "definitions.hpp"

#include <QList>
#include <QStringList>
#include <QHash>
#include <QString>
#include <QVariant>

namespace Huggle
{
    /*!
     * \brief The HuggleOption class can be used to store user options in a very simple way
     *
     *  Every option here is tracked for changes, that makes the huggle store it to user config file only if it has changed,
     *  this is necessary for options that override the project configuration, because we want to store these, only if they
     *  differ
     */
    class HUGGLE_EX HuggleOption
    {
        public:
	    /*!
	     * \brief HuggleOption Creates a new instance of the HuggleOption class
	     * \param name The name of the user option
	     * \param value The state of the option, can hold the most common Qt variable types
	     * \param isdefault True if option should be default
	     */
            HuggleOption(QString name, QVariant value, bool isdefault);
	    /*!
	     * \brief IsDefault Returns if user option is marked as default
	     * \returns State of isDefault
	     */
            bool IsDefault()  { return this->isDefault; }
            /*!
	     * \brief SetVariant Sets a new value of the HuggleOption
	     * \param value The new state of the option, can hold the most common Qt variable types
	     */
            void SetVariant(QVariant value);
	    /*!
	     * \brief GetVariant Returns the actual value for this user option
	     * \returns The value of the option, can hold the most common Qt variable types
	     */
            QVariant GetVariant()  { return this->Value; }
            QString Name; ///< The name of the option

        private:
            QVariant Value;
            bool isDefault = true;
    };
}

#endif // HUGGLEOPTION_HPP
