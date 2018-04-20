//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef UIEXCEPTIONHANDLER_HPP
#define UIEXCEPTIONHANDLER_HPP

#include <huggle_core/exceptionhandler.hpp>

namespace Huggle
{
    class UiExceptionHandler : public ExceptionHandler
    {
        public:
            UiExceptionHandler();
            void HandleException(Exception *e);
    };
}

#endif // UIEXCEPTIONHANDLER_HPP
