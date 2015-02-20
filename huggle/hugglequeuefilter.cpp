//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "configuration.hpp"
#include "hugglequeuefilter.hpp"
#include "exception.hpp"
#include "wikiedit.hpp"
#include "wikiuser.hpp"
#include "wikisite.hpp"
#include "wikipage.hpp"

using namespace Huggle;

HuggleQueueFilter *HuggleQueueFilter::DefaultFilter = new HuggleQueueFilter();
QHash<WikiSite*,QList<HuggleQueueFilter*>*> HuggleQueueFilter::Filters;

void HuggleQueueFilter::Delete()
{
    foreach (QList<HuggleQueueFilter*>*list, Filters)
    {
        if (!list)
            throw new Huggle::NullPointerException("QList<HuggleQueueFilter*> list", BOOST_CURRENT_FUNCTION);
        while(list->count() > 0)
        {
            if (list->at(0) != DefaultFilter)
                delete list->at(0);
            list->removeAt(0);
        }
        delete list;
    }
    Filters.clear();
}

HuggleQueueFilter *HuggleQueueFilter::GetFilter(QString filter_name, WikiSite *site)
{
    if (!Filters.contains(site))
        throw new Huggle::Exception("Invalid key", BOOST_CURRENT_FUNCTION);

    foreach (HuggleQueueFilter *filter, *Filters[site])
    {
        if (filter->QueueName == filter_name)
            return filter;
    }
    throw new Huggle::Exception("There is no such a filter", BOOST_CURRENT_FUNCTION);
}

void HuggleQueueFilter::SetFilters()
{
    foreach (WikiSite *site, hcfg->Projects)
        site->CurrentFilter = HuggleQueueFilter::GetFilter(site->GetUserConfig()->QueueID, site);
}

HuggleQueueFilter::HuggleQueueFilter()
{
    this->QueueName = "default";
    this->Bots = HuggleQueueFilterMatchExclude;
    this->WL = HuggleQueueFilterMatchExclude;
    this->ProjectSpecific = false;
    this->Friends = HuggleQueueFilterMatchExclude;
    this->IP = HuggleQueueFilterMatchIgnore;
    this->Minor = HuggleQueueFilterMatchIgnore;
    this->NewPages = HuggleQueueFilterMatchIgnore;
    this->Self = HuggleQueueFilterMatchExclude;
    this->Reverts = HuggleQueueFilterMatchIgnore;
    this->Users = HuggleQueueFilterMatchIgnore;
    this->TalkPage = HuggleQueueFilterMatchExclude;
    this->UserSpace = HuggleQueueFilterMatchIgnore;
}

bool HuggleQueueFilter::Matches(WikiEdit *edit)
{
    if (edit == nullptr)
        throw new Huggle::NullPointerException("WikiEdit *edit", BOOST_CURRENT_FUNCTION);

    if (this->IgnoresNS(edit->Page->GetNS()->GetID()))
        return false;

    if (this->UserSpace != HuggleQueueFilterMatchIgnore)
    {
        if (this->UserSpace == HuggleQueueFilterMatchExclude && edit->Page->GetNS()->GetCanonicalName() == "User")
            return false;
        if (this->UserSpace == HuggleQueueFilterMatchRequire && edit->Page->GetNS()->GetCanonicalName() != "User")
            return false;
    }
    if (this->TalkPage != HuggleQueueFilterMatchIgnore)
    {
        if (this->TalkPage == HuggleQueueFilterMatchExclude && edit->Page->IsTalk())
            return false;
        if (this->TalkPage == HuggleQueueFilterMatchRequire && !edit->Page->IsTalk())
            return false;
    }
    int i = 0;
    while (i < Configuration::HuggleConfiguration->ProjectConfig->IgnorePatterns.count())
    {
        if (edit->Page->PageName.contains(Configuration::HuggleConfiguration->ProjectConfig->IgnorePatterns.at(i)))
            return false;
        i++;
    }
    if (Configuration::HuggleConfiguration->ProjectConfig->Ignores.contains(edit->Page->PageName))
        return false;
    if (this->WL != HuggleQueueFilterMatchIgnore)
    {
        if (this->WL == HuggleQueueFilterMatchRequire && !edit->User->IsWhitelisted())
            return false;
        if (this->WL == HuggleQueueFilterMatchExclude && edit->User->IsWhitelisted())
            return false;
    }
    if (this->Friends != HuggleQueueFilterMatchIgnore)
    {
        if (this->Friends == HuggleQueueFilterMatchExclude && edit->TrustworthEdit)
            return false;
        if (this->Friends == HuggleQueueFilterMatchRequire && edit->TrustworthEdit != true)
            return false;
    }
    if (this->Minor != HuggleQueueFilterMatchIgnore)
    {
        if (this->Minor == HuggleQueueFilterMatchExclude && edit->Minor)
            return false;
        if (this->Minor == HuggleQueueFilterMatchRequire && !edit->Minor)
            return false;
    }
    if (this->Reverts != HuggleQueueFilterMatchIgnore)
    {
        if (this->Reverts == HuggleQueueFilterMatchExclude && edit->IsRevert)
            return false;
        if (this->Reverts == HuggleQueueFilterMatchRequire && !edit->IsRevert)
            return false;
    }
    if (this->NewPages != HuggleQueueFilterMatchIgnore)
    {
        if (edit->NewPage && this->NewPages == HuggleQueueFilterMatchExclude)
            return false;
        if (this->NewPages == HuggleQueueFilterMatchRequire && !edit->NewPage)
            return false;
    }
    if (this->Bots != HuggleQueueFilterMatchIgnore)
    {
        if (edit->Bot && this->Bots == HuggleQueueFilterMatchExclude)
            return false;
        if (this->Bots == HuggleQueueFilterMatchRequire && !edit->Bot)
            return false;
    }
    if (this->Self != HuggleQueueFilterMatchIgnore)
    {
        if (this->Self == HuggleQueueFilterMatchExclude && edit->User->Username.toLower() == Configuration::HuggleConfiguration->SystemConfig_Username.toLower())
            return false;
        if (this->Self == HuggleQueueFilterMatchRequire && edit->User->Username.toLower() == Configuration::HuggleConfiguration->SystemConfig_Username.toLower())
            return false;
    }
    return true;
}

bool HuggleQueueFilter::IgnoresNS(int ns)
{
    if (this->Namespaces.contains(ns))
        return this->Namespaces[ns];

    return false;
}
