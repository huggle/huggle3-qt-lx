//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <iostream>
#include <QString>

namespace Huggle
{
    //! Every exception raised by huggle is defined by this class
    class Exception
    {
        public:
            //! Error code
            int ErrorCode;
            //! Source
            QString Source;
            //! Reason for crash
            QString Message;
            //! ctor
            Exception(QString Text, bool __IsRecoverable = true);
            Exception(QString Text, QString _Source, bool __IsRecoverable = true);
            bool IsRecoverable() const;
        private:
            bool _IsRecoverable;
    };
}

#endif // EXCEPTION_H
