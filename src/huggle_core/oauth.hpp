//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef OAUTH_H
#define OAUTH_H

#include "definitions.hpp"

#include <QString>

namespace Huggle
{
    //! Not being used now because wmf doesn't provide OAuth yet
    class OAuth
    {
        public:
            OAuth(QString login);
            bool CheckSubscription();
            bool RequestSubscription();
            QString ObtainToken();
    };
}

#endif // OAUTH_H
