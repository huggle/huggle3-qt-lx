//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WIKISITE_H
#define WIKISITE_H

#include "definitions.hpp"

#include <QString>
#include <QHash>
#include "projectconfiguration.hpp"
#include "userconfiguration.hpp"
#include "version.hpp"

namespace Huggle
{
    class HuggleFeed;
    class HuggleQueueFilter;
    //! Namespace (mediawiki)
    class HUGGLE_EX_CORE WikiPageNS
    {
        public:
            WikiPageNS(int id, const QString &localized_name, QString canonical_name);
            WikiPageNS(const WikiPageNS &k);
            WikiPageNS(WikiPageNS *k);
            ~WikiPageNS()=default;
            QString GetName();
            QString GetCanonicalName();
            bool IsTalkPage();
            int GetID();
        private:
            QString localizedName;
            QString canonicalName;
            bool isTalk;
            int ID;
    };

    inline QString WikiPageNS::GetName()
    {
        return this->localizedName;
    }

    inline QString WikiPageNS::GetCanonicalName()
    {
        return this->canonicalName;
    }

    inline bool WikiPageNS::IsTalkPage()
    {
        return this->isTalk;
    }

    inline int WikiPageNS::GetID()
    {
        return this->ID;
    }

    //! Extension info
    class HUGGLE_EX_CORE WikiSite_Ext
    {
        public:
            WikiSite_Ext(const QString &name, const QString &type, const QString &description, const QString &author, const QString &url, const QString &version);
            QString Type;
            QString Name;
            QString Description;
            QString Author;
            QString URL;
            Version MediaWikiExtVersion;
    };

    //! Site
    class HUGGLE_EX_CORE WikiSite
    {
        public:
            //! This NS is used in case we can't find a match for page
            static WikiPageNS *UnknownNS;

            WikiSite(WikiSite *w);
            WikiSite(const WikiSite &w);
            WikiSite(const QString &name, const QString &url);
            //! This will create a new instance of wikisite with most of configuration
            /*!
              \param name is a name of wiki for internal purposes
              \param url is relative url to wiki (no http prefix) which must be terminated with slash
              \param path is long path for articles, like wiki/
              \param script is short path w/
              \param https set this true if your wiki support https
              \param oauth set this true if your wiki support oauth
              \param channel irc
              \param wl whitelist
            */
            WikiSite(const QString &name, const QString &url, const QString &path, const QString &script, bool https, bool oauth, const QString &channel, const QString &wl, const QString &han, bool isrtl = false);
            ~WikiSite();
            WikiPageNS *RetrieveNSFromTitle(const QString &title);
            WikiPageNS *RetrieveNSByCanonicalName(QString CanonicalName);
            ProjectConfiguration *GetProjectConfig();
            UserConfiguration    *GetUserConfig();
            void InsertNS(WikiPageNS *Ns);
            void RemoveNS(int ns);
            void ClearNS();
            HuggleQueueFilter *CurrentFilter = nullptr;
            //! If this is true it shouldn't be possible to login to wiki without SSL, it may be needed for some WMF sites which now require SSL
            bool ForceSSL = false;
            QHash<int, WikiPageNS*> NamespaceList;
            //! Name of wiki, used by huggle only
            QString Name;
            //! URL of wiki, no http prefix must be present
            QString URL;
            HuggleFeed *Provider = nullptr;
            //! long article path (wiki/ for example on english wp)
            QString LongPath;
            //! short path
            QString ScriptPath;
            //! IRC channel of this site, if it doesn't have a channel leave it empty
            QString IRCChannel;
            //! Name of this site as known by XMLRPC server, if this is empty, alternative provider is used
            QString XmlRcsName;
            QString HANChannel;
            Version MediawikiVersion;
            QList<WikiSite_Ext> Extensions;
            ProjectConfiguration *ProjectConfig = nullptr;
            UserConfiguration    *UserConfig = nullptr;
            //! URL of whitelist, every site needs to have some, if your site doesn't have it
            //! leave it as test
            QString WhiteList;
            //! Whether the site supports the ssl
            bool SupportHttps;
            bool IsRightToLeft = false;
    };
}

#endif // WIKISITE_H
