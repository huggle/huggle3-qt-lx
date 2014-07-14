//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef MEDIAWIKIOBJECT_HPP
#define MEDIAWIKIOBJECT_HPP

#include "definitions.hpp"

namespace Huggle
{
    class WikiSite;

    //! Every mediawiki asset may be inherited from this

    //! This class makes it simple to create cross-wiki support for various types that are bound to a given site
    class MediaWikiObject
    {
        public:
            MediaWikiObject();
            MediaWikiObject(MediaWikiObject *m);
            MediaWikiObject(const MediaWikiObject &m);
            virtual ~MediaWikiObject();
            virtual WikiSite *GetSite();
            WikiSite *Site;
    };
}

#endif // MEDIAWIKIOBJECT_HPP
