//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "exceptionhandler.hpp"
#include "exception.hpp"
#include <iostream>
#include <QCoreApplication>

using namespace Huggle;

ExceptionHandler::ExceptionHandler()
{

}

ExceptionHandler::~ExceptionHandler()
{

}

void ExceptionHandler::HandleException(Exception *ex)
{
    std::cout << "FATAL: Unhandled exception occured, description: " << ex->Message.toStdString() << std::endl
              << "Source: " << ex->Source.toStdString() << std::endl
              << "Stack: " << ex->StackTrace.toStdString() << std::endl;

    // Kill the application to prevent collateral damage to system
    QCoreApplication::exit(ex->ErrorCode);
}
