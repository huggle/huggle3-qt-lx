//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "configuration.hpp"
#include "core.hpp"
#include "localization.hpp"
#include "syslog.hpp"
#include "gc.hpp"
#include "query.hpp"
#include "querypool.hpp"
#include "iextension.hpp"
#include "exception.hpp"

using namespace Huggle;

void iExtension::huggle__internal_SetPath(QString path)
{
    if (!this->huggle__internal_ExtensionPath.isEmpty())
        throw new Exception("Extension path is read only", BOOST_CURRENT_FUNCTION);

    this->huggle__internal_ExtensionPath = path;
}

QString iExtension::GetExtensionFullPath()
{
    return this->huggle__internal_ExtensionPath;
}

void iExtension::Init()
{
    Huggle::Core::HuggleCore = (Huggle::Core*) this->HuggleCore;
    Huggle::QueryPool::HugglePool = Huggle::Core::HuggleCore->HGQP;
    Huggle::Localizations::HuggleLocalizations = (Huggle::Localizations*) this->Localization;
    Huggle::Syslog::HuggleLogs = Huggle::Core::HuggleCore->HuggleSyslog;
    Huggle::GC::gc = Huggle::Core::HuggleCore->gc;
    Huggle::Query::NetworkManager = this->Networking;
    Huggle::Configuration::HuggleConfiguration = (Huggle::Configuration*) this->Configuration;
}

QString iExtension::GetConfig(QString key, QString dv)
{
    return hcfg->GetExtensionConfig(this->GetExtensionName(), key, dv);
}

void iExtension::SetConfig(QString key, QString value)
{
    hcfg->SetExtensionConfig(this->GetExtensionName(), key, value);
}
