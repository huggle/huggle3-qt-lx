//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "warnings.hpp"

using namespace Huggle;

int PendingWarning::GCID = 0;

PendingWarning::PendingWarning(Message *message, QString warning, WikiEdit *edit)
{
    this->gcid = GCID;
    GCID++;
    this->Template = warning;
    this->RelatedEdit = edit;
    // we register a unique consumer here in case that multiple warnings pointer to same
    edit->RegisterConsumer("PendingWarning" + QString::number(gcid));
    this->Warning = message;
    this->Query = NULL;
}

PendingWarning::~PendingWarning()
{
    this->RelatedEdit->UnregisterConsumer("PendingWarning" + QString::number(gcid));
    if (this->Query != NULL)
    {
        this->Query->UnregisterConsumer(HUGGLECONSUMER_MAINFORM);
    }
    this->Warning->UnregisterConsumer(HUGGLECONSUMER_CORE_MESSAGE);
}

PendingWarning *Warnings::WarnUser(QString WarningType, RevertQuery *Dependency, WikiEdit *Edit, bool *Report)
{
    *Report = false;
    if (Edit == NULL)
    {
        throw new Huggle::Exception("WikiEdit *Edit must not be NULL",
                                    "PendingWarning *Warnings::WarnUser(QString WarningType, RevertQuery *Dependency, "\
                                    "WikiEdit *Edit, bool *Report)");
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return NULL;
    }

    // check if user wasn't changed and if was, let's update the info
    Edit->User->Resync();
    // get a template
    Edit->User->WarningLevel++;

    if (Edit->User->WarningLevel > 4)
    {
        // we should report this user instead
        if (Edit->User->IsReported)
        {
            // the user is already reported we don't need to do anything
            return NULL;
        }

        if (!Configuration::HuggleConfiguration->LocalConfig_AIV)
        {
            // there is no AIV function for this wiki
            Syslog::HuggleLogs->WarningLog("This user has already reached level 4 warning and there is no AIV "\
                                           "supported on this wiki, you should block the user now");
            return NULL;
        }

        if (Core::HuggleCore->ReportPreFlightCheck())
        {
            *Report = true;
        }
        return NULL;
    }

    QString Template_ = WarningType + QString::number(Edit->User->WarningLevel);
    QString MessageText_ = Core::HuggleCore->RetrieveTemplateToWarn(Template_);

    if (MessageText_ == "")
    {
        // This is very rare error, no need to localize it
        Syslog::HuggleLogs->Log("There is no such warning template " + Template_);
        return NULL;
    }

    MessageText_ = MessageText_.replace("$2", Edit->GetFullUrl()).replace("$1", Edit->Page->PageName);
    /// \todo This needs to be localized because it's in message, but it must be in config, not localization
    QString Summary_ = "Message re " + Edit->Page->PageName;

    switch (Edit->User->WarningLevel)
    {
        case 1:
            Summary_ = Configuration::HuggleConfiguration->LocalConfig_WarnSummary;
            break;
        case 2:
            Summary_ = Configuration::HuggleConfiguration->LocalConfig_WarnSummary2;
            break;
        case 3:
            Summary_ = Configuration::HuggleConfiguration->LocalConfig_WarnSummary3;
            break;
        case 4:
            Summary_ = Configuration::HuggleConfiguration->LocalConfig_WarnSummary4;
            break;
    }

    Summary_ = Summary_.replace("$1", Edit->Page->PageName);
    /// \todo This really needs to be localized somehow (in config only)
    QString HeadingText_ = "Your edits to " + Edit->Page->PageName;
    if (Configuration::HuggleConfiguration->LocalConfig_Headings == HeadingsStandard)
    {
        QDateTime d = QDateTime::currentDateTime();
        HeadingText_ = Core::HuggleCore->MonthText(d.date().month()) + " " + QString::number(d.date().year());
    } else if (Configuration::HuggleConfiguration->LocalConfig_Headings == HeadingsNone)
    {
        HeadingText_ = "";
    }

    MessageText_ = Warnings::UpdateSharedIPTemplate(Edit->User, MessageText_);
    PendingWarning *PendingWarning_ = new PendingWarning(Core::HuggleCore->MessageUser(Edit->User, MessageText_, HeadingText_,
                                             Summary_, true, Dependency, false,
                                             Configuration::HuggleConfiguration->UserConfig_SectionKeep,
                                             false, Edit->TPRevBaseTime), WarningType, Edit);
    Hooks::OnWarning(Edit->User);
    return PendingWarning_;
}

void Warnings::ForceWarn(int Level, WikiEdit *Edit)
{
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Core::HuggleCore->DeveloperError();
        return;
    }

    if (Edit == NULL)
    {
        return;
    }

    QString __template = "warning" + QString::number(Level);

    QString MessageText_ = Core::HuggleCore->RetrieveTemplateToWarn(__template);

    if (MessageText_ == "")
    {
        // this is very rare error no need to translate it
        Syslog::HuggleLogs->Log("There is no such warning template " + __template);
        return;
    }

    MessageText_ = MessageText_.replace("$2", Edit->GetFullUrl()).replace("$1", Edit->Page->PageName);

    QString MessageTitle_ = "Message re " + Configuration::HuggleConfiguration->LocalConfig_EditSuffixOfHuggle;

    switch (Level)
    {
        case 1:
            MessageTitle_ = Configuration::HuggleConfiguration->LocalConfig_WarnSummary;
            break;
        case 2:
            MessageTitle_ = Configuration::HuggleConfiguration->LocalConfig_WarnSummary2;
            break;
        case 3:
            MessageTitle_ = Configuration::HuggleConfiguration->LocalConfig_WarnSummary3;
            break;
        case 4:
            MessageTitle_ = Configuration::HuggleConfiguration->LocalConfig_WarnSummary4;
            break;
    }

    MessageTitle_ = MessageTitle_.replace("$1", Edit->Page->PageName);
    QString id = "Your edits to " + Edit->Page->PageName;
    if (Configuration::HuggleConfiguration->UserConfig_EnforceMonthsAsHeaders)
    {
        QDateTime date_ = QDateTime::currentDateTime();
        id = Core::HuggleCore->MonthText(date_.date().month()) + " " + QString::number(date_.date().year());
    }
    MessageText_ = Warnings::UpdateSharedIPTemplate(Edit->User, MessageText_);
    Core::HuggleCore->MessageUser(Edit->User, MessageText_, id, MessageTitle_, true, NULL, false,
                               Configuration::HuggleConfiguration->UserConfig_SectionKeep, true, Edit->TPRevBaseTime);
}

QString Warnings::UpdateSharedIPTemplate(WikiUser *User, QString Text)
{
    if (!User->IsIP())
    {
        return Text;
    }

    if (Configuration::HuggleConfiguration->LocalConfig_SharedIPTemplate == "")
    {
        return Text;
    }

    if (!User->TalkPage_ContainsSharedIPTemplate())
    {
        Text += "\n" + Configuration::HuggleConfiguration->LocalConfig_SharedIPTemplate + "\n";
    }
    return Text;
}
