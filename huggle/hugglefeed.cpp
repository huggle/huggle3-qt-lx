//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "configuration.hpp"
#include "hugglefeed.hpp"
#include "exception.hpp"
#include "wikisite.hpp"

using namespace Huggle;

QList<HuggleFeed*> HuggleFeed::Providers;

HuggleFeed::HuggleFeed(WikiSite *site)
{
    this->Site = site;
    this->EditCounter = 0;
    this->RvCounter = 0;
    this->UptimeDate = QDateTime::currentDateTime();
    Providers.append(this);
}

HuggleFeed::~HuggleFeed()
{
    if (Providers.contains(this))
        Providers.removeOne(this);
}

double HuggleFeed::GetUptime()
{
    return (double)this->UptimeDate.secsTo(QDateTime::currentDateTime());
}
