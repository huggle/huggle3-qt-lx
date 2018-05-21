//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "hugglequeryjs.hpp"
#include "../apiquery.hpp"
#include "../configuration.hpp"
#include "../syslog.hpp"

using namespace Huggle;

HuggleQueryJS::HuggleQueryJS(Script *s) : GenericJSClass(s)
{
    this->functions.insert("get_all_bytes_received", "(): return approximate bytes received by all queries");
    this->functions.insert("get_all_bytes_sent", "(): get approximate bytes sent to all queries");
}

QJSValue HuggleQueryJS::get_all_bytes_sent()
{
    return QJSValue(static_cast<double>(Query::GetBytesSentSinceStartup()));
}

QJSValue HuggleQueryJS::get_all_bytes_received()
{
    return QJSValue(static_cast<double>(Query::GetBytesReceivedSinceStartup()));
}
