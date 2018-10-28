//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef JSMARSHALLINGHELPER_HPP
#define JSMARSHALLINGHELPER_HPP

#include "../definitions.hpp"
#include <QJSEngine>

namespace Huggle
{
    class ApiQuery;
    class ApiQueryResult;
    class EditQuery;
    class Version;
    class UserConfiguration;
    class ProjectConfiguration;
    class Query;
    class WikiEdit;
    class WikiSite;
    class WikiPage;
    class WikiPageNS;
    class WikiUser;
    namespace JSMarshallingHelper
    {
        HUGGLE_EX_CORE QJSValue FromQStringList(QStringList string_list, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromSite(WikiSite *site, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromUser(WikiUser *user, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromEdit(WikiEdit *edit, QJSEngine *engine, int pool_id = -1);
        HUGGLE_EX_CORE QJSValue FromPage(WikiPage *page, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromVersion(Version *version, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromNS(WikiPageNS *ns, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromQuery(Query *query, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromApiQueryResult(ApiQueryResult *res, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromApiQuery(ApiQuery *query, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromEditQuery(EditQuery *eq, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromSiteProjectConfig(ProjectConfiguration *config, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromSiteUserConfig(UserConfiguration *config, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromQVariantHash(QHash<QString, QVariant> hash, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromVariant(QVariant variant);
    }
}

#endif // JSMARSHALLINGHELPER_HPP
