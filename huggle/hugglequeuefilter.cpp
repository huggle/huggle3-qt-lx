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
#include "wikipage.hpp"

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
    this->Ignore_UserSpace = false;
}

bool HuggleQueueFilter::Matches(WikiEdit *edit)
{
    if (edit == nullptr)
        throw new Huggle::Exception("WikiEdit *edit must not be NULL in this context", "bool HuggleQueueFilter::Matches(WikiEdit *edit)");

    if (this->Ignore_UserSpace && edit->Page->GetNS()->GetCanonicalName() == "User")
        return false;
    if (edit->Page->IsTalk() && this->IgnoreTalk)
        return false;
    int i = 0;
    while (i < Configuration::HuggleConfiguration->ProjectConfig->IgnorePatterns.count())
    {
        if (edit->Page->PageName.contains(Configuration::HuggleConfiguration->ProjectConfig->IgnorePatterns.at(i)))
        {
            return false;
        }
        i++;
    }
    if (Configuration::HuggleConfiguration->ProjectConfig->Ignores.contains(edit->Page->PageName))
        return false;
    if (edit->User->IsWhitelisted() && this->IgnoreWL)
        return false;
    if (edit->TrustworthEdit && this->IgnoreFriends)
        return false;
    if (edit->Minor && this->IgnoreMinor)
        return false;
    if (edit->IsRevert && this->IgnoreReverts)
        return false;
    if (edit->NewPage && this->IgnoreNP)
        return false;
    if (edit->Bot && this->IgnoreBots)
        return false;
    if (this->IgnoreSelf && edit->User->Username.toLower() == Configuration::HuggleConfiguration->SystemConfig_Username.toLower())
        return false;
    return true;
}
