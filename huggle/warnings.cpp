//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "warnings.hpp"
#include <QtXml>
#include "configuration.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "huggleparser.hpp"
#include "mainwindow.hpp"
#include "querypool.hpp"
#include "message.hpp"
#include "reportuser.hpp"
#include "revertquery.hpp"
#include "localization.hpp"
#include "hooks.hpp"
#include "syslog.hpp"
#include "wikisite.hpp"
#include "wikiutil.hpp"
#include "wikiuser.hpp"

using namespace Huggle;

QList<PendingWarning*> PendingWarning::PendingWarnings;

PendingWarning::PendingWarning(Message *message, QString warning, WikiEdit *edit)
{
    this->Template = warning;
    this->RelatedEdit = edit;
    // we register a unique consumer here in case that multiple warnings pointers to same
    edit->IncRef();
    this->Warning = message;
}

PendingWarning::~PendingWarning()
{
    this->RelatedEdit->DecRef();
    this->Warning->UnregisterConsumer(HUGGLECONSUMER_CORE_MESSAGE);
}

PendingWarning *Warnings::WarnUser(QString WarningType, RevertQuery *Dependency, WikiEdit *Edit, bool *Report)
{
    *Report = false;
    if (Edit == nullptr)
    {
        throw new Huggle::NullPointerException("WikiEdit *Edit", BOOST_CURRENT_FUNCTION);
    }
    if (Configuration::HuggleConfiguration->Restricted)
    {
        Generic::DeveloperError();
        return nullptr;
    }

    // check if user wasn't changed and if was, let's update the info
    Edit->User->Resync();

    if (Edit->User->GetWarningLevel() >= Edit->GetSite()->GetProjectConfig()->WarningLevel)
    {
        // we should report this user instead
        if (Edit->User->IsReported)
        {
            // the user is already reported we don't need to do anything
            return nullptr;
        }

        if (!Edit->GetSite()->GetProjectConfig()->AIV)
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

    // get a template
    Edit->User->IncrementWarningLevel();
    // we need to update the user so that new level gets propagated everywhere on interface of huggle
    Edit->User->Update();
    QString Template_ = WarningType + QString::number(Edit->User->GetWarningLevel());
    QString MessageText_ = Warnings::RetrieveTemplateToWarn(Template_, Edit->GetSite());

    if (!MessageText_.size())
    {
        Syslog::HuggleLogs->Log(_l("missing-warning",Template_));
        return nullptr;
    }

    MessageText_ = MessageText_.replace("$2", Edit->GetFullUrl()).replace("$1", Edit->Page->PageName);
    /// \todo This needs to be localized because it's in message, but it must be in config, not localization
    QString Summary_ = "Message re " + Edit->Page->PageName;

    switch (Edit->User->GetWarningLevel())
    {
        case 1:
            Summary_ = Edit->GetSite()->GetProjectConfig()->WarnSummary;
            break;
        case 2:
            Summary_ = Edit->GetSite()->GetProjectConfig()->WarnSummary2;
            break;
        case 3:
            Summary_ = Edit->GetSite()->GetProjectConfig()->WarnSummary3;
            break;
        case 4:
            Summary_ = Edit->GetSite()->GetProjectConfig()->WarnSummary4;
            break;
    }

    Summary_ = Summary_.replace("$1", Edit->Page->PageName);
    /// \todo This really needs to be localized somehow (in config only)
    QString HeadingText_ = "Your edits to " + Edit->Page->PageName;
    if (Edit->GetSite()->GetProjectConfig()->MessageHeadings == HeadingsStandard)
    {
        QDateTime d = Edit->GetSite()->GetProjectConfig()->ServerTime();
        HeadingText_ = WikiUtil::MonthText(d.date().month(), Edit->GetSite()) + " " + QString::number(d.date().year());
    } else if (Edit->GetSite()->GetProjectConfig()->MessageHeadings == HeadingsNone)
    {
        HeadingText_ = "";
    }
    MessageText_ = Warnings::UpdateSharedIPTemplate(Edit->User, MessageText_, Edit->GetSite());
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
                    Syslog::HuggleLogs->ErrorLog("Unable to retrieve a new version of talk page for user " + warning->RelatedEdit->User->Username
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
                                                     + warning->RelatedEdit->User->Username
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
                            Huggle::Syslog::HuggleLogs->ErrorLog("Talk page timestamp of " + warning->RelatedEdit->User->Username +
                                                                 " couldn't be retrieved, mediawiki returned no data for it");
                            PendingWarning::PendingWarnings.removeAt(x);
                            delete warning;
                            continue;
                        } else
                        {
                            TPRevBaseTime = e.attribute("timestamp");
                        }
                        warning->RelatedEdit->User->TalkPage_SetContents(e.text());
                    } else
                    {
                        // there was some error, which suck, we print it to console and delete this warning, there is a little point
                        // in doing anything else to fix it.
                        Syslog::HuggleLogs->ErrorLog("Unable to retrieve a new version of talk page for user "
                                                     + warning->RelatedEdit->User->Username
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
                    Syslog::HuggleLogs->ErrorLog("Unable to retrieve a new version of talk page for user " + warning->RelatedEdit->User->Username
                                        + " the warning will not be delivered to this user, check debug logs for more");
                    Syslog::HuggleLogs->DebugLog(warning->Query->Result->Data);
                    PendingWarning::PendingWarnings.removeAt(x);
                    delete warning;
                    continue;
                }

                // so we now have the new talk page content so we need to reclassify the user
                warning->RelatedEdit->User->ParseTP(QDate::currentDate());
                warning->RelatedEdit->User->Update(true);

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
            Syslog::HuggleLogs->DebugLog("Failed to deliver message to " + warning->Warning->User->Username);
            // we need to dec. the warning level of that user because we didn't deliver the warning message
            warning->RelatedEdit->User->DecrementWarningLevel();
            warning->RelatedEdit->User->Update();
            // check if the warning wasn't delivered because someone edited the page
            if (warning->Warning->Error == Huggle::MessageError_Obsolete || warning->Warning->Error == Huggle::MessageError_ArticleExist)
            {
                Syslog::HuggleLogs->DebugLog("Someone changed the content of " + warning->Warning->User->Username + " reparsing it now");
                // we need to fetch the talk page again and later we need to issue new warning
                if (warning->Query != nullptr)
                {
                    Syslog::HuggleLogs->DebugLog("Possible memory leak in MainWindow::ResendWarning: warning->Query != nullptr");
                }
                warning->Query = new Huggle::ApiQuery(ActionQuery, warning->RelatedEdit->GetSite());
                warning->Query->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") +
                                             "&titles=" + QUrl::toPercentEncoding(warning->Warning->User->GetTalk());
                QueryPool::HugglePool->AppendQuery(warning->Query);
                warning->Query->Target = _l("main-user-retrieving-tp", warning->Warning->User->Username);
                warning->Query->Process();
            } else if (warning->Warning->Error == Huggle::MessageError_Expired)
            {
                Syslog::HuggleLogs->DebugLog("Expired " + warning->Warning->User->Username + " reparsing it now");
                // we need to fetch the talk page again and later we need to issue new warning
                warning->Query = new Huggle::ApiQuery(ActionQuery, warning->RelatedEdit->GetSite());
                warning->Query->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") +
                                             "&titles=" + QUrl::toPercentEncoding(warning->Warning->User->GetTalk());
                QueryPool::HugglePool->AppendQuery(warning->Query);
                warning->Query->Target = _l("main-user-retrieving-tp", warning->Warning->User->Username);
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

    bool instant = false;

    if (Level == 0)
    {
        Level = 4;
        instant = true;
    }

    if (instant && !Edit->GetSite()->GetProjectConfig()->InstantWarnings)
    {
        Syslog::HuggleLogs->ErrorLog(_l("no-instant", Edit->GetSite()->Name, Edit->User->UnderscorelessUsername()));
        return;
    }

    if (Edit == nullptr)
        return;

    QString __template = "warning" + QString::number(Level);
    QString MessageText_ = Warnings::RetrieveTemplateToWarn(__template, Edit->GetSite(), instant);

    if (!MessageText_.size())
    {
        // this is very rare error no need to translate it
        Syslog::HuggleLogs->Log("There is no such warning template " + __template);
        return;
    }

    MessageText_ = MessageText_.replace("$2", Edit->GetFullUrl()).replace("$1", Edit->Page->PageName);
    QString MessageTitle_ = Configuration::GenerateSuffix("Message re", Edit->GetSite()->GetProjectConfig());

    switch (Level)
    {
        case 1:
            MessageTitle_ = Edit->GetSite()->GetProjectConfig()->WarnSummary;
            break;
        case 2:
            MessageTitle_ = Edit->GetSite()->GetProjectConfig()->WarnSummary2;
            break;
        case 3:
            MessageTitle_ = Edit->GetSite()->GetProjectConfig()->WarnSummary3;
            break;
        case 4:
            MessageTitle_ = Edit->GetSite()->GetProjectConfig()->WarnSummary4;
            break;
    }

    MessageTitle_ = MessageTitle_.replace("$1", Edit->Page->PageName);
    QString id = "Your edits to " + Edit->Page->PageName;
    if (Configuration::HuggleConfiguration->UserConfig->EnforceMonthsAsHeaders)
    {
        QDateTime date_ = Edit->GetSite()->GetProjectConfig()->ServerTime();
        id = WikiUtil::MonthText(date_.date().month(), Edit->GetSite()) + " " + QString::number(date_.date().year());
    }
    MessageText_ = Warnings::UpdateSharedIPTemplate(Edit->User, MessageText_, Edit->GetSite());
    WikiUtil::MessageUser(Edit->User, MessageText_, id, MessageTitle_, true, nullptr, false,
                              Configuration::HuggleConfiguration->UserConfig->SectionKeep,
                              true, Edit->TPRevBaseTime);
}

QString Warnings::RetrieveTemplateToWarn(QString type, WikiSite *site, bool force)
{
    int x=0;
    QString result = "";
    while (x < site->GetProjectConfig()->WarningTemplates.count())
    {
        if (HuggleParser::GetKeyFromValue(site->GetProjectConfig()->WarningTemplates.at(x)) == type)
        {
            result = HuggleParser::GetValueFromKey(site->GetProjectConfig()->WarningTemplates.at(x));
            if (force)
                result += "im";
            return result;
        }
        x++;
    }
    return "";
}

QString Warnings::UpdateSharedIPTemplate(WikiUser *User, QString Text, WikiSite *site)
{
    if (!User->IsIP() || site->GetProjectConfig()->SharedIPTemplate.isEmpty())
    {
        return Text;
    }
    if (!User->TalkPage_ContainsSharedIPTemplate())
    {
        Text += "\n" + site->GetProjectConfig()->SharedIPTemplate + "\n";
    }
    return Text;
}
