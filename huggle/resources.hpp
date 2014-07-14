//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef RESOURCES_HPP
#define RESOURCES_HPP

#include "definitions.hpp"

#include <QFile>
#include <QString>

namespace Huggle
{
    //! Embedded resource files
    class Resources
    {
        public:
            static void Init();
            static QString GetHtmlHeader();
            static QString HtmlIncoming;
            //! This string contains a html header
            static QString HtmlHeader;
            static QString DiffHeader;
            static QString DiffFooter;
            static QString Html_StopFire;
            static QString CssRtl;
            //! This string contains a html footer
            static QString HtmlFooter;
    };
}

#endif // RESOURCES_HPP
