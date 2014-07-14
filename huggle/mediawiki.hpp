//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef MEDIAWIKI_HPP
#define MEDIAWIKI_HPP

#include "definitions.hpp"

#include <QString>
#include <QDateTime>

namespace Huggle
{
    //! Helper functions to convert some items to mediawiki format
    namespace MediaWiki
    {
        QDateTime   FromMWTimestamp(QString timestamp);
        QString     ToMWTimestamp(QDateTime DateTime_);
    }
}

#endif // MEDIAWIKI_HPP
