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
    class HuggleOption
    {
        public:
            HuggleOption(QString name, QVariant value, bool isdefault);
            bool IsDefault()  { return this->isDefault; }
            void SetVariant(QVariant value);
            QVariant GetVariant()  { return this->Value; }
            QString Name;

        private:
            QVariant Value;
            bool isDefault = true;
    };
}

#endif // HUGGLEOPTION_HPP
