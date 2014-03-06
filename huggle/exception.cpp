//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "exception.hpp"

using namespace Huggle;

Exception::Exception(QString Text, bool __IsRecoverable)
{
    std::cerr << "FATAL Exception thrown: " + Text.toStdString() << std::endl;
    this->Message = Text;
    this->ErrorCode = 2;
    this->Source = "{hidden}";
    this->_IsRecoverable = __IsRecoverable;
}

Exception::Exception(QString Text, QString _Source, bool __IsRecoverable)
{
    std::cerr << "FATAL Exception thrown: " + Text.toStdString() << std::endl;
    this->Source = _Source;
    this->Message = Text;
    this->ErrorCode = 2;
    this->_IsRecoverable = __IsRecoverable;
}

bool Exception::IsRecoverable() const
{
    return this->_IsRecoverable;
}
