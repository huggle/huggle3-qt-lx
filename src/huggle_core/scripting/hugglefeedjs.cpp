//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "hugglefeedjs.hpp"
#include "../hugglefeed.hpp"

using namespace Huggle;

HuggleFeedJS::HuggleFeedJS(Script *s) : GenericJSClass(s)
{
    this->functions.insert("get_all_bytes_sent", "(): returns bytes sent by all providers together");
    this->functions.insert("get_all_bytes_received", "(): returns bytes received by all providers together");
}

QJSValue HuggleFeedJS::get_all_bytes_sent()
{
    return QJSValue(static_cast<double>(HuggleFeed::GetTotalBytesSent()));
}

QJSValue HuggleFeedJS::get_all_bytes_received()
{
    return QJSValue(static_cast<double>(HuggleFeed::GetTotalBytesRcvd()));
}

QHash<QString, QString> HuggleFeedJS::GetFunctions()
{
    return this->functions;
}
