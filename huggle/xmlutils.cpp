//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "xmlutils.hpp"

using namespace Huggle;

static void RecursiveFetch(QDomNode node, QList<QDomElement> *list)
{
    list->append(node.toElement());
    foreach (QDomNode nx, node.childNodes())
    {
        RecursiveFetch(nx, list);
    }
}

static void RecursiveFetch(QDomNode node, QList<QDomElement> *list, QString name)
{
    if (node.nodeName() == name)
        list->append(node.toElement());
    foreach (QDomNode nx, node.childNodes())
    {
        RecursiveFetch(nx, list);
    }
}

QList<QDomElement> XmlUtils::FetchElementsFromDocument(QDomDocument xmls)
{
    QList<QDomElement> results;
    foreach (QDomNode node, xmls.childNodes())
    {
        RecursiveFetch(node, &results);
    }
    return results;
}


QList<QDomElement> XmlUtils::FetchAllElementsByName(QDomDocument xmls, QString name)
{

}
