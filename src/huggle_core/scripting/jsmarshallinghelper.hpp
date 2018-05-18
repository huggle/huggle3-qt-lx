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
    class Version;
    class WikiEdit;
    class WikiSite;
    class WikiPage;
    class WikiPageNS;
    class WikiUser;
    namespace JSMarshallingHelper
    {
        QJSValue FromSite(WikiSite *site, QJSEngine *engine);
        QJSValue FromUser(WikiUser *user, QJSEngine *engine);
        QJSValue FromEdit(WikiEdit *edit, QJSEngine *engine);
        QJSValue FromPage(WikiPage *page, QJSEngine *engine);
        QJSValue FromVersion(Version *version, QJSEngine *engine);
        QJSValue FromNS(WikiPageNS *ns, QJSEngine *engine);
    }
}

#endif // JSMARSHALLINGHELPER_HPP
