//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "warnings.hpp"
#include "configuration.hpp"
#include "mainwindow.hpp"
#include "querypool.hpp"
#include "generic.hpp"
#include "localization.hpp"
#include "hooks.hpp"
#include "syslog.hpp"
#include "wikiutil.hpp"

using namespace Huggle;

QList<PendingWarning*> PendingWarning::PendingWarnings;
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
    this->Query = nullptr;
}

PendingWarning::~PendingWarning()
{
    this->RelatedEdit->UnregisterConsumer("PendingWarning" + QString::number(gcid));
    if (this->Query != nullptr)
        this->Query->DecRef();
    this->Warning->UnregisterConsumer(HUGGLECONSUMER_CORE_MESSAGE);
}

PendingWarning *Warnings::WarnUser(QString WarningType, RevertQuery *Dependency, WikiEdit *Edit, bool *Report)
{
    *Report = false;
    if (Edit == nullptr)
    {
        throw new Huggle::Exception("WikiEdit *Edit must not be nullptr",
                                    "PendingWarning *Warnings::WarnUser(QString WarningType, RevertQuery *Dependency, "\
                                    "WikiEdit *Edit, bool *Report)");
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return nullptr;
    }

    // check if user wasn't changed and if was, let's update the info
    Edit->User->Resync();
    // get a template
    Edit->User->WarningLevel++;

    if (Edit->User->WarningLevel > Configuration::HuggleConfiguration->ProjectConfig->WarningLevel)
    {
        // we should report this user instead
        if (Edit->User->IsReported)
        {
            // the user is already reported we don't need to do anything
            return nullptr;
        }

        if (!Configuration::HuggleConfiguration->ProjectConfig->AIV)
        {
            // there is no AIV function for this wiki
            Syslog::HuggleLogs->WarningLog("This user has already reached level 4 warning and there is no AIV "\
                                           "supported on this wiki, you should block the user now");
            return nullptr;
        }

        if (Generic::ReportPreFlightCheck())
        {
            *Report = true;
        }
        return nullptr;
    }

    QString Template_ = WarningType + QString::number(Edit->User->WarningLevel);
    QString MessageText_ = Warnings::RetrieveTemplateToWarn(Template_);

    if (!MessageText_.size())
    {
        // This is very rare error, no need to localize it
        Syslog::HuggleLogs->Log("There is no such warning template " + Template_);
        return nullptr;
    }

    MessageText_ = MessageText_.replace("$2", Edit->GetFullUrl()).replace("$1", Edit->Page->PageName);
    /// \todo This needs to be localized because it's in message, but it must be in config, not localization
    QString Summary_ = "Message re " + Edit->Page->PageName;

    switch (Edit->User->WarningLevel)
    {
        case 1:
            Summary_ = Configuration::HuggleConfiguration->ProjectConfig->WarnSummary;
            break;
        case 2:
            Summary_ = Configuration::HuggleConfiguration->ProjectConfig->WarnSummary2;
            break;
        case 3:
            Summary_ = Configuration::HuggleConfiguration->ProjectConfig->WarnSummary3;
            break;
        case 4:
            Summary_ = Configuration::HuggleConfiguration->ProjectConfig->WarnSummary4;
            break;
    }

    Summary_ = Summary_.replace("$1", Edit->Page->PageName);
    /// \todo This really needs to be localized somehow (in config only)
    QString HeadingText_ = "Your edits to " + Edit->Page->PageName;
    if (Configuration::HuggleConfiguration->ProjectConfig->MessageHeadings == HeadingsStandard)
    {
        QDateTime d = Configuration::HuggleConfiguration->ServerTime();
        HeadingText_ = WikiUtil::MonthText(d.date().month()) + " " + QString::number(d.date().year());
    } else if (Configuration::HuggleConfiguration->ProjectConfig->MessageHeadings == HeadingsNone)
    {
        HeadingText_ = "";
    }

    MessageText_ = Warnings::UpdateSharedIPTemplate(Edit->User, MessageText_);
    bool CreateOnly = false;
    if (Edit->User->TalkPage_GetContents().isEmpty())
    {
        CreateOnly = true;
    }
    PendingWarning *pw = new PendingWarning(WikiUtil::MessageUser(Edit->User, MessageText_, HeadingText_, Summary_, true, Dependency, false,
                                                                  Configuration::HuggleConfiguration->UserConfig->SectionKeep, false,
                                                                  Edit->TPRevBaseTime, CreateOnly, true), WarningType, Edit);
    Hooks::OnWarning(Edit->User);
    return pw;
}

void Warnings::ResendWarnings()
{
    int x = 0;
    while (x < PendingWarning::PendingWarnings.count())
    {
        PendingWarning *warning = PendingWarning::PendingWarnings.at(x);
        if (warning->Query != nullptr)
        {
            // we are already getting talk page so we need to check if it finished here
            if (warning->Query->IsProcessed())
            {
                // this query is done so we check if it fallen to error now
                if (warning->Query->IsFailed())
                {
                    // there was some error, which suck, we print it to console and delete this warning, there is a little point
                    // in doing anything else to fix it.
                    Syslog::HuggleLogs->ErrorLog("Unable to retrieve a new version of talk page for user " + warning->Warning->user->Username
                                     + " the warning will not be delivered to this user");
                    PendingWarning::PendingWarnings.removeAt(x);
                    delete warning;
                    continue;
                }
                // we get the new talk page
                QDomDocument TalkPage_;
                TalkPage_.setContent(warning->Query->Result->Data);
                QDomNodeList revisions_ = TalkPage_.elementsByTagName("rev");
                QDomNodeList pages_ = TalkPage_.elementsByTagName("page");
                QString TPRevBaseTime = "";
                if (pages_.count() > 0)
                {
                    QDomElement e = pages_.at(0).toElement();
                    if (e.attributes().contains("missing"))
                    {
                        // the talk page which existed was probably deleted by someone
                        Syslog::HuggleLogs->ErrorLog("Unable to retrieve a new version of talk page for user "
                                                     + warning->Warning->user->Username
                                                     + " because it was deleted meanwhile, the warning will not be delivered to this user");
                        PendingWarning::PendingWarnings.removeAt(x);
                        delete warning;
                        continue;
                    }
                }
                // get last id
                if (revisions_.count() > 0)
                {
                    QDomElement e = revisions_.at(0).toElement();
                    if (e.nodeName() == "rev")
                    {
                        if (!e.attributes().contains("timestamp"))
                        {
                            Huggle::Syslog::HuggleLogs->ErrorLog("Talk page timestamp of " + warning->Warning->user->Username +
                                                                 " couldn't be retrieved, mediawiki returned no data for it");
                            PendingWarning::PendingWarnings.removeAt(x);
                            delete warning;
                            continue;
                        } else
                        {
                            TPRevBaseTime = e.attribute("timestamp");
                        }
                        warning->Warning->user->TalkPage_SetContents(e.text());
                    } else
                    {
                        // there was some error, which suck, we print it to console and delete this warning, there is a little point
                        // in doing anything else to fix it.
                        Syslog::HuggleLogs->ErrorLog("Unable to retrieve a new version of talk page for user "
                                                     + warning->Warning->user->Username
                                                     + " the warning will not be delivered to this user, check debug logs for more");
                        Syslog::HuggleLogs->DebugLog(warning->Query->Result->Data);
                        PendingWarning::PendingWarnings.removeAt(x);
                        delete warning;
                        continue;
                    }
                } else
                {
                    // there was some error, which suck, we print it to console and delete this warning, there is a little point
                    // in doing anything else to fix it.
                    Syslog::HuggleLogs->ErrorLog("Unable to retrieve a new version of talk page for user " + warning->Warning->user->Username
                                     + " the warning will not be delivered to this user, check debug logs for more");
                    Syslog::HuggleLogs->DebugLog(warning->Query->Result->Data);
                    PendingWarning::PendingWarnings.removeAt(x);
                    delete warning;
                    continue;
                }

                // so we now have the new talk page content so we need to reclassify the user
                warning->Warning->user->ParseTP(QDate::currentDate());
                warning->Warning->user->Update(true);

                // now when we have the new level of warning we can try to send a new warning and hope that talk page wasn't
                // changed meanwhile again lol :D
                warning->RelatedEdit->TPRevBaseTime = TPRevBaseTime;
                bool Report_;
                PendingWarning *ptr_warning_ = Warnings::WarnUser(warning->Template, nullptr, warning->RelatedEdit, &Report_);
                if (Report_)
                    MainWindow::HuggleMain->DisplayReportUserWindow(warning->RelatedEdit->User);

                if (ptr_warning_ != nullptr)
                    PendingWarning::PendingWarnings.append(ptr_warning_);

                // we can delete this warning now because we created another one
                PendingWarning::PendingWarnings.removeAt(x);
                delete warning;
                continue;
            }
            // in case that it isn't processed yet we can continue on next warning
            x++;
            continue;
        }

        if (warning->Warning->IsFinished())
        {
            if (!warning->Warning->IsFailed())
            {
                // we no longer need to care about this one
                PendingWarning::PendingWarnings.removeAt(x);
                delete warning;
                continue;
            }
            Syslog::HuggleLogs->DebugLog("Failed to deliver message to " + warning->Warning->user->Username);
            // we need to decrease the warning level of that user because we didn't deliver the warning message
            if (warning->RelatedEdit->User->WarningLevel > 0)
            {
                warning->RelatedEdit->User->WarningLevel--;
                warning->RelatedEdit->User->Update();
            }
            // check if the warning wasn't delivered because someone edited the page
            if (warning->Warning->Error == Huggle::MessageError_Obsolete || warning->Warning->Error == Huggle::MessageError_ArticleExist)
            {
                Syslog::HuggleLogs->DebugLog("Someone changed the content of " + warning->Warning->user->Username + " reparsing it now");
                // we need to fetch the talk page again and later we need to issue new warning
                if (warning->Query != nullptr)
                {
                    Syslog::HuggleLogs->DebugLog("Possible memory leak in MainWindow::ResendWarning: warning->Query != nullptr");
                }
                warning->Query = new Huggle::ApiQuery();
                warning->Query->SetAction(ActionQuery);
                warning->Query->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") +
                                             "&titles=" + QUrl::toPercentEncoding(warning->Warning->user->GetTalk());
                warning->Query->IncRef();
                QueryPool::HugglePool->AppendQuery(warning->Query);
                //! \todo LOCALIZE ME
                warning->Query->Target = "Retrieving tp of " + warning->Warning->user->GetTalk();
                warning->Query->Process();
            } else if (warning->Warning->Error == Huggle::MessageError_Expired)
            {
                Syslog::HuggleLogs->DebugLog("Expired " + warning->Warning->user->Username + " reparsing it now");
                // we need to fetch the talk page again and later we need to issue new warning
                if (warning->Query != nullptr)
                {
                    Syslog::HuggleLogs->DebugLog("Possible memory leak in MainWindow::ResendWarning: warning->Query != NULL");
                }
                warning->Query = new Huggle::ApiQuery();
                warning->Query->SetAction(ActionQuery);
                warning->Query->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") +
                                             "&titles=" + QUrl::toPercentEncoding(warning->Warning->user->GetTalk());
                warning->Query->IncRef();
                QueryPool::HugglePool->AppendQuery(warning->Query);
                //! \todo LOCALIZE ME
                warning->Query->Target = "Retrieving tp of " + warning->Warning->user->GetTalk();
                warning->Query->Process();
            } else
            {
                PendingWarning::PendingWarnings.removeAt(x);
                delete warning;
                continue;
            }
        }
        x++;
        continue;
    }
}

void Warnings::ForceWarn(int Level, WikiEdit *Edit)
{
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return;
    }

    if (Edit == nullptr)
        return;

    QString __template = "warning" + QString::number(Level);
    QString MessageText_ = Warnings::RetrieveTemplateToWarn(__template);

    if (!MessageText_.size())
    {
        // this is very rare error no need to translate it
        Syslog::HuggleLogs->Log("There is no such warning template " + __template);
        return;
    }

    MessageText_ = MessageText_.replace("$2", Edit->GetFullUrl()).replace("$1", Edit->Page->PageName);
    QString MessageTitle_ = "Message re " + Configuration::HuggleConfiguration->ProjectConfig->EditSuffixOfHuggle;

    switch (Level)
    {
        case 1:
            MessageTitle_ = Configuration::HuggleConfiguration->ProjectConfig->WarnSummary;
            break;
        case 2:
            MessageTitle_ = Configuration::HuggleConfiguration->ProjectConfig->WarnSummary2;
            break;
        case 3:
            MessageTitle_ = Configuration::HuggleConfiguration->ProjectConfig->WarnSummary3;
            break;
        case 4:
            MessageTitle_ = Configuration::HuggleConfiguration->ProjectConfig->WarnSummary4;
            break;
    }

    MessageTitle_ = MessageTitle_.replace("$1", Edit->Page->PageName);
    QString id = "Your edits to " + Edit->Page->PageName;
    if (Configuration::HuggleConfiguration->UserConfig->EnforceMonthsAsHeaders)
    {
        QDateTime date_ = Configuration::HuggleConfiguration->ServerTime();
        id = WikiUtil::MonthText(date_.date().month()) + " " + QString::number(date_.date().year());
    }
    MessageText_ = Warnings::UpdateSharedIPTemplate(Edit->User, MessageText_);
    WikiUtil::MessageUser(Edit->User, MessageText_, id, MessageTitle_, true, nullptr, false,
                              Configuration::HuggleConfiguration->UserConfig->SectionKeep,
                              true, Edit->TPRevBaseTime);
}

QString Warnings::RetrieveTemplateToWarn(QString type)
{
    int x=0;
    while (x < Configuration::HuggleConfiguration->ProjectConfig->WarningTemplates.count())
    {
        if (HuggleParser::GetKeyFromValue(Configuration::HuggleConfiguration->ProjectConfig->WarningTemplates.at(x)) == type)
        {
            return HuggleParser::GetValueFromKey(Configuration::HuggleConfiguration->ProjectConfig->WarningTemplates.at(x));
        }
        x++;
    }
    return "";
}

QString Warnings::UpdateSharedIPTemplate(WikiUser *User, QString Text)
{
    if (!User->IsIP() || Configuration::HuggleConfiguration->ProjectConfig->SharedIPTemplate.isEmpty())
    {
        return Text;
    }
    if (!User->TalkPage_ContainsSharedIPTemplate())
    {
        Text += "\n" + Configuration::HuggleConfiguration->ProjectConfig->SharedIPTemplate + "\n";
    }
    return Text;
}
