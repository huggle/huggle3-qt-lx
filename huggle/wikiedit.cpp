//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikiedit.hpp"
#include <QMutex>
#include <QUrl>
#include "apiqueryresult.hpp"
#include "configuration.hpp"
#include "generic.hpp"
#include "hooks.hpp"
#include "core.hpp"
#include "querypool.hpp"
#include "exception.hpp"
#include "syslog.hpp"
#include "mediawiki.hpp"
#include "wikipage.hpp"
#include "wikiutil.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"
#include "localization.hpp"

using namespace Huggle;
QList<WikiEdit*> WikiEdit::EditList;
QMutex *WikiEdit::Lock_EditList = new QMutex(QMutex::Recursive);

WikiEdit::WikiEdit()
{
    this->Bot = false;
    this->User = nullptr;
    this->Minor = false;
    this->NewPage = false;
    this->Size = 0;
    this->Diff = 0;
    this->OldID = 0;
    this->Summary = "";
    this->Status = StatusNone;
    this->CurrentUserWarningLevel = WarningLevelNone;
    this->OwnEdit = false;
    this->EditMadeByHuggle = false;
    this->TrustworthEdit = false;
    this->PostProcessing = false;
    this->ProcessingDiff = false;
    this->ProcessingRevs = false;
    this->DiffText = "";
    this->Priority = 20;
    this->Score = 0;
    this->IsRevert = false;
    this->TPRevBaseTime = "";
    this->Previous = nullptr;
    // this is a problem we can't do this if we don't know the datetime because then the older edits
    // become newer and preflight checks will slap us for no reason
    // this->Time = QDateTime::currentDateTime();
    this->Time = GetUnknownEditTime();
    this->Next = nullptr;
    this->ProcessingByWorkerThread = false;
    this->ProcessedByWorkerThread = false;
    this->RevID = WIKI_UNKNOWN_REVID;
    WikiEdit::Lock_EditList->lock();
    WikiEdit::EditList.append(this);
    WikiEdit::Lock_EditList->unlock();
}

WikiEdit::~WikiEdit()
{
    WikiEdit::Lock_EditList->lock();
    WikiEdit::EditList.removeAll(this);
    WikiEdit::Lock_EditList->unlock();
    if (this->Previous != nullptr && this->Next != nullptr)
    {
        this->Previous->Next = this->Next;
        this->Next->Previous = this->Previous;
    } else
    {
        if (this->Previous != nullptr)
        {
            this->Previous->Next = nullptr;
        }
        if (this->Next != nullptr)
        {
            this->Next->Previous = nullptr;
        }
    }
    delete this->User;
    delete this->Page;
}

bool WikiEdit::FinalizePostProcessing()
{
    if (this->ProcessedByWorkerThread || !this->PostProcessing)
    {
        WikiUser::UpdateWl(this->User, this->Score);
        return true;
    }

    if (this->ProcessingByWorkerThread)
    {
        return false;
    }

    if (this->qFounder != nullptr && this->qFounder->IsProcessed())
    {
        if (this->qFounder->IsFailed())
        {
            Syslog::HuggleLogs->ErrorLog("Failed to retrieve founder for page " + this->Page->PageName + ": " + this->qFounder->GetFailureReason());
        } else
        {
            QList<ApiQueryResultNode*> revisions = this->qFounder->GetApiQueryResult()->GetNodes("rev");
            if (revisions.count() == 0)
            {
                Syslog::HuggleLogs->ErrorLog("Failed to retrieve founder for page " + this->Page->PageName + ": " + this->qFounder->GetFailureReason());
                HUGGLE_DEBUG(this->qFounder->Result->Data, 1);
            }
            else if (revisions.count() == 1)
            {
                ApiQueryResultNode *node = revisions.at(0);
                if (!node->Attributes.contains("timestamp") || !node->Attributes.contains("user"))
                {
                    HUGGLE_DEBUG1("Invalid revision info while fetching info for page");
                }
                else
                {
                    this->Page->SetFounder(node->GetAttribute("user"));
                }
            }
        }
        this->qFounder = nullptr;
    }

    if (this->qUser != nullptr && this->qUser->IsProcessed())
    {
        if (this->qUser->IsFailed())
        {
            // it failed for some reason
            Syslog::HuggleLogs->ErrorLog("Unable to fetch user information for " + this->User->Username + ": " +
                                         this->qUser->GetFailureReason());
            // we can remove this query now
            this->qUser = nullptr;

        } else
        {
            // we fetch the number of edits, registration and groups of user
            QList<ApiQueryResultNode*> user_data = this->qUser->GetApiQueryResult()->GetNodes("user");
            QList<ApiQueryResultNode*> group_data = this->qUser->GetApiQueryResult()->GetNodes("g");
            if (user_data.count() > 0)
            {
                ApiQueryResultNode *user_info_ = user_data.at(0);
                if (user_info_->Attributes.contains("editcount"))
                {
                    this->User->EditCount = user_info_->GetAttribute("editcount").toLong();
                    // users with high number of edits aren't vandals
                    this->Score += this->User->EditCount*-2;
                }
                else
                {
                    Syslog::HuggleLogs->WarningLog("Failed to retrieve edit count for " + this->User->Username);
                }
                if (user_info_->Attributes.contains("registration"))
                {
                    this->User->RegistrationDate = user_info_->GetAttribute("registration");
                }
                else
                {
                    Syslog::HuggleLogs->WarningLog("Wiki returned no registration time of " + this->User->Username);
                }
            }
            int x = 0;
            while (x < group_data.count())
            {
                ApiQueryResultNode *group = group_data.at(x);
                QString gn = group->Value;
                if (gn != "*" && gn != "user")
                this->User->Groups.append(gn);
                ++x;
            }
            this->Score += (this->GetSite()->ProjectConfig->ScoreFlag * this->User->Groups.count());
            if (this->User->Groups.contains("bot"))
            {
                // if it's a flagged bot we likely don't need to watch them
                this->Score += this->GetSite()->ProjectConfig->BotScore;
            }
            // let's delete it now
            this->qUser = nullptr;
        }
    }

    if (this->ProcessingRevs)
    {
        // check if api was processed
        if (!this->qTalkpage->IsProcessed())
            return false;

        if (this->qTalkpage->IsFailed())
        {
            Huggle::Syslog::HuggleLogs->Log(_l("wikiedit-tp-fail", this->User->GetTalk()));
        } else
        {
            // parse the talk page now
            QList<ApiQueryResultNode*> rev_ = this->qTalkpage->GetApiQueryResult()->GetNodes("rev");
            QList<ApiQueryResultNode*> pages_ = this->qTalkpage->GetApiQueryResult()->GetNodes("page");
            bool missing = false;
            if (pages_.count() > 0)
            {
                ApiQueryResultNode *node = pages_.at(0);
                if (node->Attributes.contains("missing"))
                {
                    missing = true;
                }
            }
            // get last id
            if (missing != true && rev_.count() > 0)
            {
                ApiQueryResultNode *rv = rev_.at(0);
                if (!rv->Attributes.contains("timestamp"))
                {
                    Huggle::Syslog::HuggleLogs->ErrorLog("Talk page timestamp of " + this->User->Username + " couldn't be retrieved, mediawiki returned no data for it");
                } else
                {
                    this->TPRevBaseTime = rv->GetAttribute("timestamp");
                }
                this->User->TalkPage_SetContents(rv->Value);
            } else
            {
                if (missing)
                {
                    // we set an empty talk page so that we know we do have the contents of this page
                    this->User->TalkPage_SetContents("");
                } else
                {
                    Huggle::Syslog::HuggleLogs->Log(_l("wikiedit-tp-fail", this->User->GetTalk()));
                    HUGGLE_DEBUG(this->qTalkpage->Result->Data, 1);
                }
            }
        }
        this->ProcessingRevs = false;
    }

    if (this->ProcessingDiff)
    {
        // check if api was processed
        if (!this->qDifference->IsProcessed())
        {
            return false;
        }

        if (this->qDifference->IsFailed())
        {
            // whoa it ended in error, we need to get rid of this edit somehow now
            Huggle::Syslog::HuggleLogs->WarningLog("Failed to obtain diff for " + this->Page->PageName + " the error was: "
                                                   + this->qDifference->Result->Data);
            this->qDifference = nullptr;
            this->PostProcessing = false;
            return true;
        }

        // parse the diff now
        QList<ApiQueryResultNode*> revision_data = this->qDifference->GetApiQueryResult()->GetNodes("rev");
        QList<ApiQueryResultNode*> diffs = this->qDifference->GetApiQueryResult()->GetNodes("diff");
        // get last id
        if (revision_data.count() > 0)
        {
            ApiQueryResultNode *revision = revision_data.at(0);
            if (revision->Value.length() > 0)
                this->Page->Contents = revision->Value;
            // check if this revision matches our user
            if (revision->Attributes.contains("user"))
            {
                if (WikiUtil::SanitizeUser(revision->GetAttribute("user")).toUpper() != WikiUtil::SanitizeUser(this->User->Username).toUpper())
                {
                    HUGGLE_DEBUG("User " + revision->GetAttribute("user") + " != " + this->User->Username, 3);
                    this->IsValid = false;
                }
            } else
            {
                this->IsValid = false;
            }
            if (revision->Attributes.contains("revid"))
                this->RevID = revision->GetAttribute("revid").toInt();
            if (revision->Attributes.contains("timestamp"))
                this->Time = MediaWiki::FromMWTimestamp(revision->GetAttribute("timestamp"));
            if (revision->Attributes.contains("comment"))
                this->Summary = revision->GetAttribute("comment");
        }
        if (diffs.count() > 0)
        {
            ApiQueryResultNode *temp = diffs.at(0);
            this->DiffText = temp->Value;
        } else
        {
            Huggle::Syslog::HuggleLogs->WarningLog("Failed to obtain diff for " + this->Page->PageName + " the error was: "
                                                 + this->qDifference->Result->Data);
        }
        this->qDifference = nullptr;
        // we are done processing the diff
        this->ProcessingDiff = false;
    }

    if (this->qText != nullptr && this->qText->IsProcessed())
    {
        bool failed = false;
        QString result = Generic::EvaluateWikiPageContents(this->qText, &failed);
        if (failed)
        {
            Syslog::HuggleLogs->ErrorLog("Failed to obtain text of " + this->Page->PageName + ": " + result);
        }
        else
        {
            this->Page->Contents = result;
            this->Page->Contents.replace("//", "https://");
        }
        this->qText = nullptr;
    }

    // check if everything was processed and clean up
    if (this->ProcessingRevs || this->ProcessingDiff || this->qUser != nullptr || this->qText != nullptr || this->qFounder != nullptr)
        return false;

    this->qTalkpage = nullptr;
    this->ProcessingByWorkerThread = true;
    ProcessorThread::EditLock.lock();
    this->RegisterConsumer(HUGGLECONSUMER_PROCESSOR);
    ProcessorThread::PendingEdits.append(this);
    ProcessorThread::EditLock.unlock();
    return false;
}

QString WikiEdit::GetPixmap()
{
    if (this->User == nullptr)
        throw new Huggle::NullPointerException("local WikiUser User", BOOST_CURRENT_FUNCTION);

    if (this->OwnEdit)
        return ":/huggle/pictures/Resources/blob-me.png";

    if (this->User->IsBlocked)
        return ":/huggle/pictures/Resources/blob-blocked.png";

    if (this->User->IsReported)
        return ":/huggle/pictures/Resources/blob-reported.png";

    if (this->IsRevert)
        return ":/huggle/pictures/Resources/blob-revert.png";

    if (this->Bot)
        return ":/huggle/pictures/Resources/blob-bot.png";

    switch (this->CurrentUserWarningLevel)
    {
        case WarningLevelNone:
            break;
        case WarningLevel1:
            return ":/huggle/pictures/Resources/blob-warn-1.png";
        case WarningLevel2:
            return ":/huggle/pictures/Resources/blob-warn-2.png";
        case WarningLevel3:
            return ":/huggle/pictures/Resources/blob-warn-3.png";
        case WarningLevel4:
            return ":/huggle/pictures/Resources/blob-warn-4.png";
    }

    if (this->Score > 1000)
        return ":/huggle/pictures/Resources/blob-warning.png";

    if (this->NewPage)
        return ":/huggle/pictures/Resources/blob-new.png";

    if (this->User->IsWhitelisted())
        return ":/huggle/pictures/Resources/blob-ignored.png";

    if (this->User->IsIP())
        return ":/huggle/pictures/Resources/blob-anon.png";

    return ":/huggle/pictures/Resources/blob-none.png";
}

static long ProcessPartsInWikiText(QStringList *list, QString text, QList<ScoreWord> *wl)
{
    long rs = 0;
    foreach (ScoreWord word, *wl)
    {
        QString w = word.word;
        if (text.contains(w))
        {
            rs += word.score;
            list->append(w);
        }
    }
    return rs;
}

static long ProcessWordsInWikiText(QStringList *list, QString text, QList<ScoreWord> *wl)
{
    long rs = 0;
    foreach (ScoreWord word, *wl)
    {
        QString w = word.word;
        // if there is no such a string in text we can skip it
        if (!text.contains(w))
            continue;

        int SD = 0;
        bool found = false;
        if (text == w)
            found = true;
        while (!found && SD < hcfg->SystemConfig_WordSeparators.count())
        {
            if (text.startsWith(w + hcfg->SystemConfig_WordSeparators.at(SD)))
            {
                found = true;
                break;
            }
            if (text.endsWith(hcfg->SystemConfig_WordSeparators.at(SD) + w))
            {
                found = true;
                break;
            }
            int SL = 0;
            while (SL < hcfg->SystemConfig_WordSeparators.count())
            {
                if (text.contains(hcfg->SystemConfig_WordSeparators.at(SD) + w + hcfg->SystemConfig_WordSeparators.at(SL)))
                {
                    found = true;
                    break;
                }
                if (found)
                    break;
                ++SL;
            }
            ++SD;
        }
        if (found)
        {
            rs += word.score;
            list->append(w);
        }
    }
    return rs;
}

void WikiEdit::ProcessWords()
{
    QString text;
    if (this->DiffText_IsSplit)
        text = this->DiffText_New.toLower();
    else
        text = this->DiffText.toLower();
    if (text.isEmpty() && this->Page->Contents.length() > 0)
    {
        text = this->Page->Contents.toLower();
    }
    // we cache the project config pointer
    ProjectConfiguration *conf = this->GetSite()->GetProjectConfig();
    if (!this->Page->IsTalk())
    {
        this->Score += ProcessPartsInWikiText(&this->ScoreWords, text, &conf->NoTalkScoreParts);
        this->Score += ProcessWordsInWikiText(&this->ScoreWords, text, &conf->NoTalkScoreWords);
    }
    this->Score += ProcessWordsInWikiText(&this->ScoreWords, text, &conf->ScoreWords);
    this->Score += ProcessPartsInWikiText(&this->ScoreWords, text, &conf->ScoreParts);
}

void WikiEdit::RemoveFromHistoryChain()
{
    if (this->Previous != nullptr && this->Next != nullptr)
    {
        this->Previous->Next = this->Next;
        this->Next->Previous = this->Previous;
        this->Previous = nullptr;
        this->Next = nullptr;
        return;
    }

    if (this->Previous != nullptr)
    {
        this->Previous->Next = nullptr;
        this->Previous = nullptr;
    }

    if (this->Next != nullptr)
    {
        this->Next->Previous = nullptr;
        this->Next = nullptr;
    }
}

void WikiEdit::PostProcess()
{
    if (this->PostProcessing)
        return;

    if (this->Page == nullptr)
        throw new Huggle::NullPointerException("local WikiPage Page", BOOST_CURRENT_FUNCTION);
    if (this->Status == Huggle::StatusNone)
    {
        Exception::ThrowSoftException("Processing edit to " + this->Page->PageName + "which was requested to be post processed,"\
                                      " but wasn't processed yet", BOOST_CURRENT_FUNCTION);
        QueryPool::HugglePool->PreProcessEdit(this);
    }
    if (this->Status == Huggle::StatusPostProcessed)
        throw new Huggle::Exception("Unable to post process an edit that is already processed", BOOST_CURRENT_FUNCTION);
    if (this->Status != Huggle::StatusProcessed)
        throw new Huggle::Exception("Unable to post process an edit that wasn't in processed status", BOOST_CURRENT_FUNCTION);
    this->PostProcessing = true;
    this->qTalkpage = Generic::RetrieveWikiPageContents(this->User->GetTalk(), this->GetSite());
    QueryPool::HugglePool->AppendQuery(this->qTalkpage);
    this->qTalkpage->Target = "Retrieving tp " + this->User->GetTalk();
    this->qTalkpage->Process();
    if (!this->NewPage)
    {
        this->qDifference = new ApiQuery(ActionQuery, this->GetSite());
        if (this->RevID != WIKI_UNKNOWN_REVID)
        {
            // &rvprop=content can't be used because of fuck up of mediawiki
            this->qDifference->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("ids|user|timestamp|comment") +
                                            "&rvlimit=1&rvstartid=" + QString::number(this->RevID) + "&rvdiffto=" + this->DiffTo + "&titles=" +
                                            QUrl::toPercentEncoding(this->Page->PageName);
        } else
        {
            this->qDifference->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("ids|user|timestamp|comment") +
                                            "&rvlimit=1&rvdiffto=" + this->DiffTo + "&titles=" +
                                            QUrl::toPercentEncoding(this->Page->PageName);
        }
        this->qDifference->Target = this->Page->PageName;
        QueryPool::HugglePool->AppendQuery(this->qDifference);
        this->qDifference->Process();
        this->ProcessingDiff = true;
    } else if (this->Page->Contents.isEmpty())
    {
        this->qText = Generic::RetrieveWikiPageContents(this->Page, true);
        this->qText->Target = "Retrieving content of " + this->Page->PageName;
        QueryPool::HugglePool->AppendQuery(this->qText);
        this->qText->Process();
    }
    if (hcfg->UserConfig->RetrieveFounder)
    {
        this->qFounder = new ApiQuery(ActionQuery, this->GetSite());
        this->qFounder->Parameters = "prop=revisions&titles=" + QUrl::toPercentEncoding(this->Page->PageName) + "&rvdir=newer&rvlimit=1&rvprop=" +
                                     QUrl::toPercentEncoding("ids|user|timestamp");
        this->qFounder->Target = this->Page->PageName + " (retrieving founder)";
        QueryPool::HugglePool->AppendQuery(this->qFounder);
        this->qFounder->Process();
    }
    this->ProcessingRevs = true;
    if (this->User->IsIP())
        return;
    this->qUser = new ApiQuery(ActionQuery, this->GetSite());
    this->qUser->Parameters = "list=users&usprop=blockinfo%7Cgroups%7Ceditcount%7Cregistration&ususers="
                                + QUrl::toPercentEncoding(this->User->Username);
    this->qUser->Process();
}

Collectable_SmartPtr<WikiEdit> WikiEdit::FromCacheByRevID(revid_ht revid, QString prev)
{
    Collectable_SmartPtr<WikiEdit> e;
    if (revid == WIKI_UNKNOWN_REVID)
    {
        // there is no such an edit
        return e;
    }
    WikiEdit::Lock_EditList->lock();
    int x = 0;
    while (x < WikiEdit::EditList.count())
    {
        WikiEdit *edit = WikiEdit::EditList.at(x);
        ++x;
        if (edit->RevID == revid && edit->DiffTo == prev)
        {
            e = edit;
            // let's return it
            break;
        }
    }
    WikiEdit::Lock_EditList->unlock();
    return e;
}

QString WikiEdit::GetPixmapFromEditType(EditType edit_type)
{
    switch (edit_type)
    {
        case EditType_Normal:
            return ":/huggle/pictures/Resources/blob-none.png";
        case EditType_1:
            return ":/huggle/pictures/Resources/blob-warn-1.png";
        case EditType_2:
            return ":/huggle/pictures/Resources/blob-warn-2.png";
        case EditType_3:
            return ":/huggle/pictures/Resources/blob-warn-3.png";
        case EditType_4:
            return ":/huggle/pictures/Resources/blob-warn-4.png";
        case EditType_Anon:
            return ":/huggle/pictures/Resources/blob-anon.png";
        case EditType_Bot:
            return ":/huggle/pictures/Resources/blob-bot.png";
        case EditType_Blocked:
            return ":/huggle/pictures/Resources/blob-blocked.png";
        case EditType_New:
            return ":/huggle/pictures/Resources/blob-new.png";
        case EditType_Reported:
            return ":/huggle/pictures/Resources/blob-reported.png";
        case EditType_Revert:
            return ":/huggle/pictures/Resources/blob-revert.png";
        case EditType_Self:
            return ":/huggle/pictures/Resources/blob-me.png";
        case EditType_W:
            return ":/huggle/pictures/Resources/blob-ignored.png";
    }
    return ":/huggle/pictures/Resources/blob-none.png";
}

WikiSite *WikiEdit::GetSite()
{
    if (this->Page == nullptr)
    {
        // this must never happen
        throw new Huggle::NullPointerException("local WikiSite site", BOOST_CURRENT_FUNCTION);
    }
    return this->Page->Site;
}

QString WikiEdit::GetFullUrl()
{
    return Configuration::GetProjectScriptURL(this->GetSite()) + "index.php?title=" + QUrl::toPercentEncoding(this->Page->PageName) +
            "&diff=" + QString::number(this->RevID);
}

QMutex ProcessorThread::EditLock(QMutex::Recursive);
QList<WikiEdit*> ProcessorThread::PendingEdits;

void ProcessorThread::run()
{
    while(Core::HuggleCore->Running)
    {
        ProcessorThread::EditLock.lock();
        int e=0;
        while (e<ProcessorThread::PendingEdits.count())
        {
            this->Process(PendingEdits.at(e));
            PendingEdits.at(e)->UnregisterConsumer(HUGGLECONSUMER_PROCESSOR);
            ++e;
        }
        PendingEdits.clear();
        ProcessorThread::EditLock.unlock();
        QThread::usleep(200000);
    }
}

void ProcessorThread::Process(WikiEdit *edit)
{
    if (Hooks::EditBeforeScore(edit))
    {
        bool IgnoreWords = false;
        if (edit->IsRevert)
        {
            if (edit->User->IsIP())
            {
                edit->Score += 200;
            } else
            {
                if (!edit->DiffText_IsSplit)
                {
                    // we have to ignore the score words here because there is always lot of them in revert text
                    IgnoreWords = true;
                }
            }
        }
        // score
        if (edit->User->IsIP())
        {
            edit->Score += edit->GetSite()->GetProjectConfig()->IPScore;
        }
        if (edit->Bot)
            edit->Score += edit->GetSite()->GetProjectConfig()->BotScore;
        if (edit->Page->IsUserpage() && !edit->Page->SanitizedName().contains(edit->User->Username))
            edit->Score += edit->GetSite()->GetProjectConfig()->ForeignUser;
        else if (edit->Page->IsUserpage())
            edit->Score += edit->GetSite()->GetProjectConfig()->ScoreUser;
        if (edit->Page->IsTalk())
            edit->Score += edit->GetSite()->GetProjectConfig()->ScoreTalk;
        if (edit->Size > 1200 || edit->Size < -1200)
            edit->Score += edit->GetSite()->GetProjectConfig()->ScoreChange;
        if (edit->Page->IsUserpage())
            IgnoreWords = true;
        if (edit->User->IsWhitelisted())
            edit->Score += edit->GetSite()->GetProjectConfig()->WhitelistScore;
        edit->Score += edit->User->GetBadnessScore();
        if (!IgnoreWords)
            edit->ProcessWords();
        if (edit->SizeIsKnown && edit->Size < (-1*edit->GetSite()->GetProjectConfig()->LargeRemoval))
            edit->Score += edit->GetSite()->GetProjectConfig()->ScoreRemoval;
        edit->User->ParseTP(QDate::currentDate());
        if (edit->Summary.size() == 0)
            edit->Score += 10;
        switch(edit->User->GetWarningLevel())
        {
            case 1:
                edit->Score += 200;
                edit->CurrentUserWarningLevel = WarningLevel1;
                break;
            case 2:
                edit->Score += 1000;
                edit->CurrentUserWarningLevel = WarningLevel2;
                break;
            case 3:
                edit->Score += 2000;
                edit->CurrentUserWarningLevel = WarningLevel3;
                break;
            case 4:
                // people with 4 warnings are so much watched that someone probably revert them
                // faster than you notice, let's put them lower than unattended vandals
                edit->Score += 1000;
                edit->CurrentUserWarningLevel = WarningLevel4;
                break;
        }
    }
    Hooks::EditPostProcess(edit);
    edit->PostProcessing = false;
    edit->ProcessedByWorkerThread = true;
    edit->Status = StatusPostProcessed;
}
