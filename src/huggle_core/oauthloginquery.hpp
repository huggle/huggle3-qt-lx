//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef OAUTHLOGINQUERY_H
#define OAUTHLOGINQUERY_H

#include "definitions.hpp"

#include <QString>
#include "query.hpp"

namespace Huggle
{
    //! This query is supposed to login user through oauth provider
    class OAuthLoginQuery : public Query
    {
        public:
            OAuthLoginQuery();
    };
}

#endif // OAUTHLOGINQUERY_H
