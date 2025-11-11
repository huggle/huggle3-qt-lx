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

PendingWarning::PendingWarning(Message *message, const QString &warning, WikiEdit *edit)
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
        return ":" + page_name;
    }
    return page_name;
}

PendingWarning *Warnings::WarnUser(const QString& warning_type, RevertQuery *dependency, WikiEdit *edit, bool *report)
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

    if (hcfg->UserConfig->ConfirmWarningOnVeryOldEdits)
    {
        // User doesn't want to send warnings for edits that are too old check if this edit is not too old
        if (edit->Time.addDays(1) < QDateTime::currentDateTime())
        {
            // The edit is older than 1 day, so let's either ignore the request to send warning or ask user if they really want to send it
            if (hcfg->UserConfig->SkipWarningOnConfirm)
            {
                HUGGLE_LOG(_l("warning-too-old-skip", edit->User->Username, edit->Page->PageName, edit->GetSite()->Name));
                return nullptr;
            } else
            {
                // Ask user if they really want to send a warning here
                // en: Edit to $1 on $2 by $3 is older than 1 day, do you really want to send them a warning message?
                if (!Hooks::ShowYesNoQuestion(_l("warning-confirm-title"), _l("warning-confirm-old-edit", edit->Page->PageName, edit->GetSite()->Name, edit->User->Username), false))
                    return nullptr;
            }
        }
    }

    // If user received another warning recently, we give him some time to realize what's going on before sending more warnings
    if (edit->User->LastMessageTimeKnown && hcfg->UserConfig->ConfirmOnRecentWarning)
    {
        if (edit->User->LastMessageTime.addSecs(hcfg->UserConfig->RecentWarningTimeSpan) > QDateTime::currentDateTime())
        {
            // This warning is too recent
            if (hcfg->UserConfig->SkipWarningOnConfirm)
            {
                HUGGLE_LOG("Not sending warning to " + edit->User->Username + " for their edit to " + edit->Page->PageName + " on " + edit->GetSite()->Name +
                           " because their talk page was edited too recently");
                return nullptr;
            } else
            {
                // Ask user if they really want to send a warning here
                // en: Edit to $1 on $2 was made by $3 who received a message on their talk page very recently (probably some other warning template?) do you really want to send them another warning message?
                if (!Hooks::ShowYesNoQuestion(_l("warning-confirm-title"), _l("warning-confirm-too-recent", edit->Page->PageName, edit->GetSite()->Name, edit->User->Username), false))
                    return nullptr;
            }
        }
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
            // en: This user has already reached level 4 warning and there is no AIV supported on this wiki, you should block the user now
            Syslog::HuggleLogs->WarningLog(_l("warning-no-aiv-config"));
            return nullptr;
        }

        if (hcfg->UserConfig->AutomaticReports || Generic::ReportPreFlightCheck())
        {
            *report = true;
        }
        return nullptr;
    }

    edit->User->IncrementWarningLevel();
    // This must not be here, many warnings fail due to expired talk page and are re-sent
    // if we set last message time here, sending after reparse will fail due to that
    //edit->User->SetLastMessageTime(QDateTime::currentDateTime());
    // We need to update the user so that new user warning level gets propagated everywhere on interface of huggle
    edit->User->Update();

    // Create a warning template and message to send
    QString message_template = warning_type + QString::number(edit->User->GetWarningLevel());
    QString message_text = Warnings::RetrieveTemplateToWarn(message_template, edit->GetSite());

    // In case that message is empty, stop and show error
    if (!message_text.size())
    {
        Syslog::HuggleLogs->ErrorLog(_l("missing-warning", message_template));
        return nullptr;
    }

    message_text = message_text.replace("$2", edit->GetFullUrl()).replace("$1", edit->Page->PageName);
    QString message_summary;
    if (!edit->GetSite()->GetProjectConfig()->WarningSummaries.contains(edit->User->GetWarningLevel()))
    {
        message_summary = edit->GetSite()->GetProjectConfig()->WarningSummaries[1];
    } else
    {
        message_summary = edit->GetSite()->GetProjectConfig()->WarningSummaries[edit->User->GetWarningLevel()];
    }
    message_summary = message_summary.replace("$1", sanitize_page_name(edit->Page->PageName));

    // Configure message heading as defined in project config
    QString message_head = edit->GetSite()->GetProjectConfig()->TemplateHeader;
    message_head = message_head.replace("$1", sanitize_page_name(edit->Page->PageName));
    if (edit->GetSite()->GetProjectConfig()->MessageHeadings == HeadingsStandard)
    {
        QDateTime server_time = edit->GetSite()->GetProjectConfig()->ServerTime();
        message_head = WikiUtil::MonthText(server_time.date().month(), edit->GetSite()) + " " + QString::number(server_time.date().year());
    } else if (edit->GetSite()->GetProjectConfig()->MessageHeadings == HeadingsNone)
    {
        message_head = "";
    }
    message_text = Warnings::UpdateSharedIPTemplate(edit->User, message_text, edit->GetSite());

    // Create only - safety check for API in case that user didn't have a user talk page, we use this parameter for API so that is fails in case someone creates a talk page meanwhile
    bool create_only = edit->User->TalkPage_GetContents().isEmpty();
    if (hcfg->UserConfig->AutomaticallyWatchlistWarnedUsers)
        WikiUtil::Watchlist(edit->User->GetTalkPage());
    PendingWarning *pw = new PendingWarning(WikiUtil::MessageUser(edit->User, message_text, message_head, message_summary, true, dependency, false, hcfg->UserConfig->SectionKeep, false,
                                                                  edit->TPRevBaseTime, create_only, true), warning_type, edit);
    Hooks::OnWarning(edit->User);
    return pw;
}

void Warnings::ResendWarnings()
{
    int warning_ix = 0;
    while (warning_ix < PendingWarning::PendingWarnings.count())
    {
        PendingWarning *warning = PendingWarning::PendingWarnings.at(warning_ix);
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
                    PendingWarning::PendingWarnings.removeAt(warning_ix);
                    delete warning;
                    continue;
                }
                // we get the new talk page
                QDomDocument talk_page;
                talk_page.setContent(warning->Query->Result->Data);
                QDomNodeList revisions_ = talk_page.elementsByTagName("rev");
                QDomNodeList pages_ = talk_page.elementsByTagName("page");
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
                        PendingWarning::PendingWarnings.removeAt(warning_ix);
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
                            PendingWarning::PendingWarnings.removeAt(warning_ix);
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
                        PendingWarning::PendingWarnings.removeAt(warning_ix);
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
                    PendingWarning::PendingWarnings.removeAt(warning_ix);
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
                PendingWarning::PendingWarnings.removeAt(warning_ix);
                delete warning;
                continue;
            }
            // in case that it isn't processed yet we can continue on next warning
            warning_ix++;
            continue;
        }

        if (warning->Warning->IsFinished())
        {
            if (!warning->Warning->IsFailed())
            {
                Hooks::WarningFinished(warning->RelatedEdit);
                // we no longer need to care about this one
                PendingWarning::PendingWarnings.removeAt(warning_ix);
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
                PendingWarning::PendingWarnings.removeAt(warning_ix);
                delete warning;
                continue;
            }
        }
        warning_ix++;
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

    QString warning_template = edit->GetSite()->GetProjectConfig()->DefaultTemplate + QString::number(level);
    QString message_text = Warnings::RetrieveTemplateToWarn(warning_template, edit->GetSite(), instant);

    if (!message_text.size())
    {
        // this is very rare error no need to translate it
        Syslog::HuggleLogs->Log("There is no such warning template " + warning_template);
        return;
    }

    message_text = message_text.replace("$2", edit->GetFullUrl()).replace("$1", edit->Page->PageName);
    QString message_summary;
    if (!edit->GetSite()->GetProjectConfig()->WarningSummaries.contains(edit->User->GetWarningLevel()))
    {
        message_summary = edit->GetSite()->GetProjectConfig()->WarningSummaries[1];
    } else
    {
        message_summary = edit->GetSite()->GetProjectConfig()->WarningSummaries[edit->User->GetWarningLevel()];
    }
    message_summary = message_summary.replace("$1", sanitize_page_name(edit->Page->PageName));
    QString message_head = edit->GetSite()->GetProjectConfig()->TemplateHeader;
    message_head = message_head.replace("$1", sanitize_page_name(edit->Page->PageName));
    if (hcfg->UserConfig->EnforceMonthsAsHeaders)
    {
        QDateTime date_ = edit->GetSite()->GetProjectConfig()->ServerTime();
        message_head = WikiUtil::MonthText(date_.date().month(), edit->GetSite()) + " " + QString::number(date_.date().year());
    }
    message_text = Warnings::UpdateSharedIPTemplate(edit->User, message_text, edit->GetSite());
    if (hcfg->UserConfig->AutomaticallyWatchlistWarnedUsers)
        WikiUtil::Watchlist(edit->User->GetTalkPage());
    WikiUtil::MessageUser(edit->User, message_text, message_head, message_summary, true, nullptr, false, hcfg->UserConfig->SectionKeep, true, edit->TPRevBaseTime);
}

QString Warnings::RetrieveTemplateToWarn(const QString& type, WikiSite *site, bool force)
{
    int x=0;
    QString result = "";
    while (x < site->GetProjectConfig()->WarningTemplates.count())
    {
        if (HuggleParser::GetKeyFromSSItem(site->GetProjectConfig()->WarningTemplates.at(x)) == type)
        {
            result = HuggleParser::GetValueFromSSItem(site->GetProjectConfig()->WarningTemplates.at(x));
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
    if (!user->IsAnon() || site->GetProjectConfig()->SharedIPTemplate.isEmpty())
    {
        return text;
    }
    if (!user->TalkPage_ContainsSharedIPTemplate())
    {
        text += "\n" + site->GetProjectConfig()->SharedIPTemplate + "\n";
    }
    return text;
}
