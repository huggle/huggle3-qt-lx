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
    class Query;
    class Version;
    class WikiEdit;
    class WikiSite;
    class WikiPage;
    class WikiPageNS;
    class WikiUser;
    namespace JSMarshallingHelper
    {
        HUGGLE_EX_CORE QJSValue FromSite(WikiSite *site, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromUser(WikiUser *user, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromEdit(WikiEdit *edit, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromPage(WikiPage *page, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromVersion(Version *version, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromNS(WikiPageNS *ns, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromQuery(Query *query, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromApiQueryResult(ApiQueryResult *res, QJSEngine *engine);
        HUGGLE_EX_CORE QJSValue FromApiQuery(ApiQuery *query, QJSEngine *engine);
    }
}

#endif // JSMARSHALLINGHELPER_HPP
