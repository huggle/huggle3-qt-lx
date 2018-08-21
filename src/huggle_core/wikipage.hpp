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
    class HUGGLE_EX_CORE WikiPage : public MediaWikiObject
    {
        public:
            //! Create new empty instance of wiki page
            WikiPage(WikiSite *site);
            WikiPage(const QString &name, WikiSite *site);
            WikiPage(WikiPage *page);
            WikiPage(const WikiPage& page);
            //! Returns a corrected version of page name in case there are spaces
            QString SanitizedName();
            //! Retrieve a namespace object for current page
            WikiPageNS *GetNS();
            //! Return true in case this is a talk page
            bool IsTalk();
            //! True if original creator of this page is known
            bool FounderKnown();
            QString GetFounder();
            bool EqualTo(WikiPage *page);
            void SetFounder(QString name);
            //! Returns a new instance of WikiPage that is pointed to talk page of this page
            WikiPage *RetrieveTalk();
            QString RootName();
            bool IsUserpage();
            //! Writes a wikipage name that is encoded using percent encoding
            QString EncodedName();
            QStringList GetCategories();
            void SetCategories(QStringList value);
            bool IsWatched();
            void SetWatched(bool value);
            QString GetContent();
            void SetContent(QString content);
            //! Whether this instance of WikiPage contains its content, if false, it means that content of this page is unknown
            bool HasContent();
            QString Contents;
            //! Content model of a page, if known
            QString ContentModel;
            //! Name of page
            QString PageName;
        protected:
            QStringList categories;
            WikiPageNS *NS;
            QString founder;
            bool hasContent = false;
            bool founderKnown = false;
            bool watched = false;
    };

    inline QString WikiPage::SanitizedName()
    {
        QString sanitized_name = this->PageName;
        sanitized_name.replace(" ", "_");
        return sanitized_name;
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
