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
    this->diffSize = 0;
    this->Diff = 0;
    this->OldID = 0;
    this->Summary = "";
    this->Status = StatusNone;
    this->CurrentUserWarningLevel = WarningLevelNone;
    this->OwnEdit = false;
    this->EditMadeByHuggle = false;
    this->TrustworthEdit = false;
    this->postProcessing = false;
    this->processingEditInfo = false;
    this->processingRevs = false;
    this->DiffText = "";
    this->Priority = 20;
    this->IsRevert = false;
    this->TPRevBaseTime = "";
    this->Previous = nullptr;
    // this is a problem we can't do this if we don't know the datetime because then the older edits
    // become newer and preflight checks will slap us for no reason
    // this->Time = QDateTime::currentDateTime();
    this->Time = GetUnknownEditTime();
    this->Next = nullptr;
    this->processingByWorkerThread = false;
    this->processedByWorkerThread = false;
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

bool WikiEdit::finalizePostProcessing()
{
    if (this->processedByWorkerThread || !this->postProcessing)
    {
        WikiUser::UpdateWl(this->User, this->Score);
        return true;
    }

    if (this->processingByWorkerThread)
    {
        return false;
    }

    if (this->qCategoriesAndWatched != nullptr && this->qCategoriesAndWatched->IsProcessed())
    {
        if (this->qCategoriesAndWatched->IsFailed())
        {
            Syslog::HuggleLogs->ErrorLog("Unable to fetch categories for page " + this->Page->PageName + ": " + this->qCategoriesAndWatched->GetFailureReason());
        } else
        {
            QList<ApiQueryResultNode*> categories = this->qCategoriesAndWatched->GetApiQueryResult()->GetNodes("cl");
            QStringList categoryStringList;
            foreach (ApiQueryResultNode *cat, categories)
            {
                categoryStringList.append(WikiPage(cat->GetAttribute("title"), this->GetSite()).RootName());
            }
            this->Page->SetCategories(categoryStringList);

            ApiQueryResultNode *page = this->qCategoriesAndWatched->GetApiQueryResult()->GetNode("page");
            this->Page->SetWatched(page->GetAttribute("watched", "false") != "false");
        }
        this->qCategoriesAndWatched = nullptr;
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
                    this->recordScore("EditScore", this->User->EditCount * this->GetSite()->ProjectConfig->EditScore);
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
            this->recordScore("ScoreFlag", this->GetSite()->ProjectConfig->ScoreFlag * this->User->Groups.count());
            if (this->User->Groups.contains("bot"))
            {
                // if it's a flagged bot we likely don't need to watch them
                this->recordScore("BotScore", this->GetSite()->ProjectConfig->BotScore);
            }
            // let's delete it now
            this->qUser = nullptr;
        }
    }

    if (this->processingRevs)
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
        this->processingRevs = false;
    }

    if (this->processingEditInfo)
    {
        // check if api was processed
        if (!this->qRevisionInfo->IsProcessed())
        {
            return false;
        }

        if (this->qRevisionInfo->IsFailed())
        {
            // whoa it ended in error, we need to get rid of this edit somehow now
            Huggle::Syslog::HuggleLogs->WarningLog("Failed to obtain diff for " + this->Page->PageName + " the error was: " + this->qRevisionInfo->GetFailureReason());
            this->qRevisionInfo = nullptr;
            this->postProcessing = false;
            return true;
        }

        // parse the revision meta-data now
        QList<ApiQueryResultNode*> revision_data = this->qRevisionInfo->GetApiQueryResult()->GetNodes("rev");
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

            foreach (ApiQueryResultNode *tags, revision->ChildNodes)
            {
                if (tags->Name == "tags" && tags->ChildNodes.count())
                {
                    foreach (ApiQueryResultNode *t, tags->ChildNodes)
                        this->Tags.append(t->Value);
                }
            }
        }

        this->qRevisionInfo = nullptr;
        this->processingEditInfo = false;
    }

    if (this->processingDiff)
    {
        if (!this->qDifference->IsProcessed())
            return false;

        if (this->qDifference->IsFailed())
        {
            // We weren't able to retrieve the diff using action=compare so let's keep it empty and let the browser component fallback to alternative method
            Huggle::Syslog::HuggleLogs->WarningLog("Failed to obtain diff for " + this->Page->PageName + " the error was: " + this->qDifference->GetFailureReason());
            this->processingDiff = false;
            return false;
        }

        ApiQueryResultNode* diff = this->qDifference->GetApiQueryResult()->GetNode("compare");
        if (diff == nullptr)
        {
            Huggle::Syslog::HuggleLogs->WarningLog("Failed to obtain diff for " + this->Page->PageName + " no diff data in query result");
            HUGGLE_DEBUG1(this->qDifference->GetApiQueryResult()->Data);
            this->processingDiff = false;
            return false;
        }

        if (hcfg->Verbosity > 0)
        {
            // Safety check
            if (this->RevID != WIKI_UNKNOWN_REVID && diff->GetAttribute("torevid") != QString::number(this->RevID))
                HUGGLE_DEBUG1(this->Page->PageName + ": revid doesn't match: " + QString::number(this->RevID) + " != " + diff->GetAttribute("torevid"));

            if (diff->GetAttribute("fromtitle") != this->Page->PageName)
                HUGGLE_DEBUG1(QString::number(this->RevID) + ": pageid doesn't match " + this->Page->PageName + " != " + diff->GetAttribute("fromtitle"));
        }

        this->DiffText = diff->Value;

        this->qDifference.Delete();
        this->processingDiff = false;
    }

    if (this->qText != nullptr && this->qText->IsProcessed())
    {
        bool failed = false;
        QString result = WikiUtil::EvaluateWikiPageContents(this->qText, &failed);
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
    if (this->processingRevs || this->processingDiff || this->processingEditInfo || this->qUser != nullptr || this->qText != nullptr || this->qFounder != nullptr || this->qCategoriesAndWatched != nullptr)
        return false;

    this->qTalkpage = nullptr;
    this->processingByWorkerThread = true;
    WikiEdit_ProcessorThread::EditLock.lock();
    this->RegisterConsumer(HUGGLECONSUMER_PROCESSOR);
    WikiEdit_ProcessorThread::PendingEdits.append(this);
    WikiEdit_ProcessorThread::EditLock.unlock();
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
        this->recordScore("PartsInWikiText_NoTalk", ProcessPartsInWikiText(&this->ScoreWords, text, &conf->NoTalkScoreParts));
        this->recordScore("WordsInWikiText_NoTalk", ProcessWordsInWikiText(&this->ScoreWords, text, &conf->NoTalkScoreWords));
    }
    this->recordScore("WordsInWikiText", ProcessWordsInWikiText(&this->ScoreWords, text, &conf->ScoreWords));
    this->recordScore("PartsInWikiText", ProcessPartsInWikiText(&this->ScoreWords, text, &conf->ScoreParts));
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

void WikiEdit::recordScore(QString name, score_ht score)
{
    this->Score += score;
    if (hcfg->SystemConfig_ScoreDebug)
    {
        QString score_name = QString("score_huggle_") + name;
        if (!this->PropertyBag.contains(score_name))
        {
            this->PropertyBag.insert(score_name, score);
        } else
        {
            HUGGLE_DEBUG1("Edit: " + QString::number(this->RevID) + ": multiple scores with name " + name);
            this->PropertyBag[score_name] = score;
        }
     }
}

void WikiEdit::PostProcess()
{
    if (this->postProcessing)
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
    this->postProcessing = true;
#ifndef HUGGLE_SDK
    // Send info to other functions
    Hooks::EditBeforePostProcess(this);
#endif
    this->qTalkpage = WikiUtil::RetrieveWikiPageContents(this->User->GetTalk(), this->GetSite());
    HUGGLE_QP_APPEND(this->qTalkpage);
    this->qTalkpage->Target = "Retrieving tp " + this->User->GetTalk();
    this->qTalkpage->Process();
    if (!this->NewPage)
    {
        // This query will fetch information about the revision(s) but not the diff itself
        this->qRevisionInfo = new ApiQuery(ActionQuery, this->GetSite());
        if (this->RevID != WIKI_UNKNOWN_REVID)
        {
            // &rvprop=content can't be used because of fuck up of mediawiki
            this->qRevisionInfo->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("ids|tags|user|timestamp|comment") +
                                              "&rvlimit=1&rvstartid=" + QString::number(this->RevID) + "&titles=" +
                                              QUrl::toPercentEncoding(this->Page->PageName);
        } else
        {
            this->qRevisionInfo->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("ids|tags|user|timestamp|comment") +
                                              "&rvlimit=1&titles=" + QUrl::toPercentEncoding(this->Page->PageName);
        }
        this->qRevisionInfo->Target = this->Page->PageName;
        HUGGLE_QP_APPEND(this->qRevisionInfo);
        this->qRevisionInfo->Process();
        if (hcfg->Verbosity > 0)
            this->PropertyBag.insert("debug_api_url_rev_info", this->qRevisionInfo->GetURL());
        this->processingEditInfo = true;

        // This query will download the actual diff of edit
        if (this->RevID != WIKI_UNKNOWN_REVID)
        {
            if (!this->IsRangeOfEdits())
                this->qDifference = WikiUtil::APIRequest(ActionCompare, this->GetSite(), "fromrev=" + QString::number(this->RevID) + "&torelative=" + this->DiffTo, false, "Diff of " + this->Page->PageName);
            else
                this->qDifference = WikiUtil::APIRequest(ActionCompare, this->GetSite(), "fromrev=" + QString::number(this->RevID) + "&torev=" + this->DiffTo, false, "Diff of " + this->Page->PageName);
        } else
        {
            this->qDifference = WikiUtil::APIRequest(ActionCompare, this->GetSite(), "fromtitle=" + QUrl::toPercentEncoding(this->Page->PageName) + "&torelative=" + this->DiffTo, false, "Diff of " + this->Page->PageName);
        }
        this->processingDiff = true;
    } else if (this->Page->Contents.isEmpty())
    {
        this->qText = WikiUtil::RetrieveWikiPageContents(this->Page, true);
        this->qText->Target = "Retrieving content of " + this->Page->PageName;
        HUGGLE_QP_APPEND(this->qText);
        this->qText->Process();
    }
    if (hcfg->UserConfig->RetrieveFounder)
    {
        this->qFounder = new ApiQuery(ActionQuery, this->GetSite());
        this->qFounder->Parameters = "prop=revisions&titles=" + QUrl::toPercentEncoding(this->Page->PageName) + "&rvdir=newer&rvlimit=1&rvprop=" +
                                     QUrl::toPercentEncoding("ids|user|timestamp");
        this->qFounder->Target = this->Page->PageName + " (retrieving founder)";
        HUGGLE_QP_APPEND(this->qFounder);
        this->qFounder->Process();
    }

    if (hcfg->SystemConfig_CatScansAndWatched)
    {
        this->qCategoriesAndWatched = new ApiQuery(ActionQuery, this->GetSite());
        this->qCategoriesAndWatched->Parameters = "prop=" + QUrl::toPercentEncoding("categories|info") + "&titles=" + QUrl::toPercentEncoding(this->Page->PageName) + "&inprop=watched";
        this->qCategoriesAndWatched->Target = this->Page->PageName + " (retrieving categories+watched)";
        HUGGLE_QP_APPEND(this->qCategoriesAndWatched);
        this->qCategoriesAndWatched->Process();
    }

    this->processingRevs = true;
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

bool WikiEdit::IsRangeOfEdits()
{
    return (this->DiffTo.toLower() != "prev"
       && this->DiffTo.toLower() != "next"
       && !this->DiffTo.isEmpty());
}

bool WikiEdit::IsReady()
{
#ifdef HUGGLE_SDK
    return true;
#else
    return Hooks::EditCheckIfReady(this);
#endif
}

QMutex WikiEdit_ProcessorThread::EditLock(QMutex::Recursive);
QList<WikiEdit*> WikiEdit_ProcessorThread::PendingEdits;

void WikiEdit_ProcessorThread::run()
{
    while(Core::HuggleCore->Running)
    {
        WikiEdit_ProcessorThread::EditLock.lock();
        int e=0;
        while (e<WikiEdit_ProcessorThread::PendingEdits.count())
        {
            this->Process(PendingEdits.at(e));
            PendingEdits.at(e)->UnregisterConsumer(HUGGLECONSUMER_PROCESSOR);
            ++e;
        }
        PendingEdits.clear();
        WikiEdit_ProcessorThread::EditLock.unlock();
        QThread::usleep(200000);
    }
}

void WikiEdit_ProcessorThread::Process(WikiEdit *edit)
{
#ifndef HUGGLE_SDK
    if (Hooks::EditBeforeScore(edit))
    {
#endif
        bool IgnoreWords = false;
        ProjectConfiguration *conf = edit->GetSite()->GetProjectConfig();
        if (edit->IsRevert)
        {
            if (edit->User->IsIP())
            {
                // Reverts made by anons are very likely reverts to vandalism
                edit->recordScore("IPScore_talk", conf->IPScore * 10);
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
            edit->recordScore("IPScore", conf->IPScore);
        }
        if (edit->Bot)
            edit->recordScore("BotScore", conf->BotScore);
        if (edit->Page->IsUserpage() && !edit->Page->SanitizedName().contains(edit->User->Username))
            edit->recordScore("ForeignUser", conf->ForeignUser);
        else if (edit->Page->IsUserpage())
            edit->recordScore("UserPage", conf->ScoreUser);
        if (edit->Page->IsTalk())
            edit->recordScore("ScoreTalk", conf->ScoreTalk);
        if (edit->diffSize > 1200 || edit->diffSize < -1200)
            edit->recordScore("ScoreChange", conf->ScoreChange);
        if (edit->Page->IsUserpage())
            IgnoreWords = true;
        if (edit->User->IsWhitelisted())
            edit->recordScore("WhitelistScore", conf->WhitelistScore);
        edit->recordScore("User_BadnessScore", edit->User->GetBadnessScore());
        if (!IgnoreWords)
            edit->ProcessWords();
        foreach (QString tx, edit->Tags)
        {
            if (conf->ScoreTags.contains(tx))
                edit->recordScore("tag_" + tx, conf->ScoreTags[tx]);
        }
        if (edit->SizeIsKnown && edit->diffSize < (-1 * conf->LargeRemoval))
            edit->recordScore("ScoreRemoval", conf->ScoreRemoval);
        edit->User->ParseTP(QDate::currentDate());
        if (edit->Summary.size() == 0)
            edit->recordScore("NoSummary", 10);
        int warning_level = edit->User->GetWarningLevel();
        if (warning_level > 0)
        {
            if (!conf->ScoreLevel.contains(warning_level))
            {
                HUGGLE_WARNING("No score present for warning level " + QString::number(warning_level) + " of user " + edit->User->Username + " site " + edit->GetSite()->Name);
            } else
            {
                edit->recordScore("WarningLevel", conf->ScoreLevel[warning_level]);
            }
        }
        switch(warning_level)
        {
            case 1:
                edit->CurrentUserWarningLevel = WarningLevel1;
                break;
            case 2:
                edit->CurrentUserWarningLevel = WarningLevel2;
                break;
            case 3:
                edit->CurrentUserWarningLevel = WarningLevel3;
                break;
            case 4:
                edit->CurrentUserWarningLevel = WarningLevel4;
                break;
        }
#ifndef HUGGLE_SDK
    }
    Hooks::EditPostProcess(edit);
#endif
    edit->postProcessing = false;
    edit->processedByWorkerThread = true;
    edit->Status = StatusPostProcessed;
}
