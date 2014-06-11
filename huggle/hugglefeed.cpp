//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglefeed.hpp"
#include "exception.hpp"

using namespace Huggle;

HuggleFeed *HuggleFeed::PrimaryFeedProvider = nullptr;
HuggleFeed *HuggleFeed::SecondaryFeedProvider = nullptr;

HuggleFeed::HuggleFeed()
{
    this->EditCounter = 0;
    this->RvCounter = 0;
    this->UptimeDate = QDateTime::currentDateTime();
}

HuggleFeed::~HuggleFeed()
{
}

double HuggleFeed::GetUptime()
{
    return (double)this->UptimeDate.secsTo(QDateTime::currentDateTime());
}
