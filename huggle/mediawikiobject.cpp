//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "mediawikiobject.hpp"
#include "configuration.hpp"
#include "wikisite.hpp"
using namespace Huggle;

MediaWikiObject::MediaWikiObject()
{
    this->Site = nullptr;
}

MediaWikiObject::MediaWikiObject(MediaWikiObject *m)
{
    this->Site = m->Site;
}

MediaWikiObject::MediaWikiObject(const MediaWikiObject &m)
{
    this->Site = m.Site;
}

MediaWikiObject::~MediaWikiObject()
{

}

WikiSite *MediaWikiObject::GetSite()
{
    if (this->Site == nullptr)
        return Configuration::HuggleConfiguration->Project;
    return this->Site;
}
