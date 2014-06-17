//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "queryresult.hpp"

using namespace Huggle;

QueryResult::QueryResult()
{
    this->Data = "";
    this->ErrorMessage = "";
    this->Failed = false;
}

QueryResult::QueryResult(bool failed)
{
    this->Data = "";
    this->ErrorMessage = "";
    if (!failed)
    {
        this->Failed = false;
    }
    else
    {
        this->SetError();
    }
}

void QueryResult::SetError()
{
    this->ErrorCode = 1;
    this->Failed = true;
    this->ErrorMessage = "Unknown error";
}

void QueryResult::SetError(QString error)
{
    this->ErrorCode = 1;
    this->ErrorMessage = error;
    this->Failed = true;
}

void QueryResult::SetError(int error, QString details)
{
    this->ErrorMessage = details;
    this->Failed = true;
    this->ErrorCode = error;
}
