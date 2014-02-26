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
// now we need to ensure that python is included first, because it
// simply suck :P
// seriously, Python.h is shitty enough that it requires to be
// included first. Don't believe it? See this:
// http://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QString>
#include "configuration.hpp"
#include "wikisite.hpp"

namespace Huggle
{
    //! Namespaces
    enum MediaWikiNS
    {
        MediaWikiNS_Main,
        MediaWikiNS_Talk,
        MediaWikiNS_Project,
        MediaWikiNS_ProjectTalk,
        MediaWikiNS_User,
        MediaWikiNS_UserTalk,
        MediaWikiNS_Help,
        MediaWikiNS_HelpTalk,
        MediaWikiNS_Category,
        MediaWikiNS_CategoryTalk,
        MediaWikiNS_Mediawiki,
        MediaWikiNS_MediawikiTalk,
        MediaWikiNS_File,
        MediaWikiNS_FileTalk,
        MediaWikiNS_Portal,
        MediaWikiNS_PortalTalk,
        MediaWikiNS_Special
    };

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
            //! Retrieve a namespace ID for current page
            MediaWikiNS GetNS();
            //! Return true in case this is a talk page
            bool IsTalk();
            WikiPage *RetrieveTalk();
            QString RootName();
            bool IsUserpage();
            QString Contents;
            //! Name of page
            QString PageName;
            //! Site this page is on
            WikiSite *Site;
    };
}

#endif // WIKIPAGE_H
