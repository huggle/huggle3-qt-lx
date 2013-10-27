//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglequeuefilter.h"

using namespace Huggle;

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
    if (edit->Page->IsTalk() && this->IgnoreTalk)
    {
        return false;
    }
    int i = 0;
    while (i < Configuration::LocalConfig_IgnorePatterns.count())
    {
        if (edit->Page->PageName.contains(Configuration::LocalConfig_IgnorePatterns.at(i)))
        {
            return false;
        }
        i++;
    }
    if (Configuration::LocalConfig_Ignores.contains(edit->Page->PageName))
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

