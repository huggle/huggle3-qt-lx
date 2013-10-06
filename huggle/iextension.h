//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef IEXTENSION_H
#define IEXTENSION_H

#include <QtPlugin>
#include <QList>
#include <QString>

namespace Huggle
{
    //! Extension interface
    class iExtension
    {
    public:
        static QList<iExtension *> Extensions;
        iExtension();
        virtual ~iExtension();
        virtual bool Register() { return false; }
    private:
        QString ExtensionName;
        QString ExtensionAuthor;
        QString ExtensionVersion;
        QString ExtensionDescription;
    };
}

Q_DECLARE_INTERFACE(Huggle::iExtension, "org.huggle.extension.qt")

#endif // IEXTENSION_H
