//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglequeuefilter.hpp"

using namespace Huggle;

HuggleQueueFilter *HuggleQueueFilter::DefaultFilter = new HuggleQueueFilter();
QList<HuggleQueueFilter*> HuggleQueueFilter::Filters;

HuggleQueueFilter::HuggleQueueFilter()
{
    QueueName = "default";
    this->IgnoreBots = true;
    this->IgnoreWL = true;
    this->IgnoreFriends = true;
    this->IgnoreIP = false;
    this->IgnoreMinor = false;
    this->IgnoreNP = false;
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
    while (i < Configuration::HuggleConfiguration->LocalConfig_IgnorePatterns.count())
    {
        if (edit->Page->PageName.contains(Configuration::HuggleConfiguration->LocalConfig_IgnorePatterns.at(i)))
        {
            return false;
        }
        i++;
    }
    if (Configuration::HuggleConfiguration->LocalConfig_Ignores.contains(edit->Page->PageName))
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
    if (edit->NewPage && this->IgnoreNP)
    {
        return false;
    }
    if (edit->Bot && IgnoreBots)
    {
        return false;
    }
    return true;
}

bool HuggleQueueFilter::getIgnoreMinor() const
{
    return IgnoreMinor;
}

void HuggleQueueFilter::setIgnoreMinor(bool value)
{
    IgnoreMinor = value;
}

bool HuggleQueueFilter::getIgnoreUsers() const
{
    return IgnoreUsers;
}

void HuggleQueueFilter::setIgnoreUsers(bool value)
{
    IgnoreUsers = value;
}

bool HuggleQueueFilter::getIgnoreWL() const
{
    return IgnoreWL;
}

void HuggleQueueFilter::setIgnoreWL(bool value)
{
    IgnoreWL = value;
}

bool HuggleQueueFilter::getIgnoreIP() const
{
    return IgnoreIP;
}

void HuggleQueueFilter::setIgnoreIP(bool value)
{
    IgnoreIP = value;
}

bool HuggleQueueFilter::getIgnoreBots() const
{
    return IgnoreBots;
}

void HuggleQueueFilter::setIgnoreBots(bool value)
{
    IgnoreBots = value;
}

bool HuggleQueueFilter::getIgnoreNP() const
{
    return IgnoreNP;
}

void HuggleQueueFilter::setIgnoreNP(bool value)
{
    IgnoreNP = value;
}

bool HuggleQueueFilter::getIgnoreFriends() const
{
    return IgnoreFriends;
}

void HuggleQueueFilter::setIgnoreFriends(bool value)
{
    IgnoreFriends = value;
}








