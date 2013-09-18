//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "query.h"

unsigned int Query::LastID = 0;
QNetworkAccessManager Query::NetworkManager;

Query::Query()
{
    this->Result = NULL;
    this->Type = QueryNull;
    this->Status = Null;
    this->ID = this->LastID;
    this->LastID++;
    this->CustomStatus = "";
}

Query::~Query()
{
    delete Result;
}

bool Query::Processed()
{
    if (this->Status == Done)
    {
        return true;
    }
    return false;
}

QString Query::QueryTypeToString()
{
    switch (this->Type)
    {
        case QueryNull:
            return "null";
        case QueryApi:
            return "Api Query";
    }
    return "Unknown";
}

QString Query::QueryTargetToString()
{
    return "Invalid target";
}

QString Query::QueryStatusToString()
{
    if (this->CustomStatus != "")
    {
        return CustomStatus;
    }

    switch (this->Status)
    {
    case Null:
        return "NULL";
    case Done:
        return "Done";
    case Processing:
        return "Processing";
    case InError:
        return "InError";
    }
    return "Unknown";
}
