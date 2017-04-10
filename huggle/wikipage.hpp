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

#include <QStringList>
#include <QString>
#include "mediawikiobject.hpp"

namespace Huggle
{
    class WikiPageNS;
    class WikiSite;

    //! Mediawiki page
    class HUGGLE_EX WikiPage : public MediaWikiObject
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
            //! True if original creator of this page is known
            bool FounderKnown();
            QString GetFounder();
            void SetFounder(QString name);
            //! Returns a new instance of WikiPage that is pointed to talk page of this page
            WikiPage *RetrieveTalk();
            QString RootName();
            bool IsUserpage();
            //! Writes a wikipage name that is encoded using percent encoding
            QString EncodedName();
            QString Contents;
            //! Name of page
            QString PageName;
            QStringList GetCategories();
            void SetCategories(QStringList value);
            bool IsWatched();
            void SetWatched(bool value);
        private:
            QStringList categories;
            WikiPageNS *NS;
            QString founder;
            bool founderKnown = false;
            bool watched = false;
    };

    inline QString WikiPage::SanitizedName()
    {
        return this->PageName.replace(" ", "_");
    }

    inline WikiPageNS *WikiPage::GetNS()
    {
        return this->NS;
    }

    inline QStringList WikiPage::GetCategories()
    {
        return this->categories;
    }

    inline void WikiPage::SetCategories(QStringList value)
    {
        this->categories = value;
    }

    inline bool WikiPage::IsWatched()
    {
        return this->watched;
    }

    inline void WikiPage::SetWatched(bool value)
    {
        this->watched = value;
    }
}

#endif // WIKIPAGE_H
