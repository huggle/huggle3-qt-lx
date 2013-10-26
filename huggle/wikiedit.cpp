//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikiedit.h"
using namespace Huggle;
QList<WikiEdit*> WikiEdit::EditList;

WikiEdit::WikiEdit()
{
    this->Bot = false;
    this->User = NULL;
    this->Minor = false;
    this->NewPage = false;
    this->Size = 0;
    this->User = NULL;
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
    this->DifferenceQuery = NULL;
    this->ProcessingQuery = NULL;
    this->ProcessingDiff = false;
    this->ProcessingRevs = false;
    this->DiffText = "";
    this->Priority = 20;
    this->Score = 0;
    this->Previous = NULL;
    this->Time = QDateTime::currentDateTime();
    this->Enqueued = false;
    this->Next = NULL;
    this->ProcessingByWorkerThread = false;
    this->ProcessedByWorkerThread = false;
    this->RevID = WIKI_UNKNOWN_REVID;
    WikiEdit::EditList.append(this);
}

WikiEdit::WikiEdit(const WikiEdit &edit)
{
    this->Enqueued = edit.Enqueued;
    this->User = NULL;
    this->Page = NULL;
    this->Bot = edit.Bot;
    this->Minor = edit.Minor;
    this->NewPage = edit.NewPage;
    if (edit.Page != NULL)
    {
        this->Page = new WikiPage(edit.Page);
    }
    this->Size = edit.Size;
    if (edit.User != NULL)
    {
        this->User = new WikiUser(edit.User);
    }
    this->Diff = edit.Diff;
    this->OldID = edit.OldID;
    this->CurrentUserWarningLevel = edit.CurrentUserWarningLevel;
    this->Summary = edit.Summary;
    this->Status = edit.Status;
    this->RollbackToken = edit.RollbackToken;
    this->OwnEdit = edit.OwnEdit;
    this->EditMadeByHuggle = edit.EditMadeByHuggle;
    this->Time = edit.Time;
    this->TrustworthEdit = edit.TrustworthEdit;
    this->PostProcessing = false;
    this->ProcessingQuery = NULL;
    this->DifferenceQuery = NULL;
    this->ProcessingDiff = false;
    this->ProcessingRevs = false;
    this->DiffText = edit.DiffText;
    this->Priority = edit.Priority;
    this->Previous = NULL;
    this->Next = NULL;
    this->Score = edit.Score;
    this->ProcessingByWorkerThread = false;
    this->ProcessedByWorkerThread = false;
    this->RevID = edit.RevID;
    WikiEdit::EditList.append(this);
}

WikiEdit::WikiEdit(WikiEdit *edit)
{
    this->Enqueued = edit->Enqueued;
    this->User = NULL;
    this->Page = NULL;
    this->Bot = edit->Bot;
    this->Minor = edit->Minor;
    this->NewPage = edit->NewPage;
    if (edit->Page != NULL)
    {
        this->Page = new WikiPage(edit->Page);
    }
    this->Size = edit->Size;
    if (edit->User != NULL)
    {
        this->User = new WikiUser(edit->User);
    }
    this->OldID = edit->OldID;
    this->CurrentUserWarningLevel = edit->CurrentUserWarningLevel;
    this->Summary = edit->Summary;
    this->Diff = edit->Diff;
    this->OwnEdit = edit->OwnEdit;
    this->Status = edit->Status;
    this->RollbackToken = edit->RollbackToken;
    this->EditMadeByHuggle = edit->EditMadeByHuggle;
    this->Time = edit->Time;
    this->TrustworthEdit = edit->TrustworthEdit;
    this->PostProcessing = false;
    this->ProcessingQuery = NULL;
    this->DifferenceQuery = NULL;
    this->ProcessingDiff = false;
    this->ProcessingRevs = false;
    this->DiffText = edit->DiffText;
    this->Priority = edit->Priority;
    this->Previous = NULL;
    this->Next = NULL;
    this->Score = edit->Score;
    this->ProcessingByWorkerThread = false;
    this->ProcessedByWorkerThread = false;
    WikiEdit::EditList.append(this);
}

WikiEdit::~WikiEdit()
{
    WikiEdit::EditList.removeAll(this);
    delete this->User;
    delete this->Page;
}

bool WikiEdit::FinalizePostProcessing()
{
    if (this->ProcessedByWorkerThread)
    {
        return true;
    }

    if (this->ProcessingByWorkerThread)
    {
        return false;
    }

    if (!this->PostProcessing)
    {
        return true;
    }

    if (this->ProcessingRevs)
    {
        // check if api was processed
        if (!this->ProcessingQuery->Processed())
        {
            return false;
        }

        if (this->ProcessingQuery->Result->Failed)
        {
            Core::Log("Unable to retrieve " + this->User->GetTalk() + " warning level will not be scored by it");
        } else
        {
            // parse the diff now
            QDomDocument d;
            d.setContent(this->ProcessingQuery->Result->Data);
            QDomNodeList page = d.elementsByTagName("rev");
            QDomNodeList code = d.elementsByTagName("page");
            bool missing = false;
            if (code.count() > 0)
            {
                QDomElement e = code.at(0).toElement();
                if (e.attributes().contains("missing"))
                {
                    missing = true;
                }
            }
            // get last id
            if (missing != true && page.count() > 0)
            {
                QDomElement e = page.at(0).toElement();
                if (e.nodeName() == "rev")
                {
                    this->User->ContentsOfTalkPage = e.text();
                } else
                {
                    Core::Log("Unable to retrieve " + this->User->GetTalk() + " warning level will not be scored by it");
                }
            } else
            {
                if (!missing)
                {
                    Core::Log("Unable to retrieve " + this->User->GetTalk() + " warning level will not be scored by it");
                    Core::DebugLog(this->ProcessingQuery->Result->Data);
                }
            }
        }
        this->ProcessingRevs = false;
    }

    if (this->ProcessingDiff)
    {
        // check if api was processed
        if (!this->DifferenceQuery->Processed())
        {
            return false;
        }

        if (this->DifferenceQuery->Result->Failed)
        {
            // whoa it ended in error, we need to get rid of this edit somehow now
            this->DifferenceQuery->UnregisterConsumer("WikiEdit::PostProcess()");
            this->DifferenceQuery = NULL;
            this->PostProcessing = false;
            return true;
        }

        // parse the diff now
        QDomDocument d;
        d.setContent(this->DifferenceQuery->Result->Data);
        QDomNodeList l = d.elementsByTagName("rev");
        QDomNodeList diff = d.elementsByTagName("diff");
        // get last id
        if (l.count() > 0)
        {
            QDomElement e = l.at(0).toElement();
            if (e.nodeName() == "rev")
            {
                if (e.text() != "")
                {
                    this->Page->Contents = e.text();
                }
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
                    {
                        this->RevID = e.attribute("revid").toInt();
                    }
                }
                if (e.attributes().contains("comment"))
                {
                    this->Summary = e.attribute("comment");
                }
            }
        }
        if (diff.count() > 0)
        {
            QDomElement e = diff.at(0).toElement();
            if (e.nodeName() == "diff")
            {
                this->DiffText = e.text();
            }
        } else
        {
            Core::DebugLog("Failed to obtain diff for " + this->Page->PageName + " the error was: " + DifferenceQuery->Result->Data);
        }
        // we are done processing the diff
        this->ProcessingDiff = false;
    }

    // check if everything was processed and clean up
    if (this->ProcessingRevs || this->ProcessingDiff)
    {
        return false;
    }

    if (this->DiffText == "")
    {
        Core::Log("ERROR: no diff available for " + this->Page->PageName + " unable to rescore");
    }

    this->ProcessingQuery->UnregisterConsumer("WikiEdit::PostProcess()");
    this->ProcessingQuery = NULL;
    this->DifferenceQuery->UnregisterConsumer("WikiEdit::PostProcess()");
    this->DifferenceQuery = NULL;
    this->ProcessingByWorkerThread = true;
    ProcessorThread::EditLock.lock();
    ProcessorThread::PendingEdits.append(this);
    ProcessorThread::EditLock.unlock();
    return false;
}

void WikiEdit::ProcessWords()
{
    int xx = 0;
    QString text = this->DiffText.toLower();
    if (this->Page->Contents != "")
    {
        text = this->Page->Contents.toLower();
    }
    while (xx<Configuration::LocalConfig_ScoreParts.count())
    {
        QString w = Configuration::LocalConfig_ScoreParts.at(xx).word;
        if (text.contains(w))
        {
            this->Score += Configuration::LocalConfig_ScoreParts.at(xx).score;
            ScoreWords.append(w);
        }
        xx++;
    }
    xx = 0;
    while (xx<Configuration::LocalConfig_ScoreWords.count())
    {
        QString w = Configuration::LocalConfig_ScoreWords.at(xx).word;
        if (text.contains(" " + w + " ") || text.contains(" " + w + ".")
                || text.contains(" " + w + ",") || text.contains(" " + w + "!")
                || text.contains(" " + w + "\n") || text.contains("\n" + w + "\n")
                || text.contains("\n" + w + " "))
        {
            this->Score += Configuration::LocalConfig_ScoreWords.at(xx).score;
            ScoreWords.append(w);
        }
        xx++;
    }
}

void WikiEdit::PostProcess()
{
    if (this->PostProcessing)
    {
        return;
    }
    this->PostProcessing = true;
    this->ProcessingQuery = new ApiQuery();
    this->ProcessingQuery->SetAction(ActionQuery);
    this->ProcessingQuery->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") + "&titles=" +
            QUrl::toPercentEncoding(this->User->GetTalk());
    this->ProcessingQuery->RegisterConsumer("WikiEdit::PostProcess()");
    Core::AppendQuery(this->ProcessingQuery);
    this->ProcessingQuery->Target = "Retrieving tp " + this->User->GetTalk();
    this->ProcessingQuery->Process();
    this->DifferenceQuery = new ApiQuery();
    this->DifferenceQuery->SetAction(ActionQuery);
    if (this->RevID != -1)
    {
        // &rvprop=content can't be used because of fuck up of mediawiki
        this->DifferenceQuery->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding( "ids|user|timestamp|comment" ) + "&rvlimit=1&rvtoken=rollback&rvstartid=" +
                                  QString::number(this->RevID) + "&rvdiffto=prev&titles=" +
                                  QUrl::toPercentEncoding(this->Page->PageName);
    } else
    {
        this->DifferenceQuery->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding( "ids|user|timestamp|comment" ) + "&rvlimit=1&rvtoken=rollback&rvdiffto=prev&titles=" +
            QUrl::toPercentEncoding(this->Page->PageName);
    }
    this->DifferenceQuery->Target = Page->PageName;
    //this->DifferenceQuery->UsingPOST = true;
    Core::AppendQuery(this->DifferenceQuery);
    this->DifferenceQuery->RegisterConsumer("WikiEdit::PostProcess()");
    this->DifferenceQuery->Process();
    this->ProcessingDiff = true;
    this->ProcessingRevs = true;
}

QString WikiEdit::GetFullUrl()
{
    return Core::GetProjectScriptURL() + "index.php?title=" + QUrl::toPercentEncoding(this->Page->PageName) +
            "&diff=" + QString::number(this->RevID);
}

bool WikiEdit::IsPostProcessed()
{
    if (this->Status == StatusPostProcessed)
    {
        return true;
    }
    return false;
}

QMutex ProcessorThread::EditLock;
QList<WikiEdit*> ProcessorThread::PendingEdits;

void ProcessorThread::run()
{
    while(Core::Running)
    {
        ProcessorThread::EditLock.lock();
        int e=0;
        while (e<ProcessorThread::PendingEdits.count())
        {
            this->Process(PendingEdits.at(e));
            e++;
        }
        PendingEdits.clear();
        ProcessorThread::EditLock.unlock();
        QThread::usleep(200000);
    }
}

void ProcessorThread::Process(WikiEdit *edit)
{
    // score
    if (edit->User->IsIP())
    {
        edit->Score += Configuration::LocalConfig_IPScore;
    }
    if (edit->Bot)
    {
        edit->Score += Configuration::LocalConfig_BotScore;
    }
    if (edit->Page->IsUserpage())
    {
        edit->Score -= 200;
    }
    if (edit->Page->IsTalk())
    {
        edit->Score -= 2000;
    }
    if (edit->Size > 1200 || edit->Size < -1200)
    {
        edit->Score += Configuration::LocalConfig_ScoreChange;
    }

    edit->Score += edit->User->BadnessScore;

    edit->ProcessWords();

    if (edit->User->ContentsOfTalkPage != "")
    {
        edit->User->WarningLevel = Core::GetLevel(edit->User->ContentsOfTalkPage);
    }

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

    edit->Status = StatusPostProcessed;
    edit->PostProcessing = false;
    edit->ProcessedByWorkerThread = true;
}

