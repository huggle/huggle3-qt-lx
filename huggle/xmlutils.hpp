//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef XMLUTILS_H
#define XMLUTILS_H

#include "definitions.hpp"

#include <QtXml>
#include <QList>

namespace Huggle
{
    //! Provides a set of functions that extend Qt's Dom model with features that were missing in it
    namespace XmlUtils
    {
        HUGGLE_EX QList<QDomElement> FetchElementsFromDocument(QDomDocument xmls);
        HUGGLE_EX QList<QDomElement> FetchAllElementsByName(QDomDocument xmls, QString name);
    }
}

#endif // XMLUTILS_H
