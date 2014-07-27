//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikiedit.hpp"
#include <QtXml>
#include <QMutex>
#include "configuration.hpp"
#include "generic.hpp"
#include "core.hpp"
#include "querypool.hpp"
#include "exception.hpp"
#include "syslog.hpp"
#include "mediawiki.hpp"
#include "wikipage.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"

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
    this->RollbackToken = "";
    this->PostProcessing = false;
    this->ProcessingDiff = false;
    this->ProcessingRevs = false;
    this->DiffText = "";
    this->Priority = 20;
    this->Score = 0;
    this->IsRevert = false;
    this->TPRevBaseTime = "";
    this->PatrolToken = "";
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

    if (this->qUser != nullptr && this->qUser->IsProcessed())
    {
        if (this->qUser->IsFailed())
        {
            // it failed for some reason
            Syslog::HuggleLogs->ErrorLog("Unable to fetch user information for " + this->User->Username + ": " +
                                         this->qUser->Result->ErrorMessage);
            // we can remove this query now
            this->qUser = nullptr;

        } else
        {
            // we fetch the number of edits, registration and groups of user
            QDomDocument d_;
            d_.setContent(this->qUser->Result->Data);
            QDomNodeList u_ = d_.elementsByTagName("user");
            QDomNodeList g_ = d_.elementsByTagName("g");
            if (u_.count() > 0)
            {
                QDomElement user_info_ = u_.at(0).toElement();
                if (user_info_.attributes().contains("editcount"))
                {
                    this->User->EditCount = user_info_.attribute("editcount").toLong();
                    // users with high number of edits aren't vandals
                    this->Score += this->User->EditCount*-2;
                }
                else
                {
                    Syslog::HuggleLogs->WarningLog("Failed to retrieve edit count for " + this->User->Username);
                }
                if (user_info_.attributes().contains("registration"))
                {
                    this->User->RegistrationDate = user_info_.attribute("registration");
                }
                else
                {
                    Syslog::HuggleLogs->WarningLog("Wiki returned no registration time of " + this->User->Username);
                }
            }
            int x = 0;
            while (x < g_.count())
            {
                QDomElement group = g_.at(x).toElement();
                QString gn = group.text();
                if (gn != "*" && gn != "user")
                this->User->Groups.append(gn);
                x++;
            }
            this->Score += (Configuration::HuggleConfiguration->ProjectConfig->ScoreFlag * this->User->Groups.count());
            if (this->User->Groups.contains("bot"))
            {
                // if it's a flagged bot we likely don't need to watch them
                this->Score += Configuration::HuggleConfiguration->ProjectConfig->BotScore;
            }
            // let's delete it now
            this->qUser = nullptr;
        }
    }

    if (this->ProcessingRevs)
    {
        // check if api was processed
        if (!this->qTalkpage->IsProcessed())
        {
            return false;
        }

        if (this->qTalkpage->Result->IsFailed())
        {
            /// \todo LOCALIZE ME
            Huggle::Syslog::HuggleLogs->Log("Unable to retrieve " + this->User->GetTalk() + " warning level will not be scored by it");
        } else
        {
            // parse the talk page now
            QDomDocument d_;
            d_.setContent(this->qTalkpage->Result->Data);
            QDomNodeList rev_ = d_.elementsByTagName("rev");
            QDomNodeList pages_ = d_.elementsByTagName("page");
            bool missing = false;
            if (pages_.count() > 0)
            {
                QDomElement e = pages_.at(0).toElement();
                if (e.attributes().contains("missing"))
                {
                    missing = true;
                }
            }
            // get last id
            if (missing != true && rev_.count() > 0)
            {
                QDomElement e = rev_.at(0).toElement();
                if (e.nodeName() == "rev")
                {
                    if (!e.attributes().contains("timestamp"))
                    {
                        Huggle::Syslog::HuggleLogs->ErrorLog("Talk page timestamp of " + this->User->Username + " couldn't be retrieved, mediawiki returned no data for it");
                    } else
                    {
                        this->TPRevBaseTime = e.attribute("timestamp");
                    }
                    this->User->TalkPage_SetContents(e.text());
                } else
                {
                    /// \todo LOCALIZE ME
                    Huggle::Syslog::HuggleLogs->Log("Unable to retrieve " + this->User->GetTalk() + " warning level will not be scored by it");
                }
            } else
            {
                if (missing)
                {
                    // we set an empty talk page so that we know we do have the contents of this page
                    this->User->TalkPage_SetContents("");
                } else
                {
                    /// \todo LOCALIZE ME
                    Huggle::Syslog::HuggleLogs->Log("Unable to retrieve " + this->User->GetTalk() + " warning level will not be scored by it");
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

        if (this->qDifference->Result->IsFailed())
        {
            // whoa it ended in error, we need to get rid of this edit somehow now
            Huggle::Syslog::HuggleLogs->WarningLog("Failed to obtain diff for " + this->Page->PageName + " the error was: "
                                                   + this->qDifference->Result->Data);
            this->qDifference = nullptr;
            this->PostProcessing = false;
            return true;
        }

        // parse the diff now
        QDomDocument d;
        d.setContent(this->qDifference->Result->Data);
        QDomNodeList l = d.elementsByTagName("rev");
        QDomNodeList diff = d.elementsByTagName("diff");
        // get last id
        if (l.count() > 0)
        {
            QDomElement e = l.at(0).toElement();
            if (e.nodeName() == "rev")
            {
                if (e.text().length() > 0)
                    this->Page->Contents = e.text();
                // check if this revision matches our user
                if (e.attributes().contains("user"))
                {
                    if (e.attribute("user") == this->User->Username)
                    {
                        if (e.attributes().contains("rollbacktoken"))
                        {
                            // let's update it from fresh diff
                            this->RollbackToken = e.attribute("rollbacktoken");
                        }
                    }
                    if (e.attributes().contains("revid"))
                        this->RevID = e.attribute("revid").toInt();
                }
                if (e.attributes().contains("timestamp"))
                    this->Time = MediaWiki::FromMWTimestamp(e.attribute("timestamp"));
                if (e.attributes().contains("comment"))
                    this->Summary = e.attribute("comment");
            }
        }
        if (diff.count() > 0)
        {
            QDomElement e = diff.at(0).toElement();
            this->DiffText = e.text();
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
    if (this->ProcessingRevs || this->ProcessingDiff || this->qUser != nullptr || this->qText != nullptr)
        return false;

    this->qTalkpage = nullptr;
    this->ProcessingByWorkerThread = true;
    ProcessorThread::EditLock.lock();
    this->RegisterConsumer(HUGGLECONSUMER_PROCESSOR);
    ProcessorThread::PendingEdits.append(this);
    ProcessorThread::EditLock.unlock();
    return false;
}

void WikiEdit::ProcessWords()
{
    QString text = this->DiffText.toLower();
    int xx = 0;
    if (this->Page->Contents.length() > 0)
    {
        text = this->Page->Contents.toLower();
    }
    while (xx<Configuration::HuggleConfiguration->ProjectConfig->ScoreParts.count())
    {
        QString w = Configuration::HuggleConfiguration->ProjectConfig->ScoreParts.at(xx).word;
        if (text.contains(w))
        {
            this->Score += Configuration::HuggleConfiguration->ProjectConfig->ScoreParts.at(xx).score;
            ScoreWords.append(w);
        }
        xx++;
    }
    xx = 0;
    while (xx<Configuration::HuggleConfiguration->ProjectConfig->ScoreWords.count())
    {
        QString w = Configuration::HuggleConfiguration->ProjectConfig->ScoreWords.at(xx).word;
        // if there is no such a string in text we can skip it
        if (!text.contains(w))
        {
            xx++;
            continue;
        }
        int SD = 0;
        bool found = false;
        if (text == w)
            found = true;
        while (!found && SD < Configuration::HuggleConfiguration->SystemConfig_WordSeparators.count())
        {
            if (text.startsWith(w + Configuration::HuggleConfiguration->SystemConfig_WordSeparators.at(SD)))
            {
                found = true;
                break;
            }
            if (text.endsWith(Configuration::HuggleConfiguration->SystemConfig_WordSeparators.at(SD) + w))
            {
                found = true;
                break;
            }
            int SL = 0;
            while (SL <Configuration::HuggleConfiguration->SystemConfig_WordSeparators.count())
            {
                if (text.contains(Configuration::HuggleConfiguration->SystemConfig_WordSeparators.at(SD) +
                             w + Configuration::HuggleConfiguration->SystemConfig_WordSeparators.at(SL)))
                {
                    found = true;
                    break;
                }
                if (found)
                    break;
                SL++;
            }
            SD++;
        }
        if (found)
        {
            this->Score += Configuration::HuggleConfiguration->ProjectConfig->ScoreWords.at(xx).score;
            ScoreWords.append(w);
        }
        xx++;
    }
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
        throw new Huggle::NullPointerException("Page", "void WikiEdit::PostProcess()");
    if (this->Status == Huggle::StatusNone)
    {
        Exception::ThrowSoftException("Processing edit to " + this->Page->PageName + "which was requested to be post processed,"\
                                      " but wasn't processed yet", "void WikiEdit::PostProcess()");
        QueryPool::HugglePool->PreProcessEdit(this);
    }
    if (this->Status == Huggle::StatusPostProcessed)
        throw new Huggle::Exception("Unable to post process an edit that is already processed");
    if (this->Status != Huggle::StatusProcessed)
        throw new Huggle::Exception("Unable to post process an edit that wasn't in processed status");
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
            this->qDifference->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding( "ids|user|timestamp|comment" ) +
                                            "&rvlimit=1&rvtoken=rollback&rvstartid=" +
                                            QString::number(this->RevID) + "&rvdiffto=" + this->DiffTo + "&titles=" +
                                            QUrl::toPercentEncoding(this->Page->PageName);
        } else
        {
            this->qDifference->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding( "ids|user|timestamp|comment" ) +
                                            "&rvlimit=1&rvtoken=rollback&rvdiffto=" + this->DiffTo + "&titles=" +
                                            QUrl::toPercentEncoding(this->Page->PageName);
        }
        this->qDifference->Target = Page->PageName;
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
    this->ProcessingRevs = true;
    if (this->User->IsIP())
        return;
    this->qUser = new ApiQuery(ActionQuery, this->GetSite());
    this->qUser->Parameters = "list=users&usprop=blockinfo%7Cgroups%7Ceditcount%7Cregistration&ususers="
                                + QUrl::toPercentEncoding(this->User->Username);
    this->qUser->Process();
}

Collectable_SmartPtr<WikiEdit> WikiEdit::FromCacheByRevID(int revid, QString prev)
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
        x++;
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

WikiSite *WikiEdit::GetSite()
{
    if (this->Page == nullptr)
    {
        // this must never happen
        throw new Huggle::Exception("NULL site", "WikiSite *WikiEdit::GetSite()");
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
            e++;
        }
        PendingEdits.clear();
        ProcessorThread::EditLock.unlock();
        QThread::usleep(200000);
    }
}

void ProcessorThread::Process(WikiEdit *edit)
{
    bool IgnoreWords = false;
    if (edit->IsRevert)
    {
        if (edit->User->IsIP())
        {
            edit->Score += 200;
        } else
        {
            // we have to ignore the score words here because there is always lot of them in revert text
            IgnoreWords = true;
        }
    }
    // score
    if (edit->User->IsIP())
    {
        edit->Score += Configuration::HuggleConfiguration->ProjectConfig->IPScore;
    }
    if (edit->Bot)
        edit->Score += Configuration::HuggleConfiguration->ProjectConfig->BotScore;
    if (edit->Page->IsUserpage() && !edit->Page->SanitizedName().contains(edit->User->Username))
        edit->Score += Configuration::HuggleConfiguration->ProjectConfig->ForeignUser;
    else if (edit->Page->IsUserpage())
        edit->Score += Configuration::HuggleConfiguration->ProjectConfig->ScoreUser;
    if (edit->Page->IsTalk())
        edit->Score += Configuration::HuggleConfiguration->ProjectConfig->ScoreTalk;
    if (edit->Size > 1200 || edit->Size < -1200)
        edit->Score += Configuration::HuggleConfiguration->ProjectConfig->ScoreChange;
    if (edit->Page->IsUserpage())
        IgnoreWords = true;
    if (edit->User->IsWhitelisted())
        edit->Score += Configuration::HuggleConfiguration->ProjectConfig->WhitelistScore;
    edit->Score += edit->User->GetBadnessScore();
    if (!IgnoreWords)
        edit->ProcessWords();
    if (edit->SizeIsKnown && edit->Size < (-1*edit->GetSite()->GetProjectConfig()->LargeRemoval))
        edit->Score += edit->GetSite()->GetProjectConfig()->ScoreRemoval;
    edit->User->ParseTP(QDate::currentDate());
    if (edit->Summary.size() == 0)
        edit->Score += 10;
    switch(edit->User->WarningLevel)
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
    edit->PostProcessing = false;
    edit->ProcessedByWorkerThread = true;
    edit->Status = StatusPostProcessed;
}
