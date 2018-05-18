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
#include "hooks.hpp"
#include "querypool.hpp"
#include "message.hpp"
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

// Fixes https://phabricator.wikimedia.org/T170449
QString sanitize_page_name(QString page_name)
{
    if (page_name.startsWith("/"))
    {
        HUGGLE_LOG("meh");
        return ":" + page_name;
    }
    return page_name;
}

PendingWarning *Warnings::WarnUser(QString warning_type, RevertQuery *dependency, WikiEdit *edit, bool *report)
{
    *report = false;
    if (edit == nullptr)
    {
        throw new Huggle::NullPointerException("WikiEdit *Edit", BOOST_CURRENT_FUNCTION);
    }
    if (Configuration::HuggleConfiguration->DeveloperMode)
    {
        Generic::DeveloperError();
        return nullptr;
    }

    // check if user wasn't changed and if was, let's update the info
    edit->User->Resync();

    if (edit->User->GetWarningLevel() >= edit->GetSite()->GetProjectConfig()->WarningLevel)
    {
        // we should report this user instead
        if (edit->User->IsReported)
        {
            // the user is already reported we don't need to do anything
            return nullptr;
        }

        if (!edit->GetSite()->GetProjectConfig()->AIV)
        {
            // there is no AIV function for this wiki
            Syslog::HuggleLogs->WarningLog("This user has already reached level 4 warning and there is no AIV "\
                                           "supported on this wiki, you should block the user now");
            return nullptr;
        }

        if (hcfg->UserConfig->AutomaticReports || Generic::ReportPreFlightCheck())
        {
            *report = true;
        }
        return nullptr;
    }

    // get a template
    edit->User->IncrementWarningLevel();
    // we need to update the user so that new level gets propagated everywhere on interface of huggle
    edit->User->Update();
    QString Template_ = warning_type + QString::number(edit->User->GetWarningLevel());
    QString MessageText_ = Warnings::RetrieveTemplateToWarn(Template_, edit->GetSite());

    if (!MessageText_.size())
    {
        Syslog::HuggleLogs->Log(_l("missing-warning",Template_));
        return nullptr;
    }

    MessageText_ = MessageText_.replace("$2", edit->GetFullUrl()).replace("$1", edit->Page->PageName);
    QString Summary_;
    if (!edit->GetSite()->GetProjectConfig()->WarningSummaries.contains(edit->User->GetWarningLevel()))
    {
        Summary_ = edit->GetSite()->GetProjectConfig()->WarningSummaries[1];
    } else
    {
        Summary_ = edit->GetSite()->GetProjectConfig()->WarningSummaries[edit->User->GetWarningLevel()];
    }
    Summary_ = Summary_.replace("$1", sanitize_page_name(edit->Page->PageName));
    QString HeadingText_ = edit->GetSite()->GetProjectConfig()->TemplateHeader;
    HeadingText_ = HeadingText_.replace("$1", sanitize_page_name(edit->Page->PageName));
    if (edit->GetSite()->GetProjectConfig()->MessageHeadings == HeadingsStandard)
    {
        QDateTime d = edit->GetSite()->GetProjectConfig()->ServerTime();
        HeadingText_ = WikiUtil::MonthText(d.date().month(), edit->GetSite()) + " " + QString::number(d.date().year());
    } else if (edit->GetSite()->GetProjectConfig()->MessageHeadings == HeadingsNone)
    {
        HeadingText_ = "";
    }
    MessageText_ = Warnings::UpdateSharedIPTemplate(edit->User, MessageText_, edit->GetSite());
    bool CreateOnly = false;
    if (edit->User->TalkPage_GetContents().isEmpty())
    {
        CreateOnly = true;
    }
    if (hcfg->UserConfig->AutomaticallyWatchlistWarnedUsers)
        WikiUtil::Watchlist(edit->User->GetTalkPage());
    PendingWarning *pw = new PendingWarning(WikiUtil::MessageUser(edit->User, MessageText_, HeadingText_, Summary_, true, dependency, false,
                                                                  Configuration::HuggleConfiguration->UserConfig->SectionKeep, false,
                                                                  edit->TPRevBaseTime, CreateOnly, true), warning_type, edit);
    Hooks::OnWarning(edit->User);
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
                {
                    if (hcfg->UserConfig->AutomaticReports)
                    {
                        Hooks::SilentReport(warning->RelatedEdit->User);
                    }
                    else
                    {
                        Hooks::ReportUser(warning->RelatedEdit->User);
                    }
                }

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
                HUGGLE_QP_APPEND(warning->Query);
                warning->Query->Target = _l("main-user-retrieving-tp", warning->Warning->User->Username);
                warning->Query->Process();
            } else if (warning->Warning->Error == Huggle::MessageError_Expired)
            {
                Syslog::HuggleLogs->DebugLog("Expired " + warning->Warning->User->Username + " reparsing it now");
                // we need to fetch the talk page again and later we need to issue new warning
                warning->Query = new Huggle::ApiQuery(ActionQuery, warning->RelatedEdit->GetSite());
                warning->Query->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") +
                                             "&titles=" + QUrl::toPercentEncoding(warning->Warning->User->GetTalk());
                HUGGLE_QP_APPEND(warning->Query);
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

void Warnings::ForceWarn(int level, WikiEdit *edit)
{
    if (Configuration::HuggleConfiguration->DeveloperMode)
    {
        Generic::DeveloperError();
        return;
    }

    bool instant = false;

    if (level == 0)
    {
        level = 4;
        instant = true;
    }

    if (instant && !edit->GetSite()->GetProjectConfig()->InstantWarnings)
    {
        Syslog::HuggleLogs->ErrorLog(_l("no-instant", edit->GetSite()->Name, edit->User->UnderscorelessUsername()));
        return;
    }

    if (edit == nullptr)
        return;

    QString _template = edit->GetSite()->GetProjectConfig()->DefaultTemplate + QString::number(level);
    QString MessageText_ = Warnings::RetrieveTemplateToWarn(_template, edit->GetSite(), instant);

    if (!MessageText_.size())
    {
        // this is very rare error no need to translate it
        Syslog::HuggleLogs->Log("There is no such warning template " + _template);
        return;
    }

    MessageText_ = MessageText_.replace("$2", edit->GetFullUrl()).replace("$1", edit->Page->PageName);
    QString Summary_;
    if (!edit->GetSite()->GetProjectConfig()->WarningSummaries.contains(edit->User->GetWarningLevel()))
    {
        Summary_ = edit->GetSite()->GetProjectConfig()->WarningSummaries[1];
    } else
    {
        Summary_ = edit->GetSite()->GetProjectConfig()->WarningSummaries[edit->User->GetWarningLevel()];
    }
    Summary_ = Summary_.replace("$1", sanitize_page_name(edit->Page->PageName));
    QString id = edit->GetSite()->GetProjectConfig()->TemplateHeader;
    id = id.replace("$1", sanitize_page_name(edit->Page->PageName));
    if (Configuration::HuggleConfiguration->UserConfig->EnforceMonthsAsHeaders)
    {
        QDateTime date_ = edit->GetSite()->GetProjectConfig()->ServerTime();
        id = WikiUtil::MonthText(date_.date().month(), edit->GetSite()) + " " + QString::number(date_.date().year());
    }
    MessageText_ = Warnings::UpdateSharedIPTemplate(edit->User, MessageText_, edit->GetSite());
    if (hcfg->UserConfig->AutomaticallyWatchlistWarnedUsers)
        WikiUtil::Watchlist(edit->User->GetTalkPage());
    WikiUtil::MessageUser(edit->User, MessageText_, id, Summary_, true, nullptr, false,
                              Configuration::HuggleConfiguration->UserConfig->SectionKeep,
                              true, edit->TPRevBaseTime);
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

QString Warnings::UpdateSharedIPTemplate(WikiUser *user, QString text, WikiSite *site)
{
    if (!user->IsIP() || site->GetProjectConfig()->SharedIPTemplate.isEmpty())
    {
        return text;
    }
    if (!user->TalkPage_ContainsSharedIPTemplate())
    {
        text += "\n" + site->GetProjectConfig()->SharedIPTemplate + "\n";
    }
    return text;
}
