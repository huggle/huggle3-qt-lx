//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef EXCEPTIONHANDLER_HPP
#define EXCEPTIONHANDLER_HPP

#include "definitions.hpp"

namespace Huggle
{
    class Exception;

    //!
    //! \brief The ExceptionHandler class is used to create an abstract handler for exceptions
    //!
    //! This default instance is writing exceptions to terminal and then ungracefully kill the program,
    //! it can be overriden though
    //!
    class HUGGLE_EX_CORE ExceptionHandler
    {
        public:
            ExceptionHandler();
            virtual ~ExceptionHandler();
            virtual void HandleException(Exception *ex);
    };
}

#endif // EXCEPTIONHANDLER_HPP
