//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglequeuefilter.hpp"
#include "exception.hpp"

using namespace Huggle;

HuggleQueueFilter *HuggleQueueFilter::DefaultFilter = new HuggleQueueFilter();
QList<HuggleQueueFilter*> HuggleQueueFilter::Filters;

HuggleQueueFilter::HuggleQueueFilter()
{
    this->QueueName = "default";
    this->IgnoreBots = true;
    this->IgnoreWL = true;
    this->ProjectSpecific = false;
    this->IgnoreFriends = true;
    this->IgnoreIP = false;
    this->IgnoreMinor = false;
    this->IgnoreNP = false;
    this->IgnoreSelf = true;
    this->IgnoreReverts = false;
    this->IgnoreUsers = false;
    this->IgnoreTalk = true;
}

bool HuggleQueueFilter::Matches(WikiEdit *edit)
{
    if (edit == NULL)
    {
        throw new Exception("WikiEdit *edit must not be NULL in this context", "bool HuggleQueueFilter::Matches(WikiEdit *edit)");
    }
    if (edit->Page->IsTalk() && this->IgnoreTalk)
    {
        return false;
    }
    int i = 0;
    while (i < Configuration::HuggleConfiguration->ProjectConfig_IgnorePatterns.count())
    {
        if (edit->Page->PageName.contains(Configuration::HuggleConfiguration->ProjectConfig_IgnorePatterns.at(i)))
        {
            return false;
        }
        i++;
    }
    if (Configuration::HuggleConfiguration->ProjectConfig_Ignores.contains(edit->Page->PageName))
    {
        return false;
    }
    if (edit->User->IsWhitelisted() && this->IgnoreWL)
    {
        return false;
    }
    if (edit->TrustworthEdit && this->IgnoreFriends)
    {
        return false;
    }
    if (edit->Minor && this->IgnoreMinor)
    {
        return false;
    }
    if (edit->IsRevert && this->IgnoreReverts)
    {
        return false;
    }
    if (edit->NewPage && this->IgnoreNP)
    {
        return false;
    }
    if (edit->Bot && this->IgnoreBots)
    {
        return false;
    }
    if (this->IgnoreSelf)
    {
        if (edit->User->Username.toLower() == Configuration::HuggleConfiguration->SystemConfig_Username.toLower())
        {
            return false;
        }
    }
    return true;
}

bool HuggleQueueFilter::getIgnoreMinor() const
{
    return this->IgnoreMinor;
}

void HuggleQueueFilter::setIgnoreMinor(bool value)
{
    this->IgnoreMinor = value;
}

bool HuggleQueueFilter::getIgnoreUsers() const
{
    return this->IgnoreUsers;
}

void HuggleQueueFilter::setIgnoreUsers(bool value)
{
    this->IgnoreUsers = value;
}

bool HuggleQueueFilter::getIgnoreWL() const
{
    return this->IgnoreWL;
}

void HuggleQueueFilter::setIgnoreWL(bool value)
{
    this->IgnoreWL = value;
}

bool HuggleQueueFilter::getIgnoreIP() const
{
    return this->IgnoreIP;
}

void HuggleQueueFilter::setIgnoreIP(bool value)
{
    this->IgnoreIP = value;
}

bool HuggleQueueFilter::getIgnoreBots() const
{
    return this->IgnoreBots;
}

void HuggleQueueFilter::setIgnoreBots(bool value)
{
    this->IgnoreBots = value;
}

bool HuggleQueueFilter::getIgnoreNP() const
{
    return this->IgnoreNP;
}

void HuggleQueueFilter::setIgnoreNP(bool value)
{
    this->IgnoreNP = value;
}

bool HuggleQueueFilter::getIgnoreFriends() const
{
    return this->IgnoreFriends;
}

bool HuggleQueueFilter::getIgnoreReverts() const
{
    return this->IgnoreReverts;
}

void HuggleQueueFilter::setIgnoreReverts(bool value)
{
    this->IgnoreReverts = value;
}

void HuggleQueueFilter::setIgnoreFriends(bool value)
{
    this->IgnoreFriends = value;
}

bool HuggleQueueFilter::getIgnoreSelf() const
{
    return this->IgnoreSelf;
}

void HuggleQueueFilter::setIgnoreSelf(bool value)
{
    this->IgnoreSelf = value;
}

bool HuggleQueueFilter::IsDefault() const
{
    return this == HuggleQueueFilter::DefaultFilter;
}

bool HuggleQueueFilter::IsChangeable() const
{
    return !this->IsDefault() && !this->ProjectSpecific;
}
