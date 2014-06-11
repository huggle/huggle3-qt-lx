//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WIKIPAGE_H
#define WIKIPAGE_H

#include "definitions.hpp"
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QString>
#include "wikisite.hpp"

namespace Huggle
{
    class WikiPageNS;
    class WikiSite;

    //! Mediawiki page
    class WikiPage
    {
        public:
            //! Create new empty instance of wiki page
            WikiPage();
            WikiPage(const QString &name);
            WikiPage(WikiPage *page);
            WikiPage(const WikiPage& page);
            QString SanitizedName();
            //! Retrieve a namespace object for current page
            WikiPageNS *GetNS();
            //! Return true in case this is a talk page
            bool IsTalk();
            WikiPage *RetrieveTalk();
            QString RootName();
            bool IsUserpage();
            //! Writes a wikipage name that is encoded using percent encoding
            QString EncodedName();
            QString Contents;
            //! Name of page
            QString PageName;
            //! Site this page is on
            WikiSite *Site;
        private:
            WikiPageNS *NS;
    };
}

#endif // WIKIPAGE_H
