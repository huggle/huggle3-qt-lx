//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "mediawiki.hpp"

using namespace Huggle;

QDateTime MediaWiki::FromMWTimestamp(QString timestamp)
{
    QDateTime date = QDateTime::fromString(timestamp, "yyyy-MM-ddThh:mm:ssZ");
    date.setTimeSpec(Qt::UTC);
    return date;
}

QString MediaWiki::ToMWTimestamp(QDateTime DateTime_)
{
    return DateTime_.toString("yyyy-MM-ddThh:mm:ssZ");
}
