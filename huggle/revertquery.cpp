//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "revertquery.hpp"

using namespace Huggle;

RevertQuery::RevertQuery()
{
    this->Type = QueryRevert;
    this->qRevert = NULL;
    this->edit = NULL;
    this->PreflightFinished = false;
    this->RollingBack = false;
    this->timer = NULL;
    this->UsingSR = false;
    this->Token = "";
    this->qRetrieve = NULL;
    this->Summary = "";
    this->MinorEdit = false;
    this->EditQuerySoftwareRollback = NULL;
    this->qPreflight = NULL;
    this->SR_Target = "";
    this->SR_RevID = WIKI_UNKNOWN_REVID;
    this->Timeout = Configuration::HuggleConfiguration->SystemConfig_WriteTimeout;
    this->qSR_PageToken = NULL;
    this->SR_EditToken = "";
}

RevertQuery::RevertQuery(WikiEdit *Edit)
{
    Edit->RegisterConsumer(HUGGLECONSUMER_REVERTQUERY);
    this->Type = QueryRevert;
    this->qRevert = NULL;
    this->edit = Edit;
    this->PreflightFinished = false;
    this->RollingBack = false;
    this->timer = NULL;
    this->qRetrieve = NULL;
    this->IgnorePreflightCheck = false;
    this->UsingSR = false;
    this->EditQuerySoftwareRollback = NULL;
    this->Token = "";
    this->MinorEdit = false;
    this->Summary = "";
    this->qPreflight = NULL;
    this->Timeout = Configuration::HuggleConfiguration->SystemConfig_WriteTimeout;
    this->SR_Target = "";
    this->SR_RevID = WIKI_UNKNOWN_REVID;
    this->qSR_PageToken = NULL;
    this->SR_EditToken = "";
}

RevertQuery::~RevertQuery()
{
    if (this->edit != NULL)
    {
        this->edit->UnregisterConsumer(HUGGLECONSUMER_REVERTQUERY);
        this->edit->UnregisterConsumer("Core::RevertEdit");
    }
    if (this->qSR_PageToken != NULL)
    {
        this->qSR_PageToken->UnregisterConsumer(HUGGLECONSUMER_REVERTQUERY);
        this->qSR_PageToken = NULL;
    }
    if (this->qPreflight != NULL)
    {
        this->qPreflight->UnregisterConsumer(HUGGLECONSUMER_REVERTQUERY);
        this->qPreflight = NULL;
    }
    if (this->qRetrieve != NULL)
    {
        this->qRetrieve->UnregisterConsumer(HUGGLECONSUMER_REVERTQUERY);
        this->qRetrieve = NULL;
    }
    delete this->timer;
}

void RevertQuery::Process()
{
    if (this->Status == StatusProcessing)
    {
        Huggle::Syslog::HuggleLogs->DebugLog("Cowardly refusing to double process the query");
        return;
    }
    this->Status = StatusProcessing;
    if (timer != NULL)
    {
        delete timer;
    }
    this->StartTime = QDateTime::currentDateTime();
    this->timer = new QTimer(this);
    connect(this->timer, SIGNAL(timeout()), this, SLOT(OnTick()));
    this->timer->start(800);
    this->RegisterConsumer("RevertQuery::Timer");
    this->CustomStatus = "Preflight check";
    this->Preflight();
}

void RevertQuery::Kill()
{
    if (this->PreflightFinished && this->qRevert != NULL)
    {
        this->qRevert->Kill();
    } else if (this->qPreflight != NULL)
    {
        this->qPreflight->Kill();
    }
    this->Status = StatusInError;
    if (this->Result == NULL)
    {
        this->Result = new QueryResult();
        this->Result->ErrorMessage = "Killed";
        this->Result->Failed = true;
    }
    if (this->qRetrieve != NULL)
    {
        this->qRetrieve->UnregisterConsumer(HUGGLECONSUMER_REVERTQUERY);
    }
    this->qRetrieve = NULL;
    this->Exit();
}

QString RevertQuery::QueryTargetToString()
{
    return this->edit->Page->PageName;
}

bool RevertQuery::IsProcessed()
{
    if (!this->PreflightFinished)
    {
        return false;
    }

    if (this->Status != StatusDone)
    {
        if (CheckRevert())
        {
            this->Status = StatusDone;
            return true;
        }
        return false;
    }

    return true;
}

void RevertQuery::OnTick()
{
    if (this->Status != StatusDone)
    {
        if (!this->PreflightFinished)
        {
            CheckPreflight();
            return;
        }

        if (!this->RollingBack)
        {
            this->Rollback();
            return;
        }
    }

    if (IsProcessed())
    {
        this->timer->stop();
        this->UnregisterConsumer("RevertQuery::Timer");
    }
}

void RevertQuery::DisplayError(QString error, QString reason)
{
    if (reason == "")
        reason = error;
    Huggle::Syslog::HuggleLogs->ErrorLog(error);
    this->Kill();
    this->Status = StatusDone;
    this->Result = new QueryResult();
    this->Result->ErrorMessage = reason;
    this->Result->Failed = true;
}

QString RevertQuery::GetCustomRevertStatus(QString RevertData)
{
    QDomDocument d;
    d.setContent(RevertData);
    QDomNodeList l = d.elementsByTagName("error");
    if (l.count() > 0)
    {
        if (l.at(0).toElement().attributes().contains("code"))
        {
            QString Error = "";
            Error = l.at(0).toElement().attribute("code");

            if (Error == "alreadyrolled")
            {
                return "Edit was reverted by someone else - skipping";
            }

            if (Error == "onlyauthor")
            {
                return "ERROR: Cannot rollback - page only has one author";
            }
            return "In error (" + Error +")";
        }
    }
    return "Reverted";
}

void RevertQuery::Preflight()
{
    // check if there is more edits in queue
    int x=0;
    bool failed = false;
    bool MadeBySameUser = true;
    WikiEdit::Lock_EditList->lock();
    while (x < WikiEdit::EditList.count())
    {
        WikiEdit *w = WikiEdit::EditList.at(x);
        if (w != this->edit)
        {
            if (w->Page->PageName != this->edit->Page->PageName)
            {
                x++;
                continue;
            }
            if (w->Time > this->edit->Time)
            {
                if (w->User->Username != this->edit->User->Username)
                {
                    MadeBySameUser = false;
                }
                failed = true;
            }
        }
        x++;
    }
    WikiEdit::Lock_EditList->unlock();
    if (failed)
    {
        if (Configuration::HuggleConfiguration->UserConfig_AutomaticallyResolveConflicts)
        {
            if (MadeBySameUser && Configuration::HuggleConfiguration->UserConfig_RevertNewBySame)
            {
                this->IgnorePreflightCheck = true;
                // Conflict resolved: revert all edits including new edits made by same users
                Huggle::Syslog::HuggleLogs->Log(Localizations::HuggleLocalizations->Localize("cr-newer-edits", this->edit->Page->PageName));
            } else
            {
                // Conflict resolved: do not perform any action - there are newer edits
                Huggle::Syslog::HuggleLogs->Log(Localizations::HuggleLocalizations->Localize("cr-stop-new-edit", this->edit->Page->PageName));
                this->Cancel();
                return;
            }
        } else
        {
            QString text;
            if (MadeBySameUser)
            {
                // There are new edits to " + PageName + ", are you sure you want to revert them?
                text = (Huggle::Localizations::HuggleLocalizations->Localize("cr-message-new", this->edit->Page->PageName));
            } else
            {
                // There are new edits made to " + PageName + " by a different user, are you sure you want
                // to revert them all? (it will likely fail anyway because of old token)
                text = (Huggle::Localizations::HuggleLocalizations->Localize("cr-message-not-same", this->edit->Page->PageName));
            }
            QMessageBox::StandardButton re;
            re = QMessageBox::question(Core::HuggleCore->Main, Huggle::Localizations::HuggleLocalizations->Localize("revert-preflightcheck"),
                                       text, QMessageBox::Yes|QMessageBox::No);
            if (re == QMessageBox::No)
            {
                this->Cancel();
                return;
            } else
            {
                this->IgnorePreflightCheck = true;
            }
        }
    }
    this->qPreflight = new ApiQuery();
    this->qPreflight->SetAction(ActionQuery);
    this->qPreflight->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("ids|flags|timestamp|user|userid|size|sha1|comment")
                                 + "&rvlimit=20&titles=" + QUrl::toPercentEncoding(this->edit->Page->PageName);
    this->qPreflight->Process();
}

void RevertQuery::CheckPreflight()
{
    if (this->IgnorePreflightCheck)
    {
        this->PreflightFinished = true;
        return;
    }
    if (this->qPreflight == NULL)
    {
        return;
    }
    if (!this->qPreflight->IsProcessed())
    {
        return;
    }
    if (this->qPreflight->Result->Failed)
    {
        Huggle::Syslog::HuggleLogs->Log(Huggle::Localizations::HuggleLocalizations->Localize("revert-fail-pre-flight",
                                                                                             this->qPreflight->Result->ErrorMessage));
        this->Kill();
        this->Result = new QueryResult();
        this->Status = StatusDone;
        this->Result->Failed = true;
        return;
    }
    QDomDocument d;
    d.setContent(this->qPreflight->Result->Data);
    QDomNodeList l = d.elementsByTagName("rev");
    int x=0;
    bool MadeBySameUser = true;
    bool MultipleEdits = false;
    bool PreviousEditsMadeBySameUser = true;
    bool passed = true;
    while (x < l.count())
    {
        QDomElement e = l.at(x).toElement();
        int RevID = WIKI_UNKNOWN_REVID;
        if (e.attributes().contains("revid"))
        {
            RevID = e.attribute("revid").toInt();
            if (edit->RevID == RevID)
            {
                x++;
                continue;
            }
        } else
        {
            x++;
            continue;
        }
        if (e.attributes().contains("user"))
        {
            QString user = e.attribute("user");
            if (PreviousEditsMadeBySameUser && this->edit->RevID != WIKI_UNKNOWN_REVID && RevID < edit->RevID)
            {
                if (user != this->edit->User->Username)
                {
                    PreviousEditsMadeBySameUser = false;
                }
                MultipleEdits = PreviousEditsMadeBySameUser;
            }
        }
        if (this->edit->RevID != WIKI_UNKNOWN_REVID && RevID > edit->RevID)
        {
            passed = false;
        }
        x++;
    }

    if (MultipleEdits && Configuration::HuggleConfiguration->LocalConfig_ConfirmMultipleEdits)
    {
        passed = false;
    }

    if (!passed)
    {
        QString text = ":)";
        if (MultipleEdits)
        {
            // There are multiple edits by same user are you sure you want to revert them
            text = (Huggle::Localizations::HuggleLocalizations->Localize("cr-message-same", this->edit->Page->PageName));
        } else if (MadeBySameUser)
        {
            // There are newer edits, are you sure you want to revert them
            text = (Huggle::Localizations::HuggleLocalizations->Localize("cr-message-new", this->edit->Page->PageName));
        } else
        {
            text = (Huggle::Localizations::HuggleLocalizations->Localize("cr-message-not-same", this->edit->Page->PageName));
        }
        if (Configuration::HuggleConfiguration->UserConfig_AutomaticallyResolveConflicts)
        {
            if (MultipleEdits && !Configuration::HuggleConfiguration->RevertOnMultipleEdits)
            {
                /// \todo LOCALIZE ME
                Huggle::Syslog::HuggleLogs->Log("Conflict resolved: do not perform any action - there are multiple edits by same user to " + this->edit->Page->PageName);
                this->Cancel();
                return;
            } else if (MultipleEdits && Configuration::HuggleConfiguration->RevertOnMultipleEdits)
            {
                /// \todo LOCALIZE ME
                Huggle::Syslog::HuggleLogs->Log("Conflict resolved: revert all edits - there are multiple edits by same user to " + this->edit->Page->PageName);
            } else
            {
                if (PreviousEditsMadeBySameUser && Configuration::HuggleConfiguration->UserConfig_RevertNewBySame)
                {
                    Huggle::Syslog::HuggleLogs->Log("Conflict resolved: revert all edits including new edits, "\
                                                    "made by same users - edits are by same user: " + this->edit->Page->PageName);
                } else
                {
                    /// \todo LOCALIZE ME
                    Huggle::Syslog::HuggleLogs->Log("Conflict resolved: do not perform any action - there are newer edits to " + this->edit->Page->PageName);
                    this->Cancel();
                    return;
                }
            }
        }
        QMessageBox::StandardButton re;
        re = QMessageBox::question(Core::HuggleCore->Main, Huggle::Localizations::HuggleLocalizations->Localize("revert-preflightcheck"),
                                   text, QMessageBox::Yes|QMessageBox::No);
        if (re == QMessageBox::No)
        {
            // abort
            this->Exit();
            this->CustomStatus = "Stopped";
            this->Result = new QueryResult();
            this->Result->Failed = true;
            this->Result->ErrorMessage = "User requested to abort this";
            this->Status = StatusDone;
            this->PreflightFinished = true;
            return;
        }
    }

    this->PreflightFinished = true;
}

bool RevertQuery::CheckRevert()
{
    if (this->UsingSR)
    {
        return ProcessRevert();
    }
    if (this->qRevert == NULL)
    {
        return false;
    }
    if (!this->qRevert->IsProcessed())
    {
        return false;
    }
    this->CustomStatus = RevertQuery::GetCustomRevertStatus(this->qRevert->Result->Data);
    if (this->CustomStatus != "Reverted")
    {
        Huggle::Syslog::HuggleLogs->Log(Localizations::HuggleLocalizations->Localize("revert-fail", this->qRevert->Target, this->CustomStatus));
        this->qRevert->Result->Failed = true;
        this->qRevert->Result->ErrorMessage = CustomStatus;
        this->Result = new QueryResult();
        this->Result->ErrorMessage = CustomStatus;
        this->Result->Failed = true;
    } else
    {
        HistoryItem item;
        this->Result = new QueryResult();
        this->Result->Data = this->qRevert->Result->Data;
        item.Target = this->qRevert->Target;
        item.Type = HistoryRollback;
        item.Result = "Success";
        if (Core::HuggleCore->Main != NULL)
        {
            Core::HuggleCore->Main->_History->Prepend(item);
        }
    }
    this->qRevert->UnregisterConsumer(HUGGLECONSUMER_REVERTQUERY);
    this->qRevert = NULL;
    return true;
}

void RevertQuery::Cancel()
{
    this->Exit();
    this->CustomStatus = "Stopped";
    this->Result = new QueryResult();
    this->Result->Failed = true;
    this->Result->ErrorMessage = "User requested to abort this";
    this->Status = StatusDone;
    this->PreflightFinished = true;
}

bool RevertQuery::ProcessRevert()
{
    if (this->SR_EditToken == "" && this->qSR_PageToken == NULL)
    {
        // we need to obtain edit token on beginning so that we prevent edit conflict resolution
        this->qSR_PageToken = new ApiQuery();
        this->qSR_PageToken->SetAction(ActionQuery);
        this->qSR_PageToken->Parameters = "prop=info&intoken=edit&titles=" + QUrl::toPercentEncoding(this->edit->Page->PageName);
        this->qSR_PageToken->Target = Localizations::HuggleLocalizations->Localize("editquery-token", this->edit->Page->PageName);
        this->qSR_PageToken->RegisterConsumer(HUGGLECONSUMER_REVERTQUERY);
        this->CustomStatus = "Retrieving token";
        Core::HuggleCore->AppendQuery(this->qSR_PageToken);
        this->qSR_PageToken->Process();
        return false;
    }

    if (this->qSR_PageToken != NULL)
    {
        if (!this->qSR_PageToken->IsProcessed())
        {
            return false;
        }

        if (this->qSR_PageToken->IsFailed())
        {
            this->DisplayError("Unable to fetch the token - query failed");
            return true;
        }

        QDomDocument d;
        d.setContent(this->qSR_PageToken->Result->Data);
        QDomNodeList l = d.elementsByTagName("page");
        if (l.count() == 0)
        {
            this->DisplayError("Unable to fetch the token no page info returned by wiki");
            return true;
        }
        QDomElement element = l.at(0).toElement();
        if (!element.attributes().contains("edittoken"))
        {
            this->DisplayError("Unable to get a token there was no token returned by wiki");
            return true;
        }
        this->SR_EditToken = element.attribute("edittoken");
        if (this->SR_EditToken == "")
        {
            // invalid token
            this->DisplayError("Invalid token");
            return true;
        }

        this->qSR_PageToken->UnregisterConsumer(HUGGLECONSUMER_REVERTQUERY);
        this->qSR_PageToken = NULL;
    }

    if (this->EditQuerySoftwareRollback != NULL)
    {
        // we already reverted the page so check if we were successful in that
        if (this->EditQuerySoftwareRollback->IsProcessed() == false)
        {
            // we are still reverting the page so quit this and wait
            return false;
        }
        this->EditQuerySoftwareRollback->UnregisterConsumer(HUGGLECONSUMER_REVERTQUERY);
        this->Result = new QueryResult();
        if (this->EditQuerySoftwareRollback->Result->Failed || this->EditQuerySoftwareRollback->Status == Huggle::StatusInError)
        {
            // failure during revert
            this->Result->Failed = true;
            this->Result->ErrorMessage = this->EditQuerySoftwareRollback->Result->ErrorMessage;
            Huggle::Syslog::HuggleLogs->ErrorLog(Localizations::HuggleLocalizations->Localize("revert-fail", this->edit->Page->PageName,
                                                                                              "edit failed"));
            this->Kill();
            this->Status = StatusDone;
        }
        Syslog::HuggleLogs->DebugLog("Sucessful SR of page " + this->edit->Page->PageName);
        return true;
    }

    if (this->qRetrieve != NULL)
    {
        // we are retrieving the content of previous edit made by a different user
        if (!this->qRetrieve->IsProcessed())
        {
            return false;
        }

        if (this->qRetrieve->IsFailed())
        {
            this->DisplayError("Unable to rollback the edit because previous content couldn't be retrieved");
            return true;
        }

        QString summary = this->Summary;
        QString content = "";

        QDomDocument d;
        d.setContent(this->qRetrieve->Result->Data);
        QDomNodeList l = d.elementsByTagName("rev");

        if (l.count() == 0)
        {
            this->DisplayError("Unable to rollback the edit because previous content couldn't be retrieved");
            return true;
        }

        QDomElement element = l.at(0).toElement();
        if (!element.attributes().contains("revid"))
        {
            this->DisplayError("Unable to rollback the edit because query used to retrieve the content of previous"\
                               " version retrieved no RevID");
            return true;
        }

        QString rv = element.attribute("revid");

        if (rv.toInt() != this->SR_RevID)
        {
            this->DisplayError("Unable to rollback the edit because query used to retrieve the content of previous"\
                               " version returned invalid RevID");
            return true;
        }

        content = element.text();

        if (summary == "")
        {
            summary = Configuration::HuggleConfiguration->LocalConfig_SoftwareRevertDefaultSummary;
        }
        summary = summary.replace("$1", this->edit->User->Username)
                .replace("$2", this->SR_Target)
                .replace("$3", QString::number(this->SR_Depth))
                .replace("$4", QString::number(this->SR_RevID));
        // we need to make sure there is edit suffix in revert summary for huggle
        summary = Huggle::Configuration::HuggleConfiguration->GenerateSuffix(summary);
        if (content == "")
        {
            /// \todo LOCALIZE ME
            this->DisplayError("Cowardly refusing to blank \"" + this->edit->Page->PageName + "\" software rollback was cancelled to prevent damage",
                               "content was resolved to blank edit");
            return true;
        }
        this->EditQuerySoftwareRollback = Core::HuggleCore->EditPage(this->edit->Page, content, summary, this->MinorEdit);
        this->EditQuerySoftwareRollback->RegisterConsumer(HUGGLECONSUMER_REVERTQUERY);
        /// \todo LOCALIZE ME
        this->CustomStatus = "Editing page";
        return false;
    }

    if (this->qPreflight == NULL)
    {
        return false;
    }

    if (this->qPreflight->IsProcessed() != true)
    {
        return false;
    }

    if (this->qPreflight->Result->Failed)
    {
        /// \todo LOCALIZE ME
        this->DisplayError("Failed to retrieve a list of edits made to this page: " + this->qPreflight->Result->ErrorMessage);
        return true;
    }
    QDomDocument d;
    d.setContent(this->qPreflight->Result->Data);
    QDomNodeList l = d.elementsByTagName("rev");
    // we need to find a first revision that is made by a different user
    // but first we need to check if last revision was actually made by this user, because if not
    // it's possible that someone else already reverted them
    if (l.count() == 0)
    {
        /// \todo LOCALIZE ME
        // if we have absolutely no revisions in the result, it's pretty fucked
        this->DisplayError("Failed to retrieve a list of edits made to this page, query returned no data");
        return true;
    }
    // if the latest revid doesn't match our revid it means that someone made an edit
    bool passed = true;
    this->SR_Depth = 0;
    int x = 0;
    while (x < l.count())
    {
        QDomElement e = l.at(x).toElement();
        if (e.attributes().contains("revid"))
        {
            if (edit->RevID == e.attribute("revid").toInt())
            {
                x++;
                continue;
            }
        } else
        {
            x++;
            continue;
        }
        if (this->edit->RevID != WIKI_UNKNOWN_REVID && e.attribute("revid").toInt() > edit->RevID)
        {
            passed = false;
        }
        x++;
    }
    if (!passed)
    {
        this->DisplayError("Unable to revert the page " + this->edit->Page->PageName + " because it was edited meanwhile");
        return true;
    }

    // now we need to find the first revision that was done by some different user
    x = 0;
    // FIXME: this list needs to be sorted by RevID
    while (x < l.count())
    {
        QDomElement e = l.at(x).toElement();
        if (!e.attributes().contains("revid") || !e.attributes().contains("user"))
        {
            // this is fucked up piece of shit
            this->DisplayError("Unable to revert the page " + this->edit->Page->PageName + " because mediawiki returned some non-sense");
            Huggle::Syslog::HuggleLogs->DebugLog("Nonsense: " + this->qPreflight->Result->Data);
            return true;
        }
        if (e.attribute("user") != this->edit->User->Username)
        {
            // we got it
            this->SR_RevID = e.attribute("revid").toInt();
            this->SR_Target = e.attribute("user");
            break;
        }
        this->SR_Depth++;
        x++;
    }
    // let's check if depth isn't too low
    if (this->SR_Depth == 0)
    {
        // something is wrong
        this->DisplayError(Localizations::HuggleLocalizations->Localize("revert-fail", this->edit->Page->PageName,
                                                                        "because it was edited meanwhile"));
        return true;
    }

    if (this->SR_RevID == WIKI_UNKNOWN_REVID)
    {
        this->DisplayError(Localizations::HuggleLocalizations->Localize("revert-fail", this->edit->Page->PageName,
                                                                        "because no previous version could be retrieved"));
        return true;
    }

    this->CustomStatus = "Retrieving content of previous version";
    // now we need to get the content of page
    this->qRetrieve = new ApiQuery();
    this->qRetrieve->RegisterConsumer(HUGGLECONSUMER_REVERTQUERY);
    this->qRetrieve->Parameters = "prop=revisions&revids=" + QString::number(this->SR_RevID) + "&rvprop=" +
                                  QUrl::toPercentEncoding("ids|content");
    this->qRetrieve->SetAction(ActionQuery);
    this->qRetrieve->Process();
    return false;
}

void RevertQuery::Rollback()
{
    if (this->RollingBack)
    {
        // wtf happened
        Huggle::Syslog::HuggleLogs->DebugLog("Multiple request to rollback same query");
        return;
    }
    this->RollingBack = true;

    if (this->Summary == "")
    {
        this->Summary = Configuration::HuggleConfiguration->LocalConfig_RollbackSummaryUnknownTarget;
    }

    if (this->Summary.contains("$1"))
    {
        this->Summary = this->Summary.replace("$1", edit->User->Username);
    }

    // we need to make sure there is edit suffix in revert summary for huggle
    this->Summary = Huggle::Configuration::HuggleConfiguration->GenerateSuffix(this->Summary);

    this->edit->User->SetBadnessScore(this->edit->User->GetBadnessScore() + 200);
    WikiUser::UpdateUser(edit->User);

    if (this->UsingSR)
    {
        this->Revert();
        return;
    }
    if (!Configuration::HuggleConfiguration->Rights.contains("rollback"))
    {
        /// \todo LOCALIZE ME
        Huggle::Syslog::HuggleLogs->Log("You don't have rollback rights, fallback to software rollback");
        this->UsingSR = true;
        this->Revert();
        return;
    }

    if (this->Token == "")
    {
        this->Token = this->edit->RollbackToken;
    }

    if (this->Token == "")
    {
        Huggle::Syslog::HuggleLogs->ErrorLog(Localizations::HuggleLocalizations->Localize("revert-fail", this->edit->Page->PageName,
                                                                                          "rollback token was empty"));
        this->Result = new QueryResult();
        this->Result->Failed = true;
        this->Result->ErrorMessage = Localizations::HuggleLocalizations->Localize("revert-fail", this->edit->Page->PageName,
                                                                                  "rollback token was empty");
        this->Status = StatusDone;
        this->Exit();
        return;
    }
    this->qRevert = new ApiQuery();
    this->qRevert->SetAction(ActionRollback);
    QString token = this->Token;
    if (token.endsWith("+\\"))
    {
        token = QUrl::toPercentEncoding(token);
    }
    this->qRevert->Parameters = "title=" + QUrl::toPercentEncoding(edit->Page->PageName)
                + "&token=" + token
                + "&user=" + QUrl::toPercentEncoding(edit->User->Username)
                + "&summary=" + QUrl::toPercentEncoding(this->Summary);
    this->qRevert->Target = edit->Page->PageName;
    this->qRevert->UsingPOST = true;
    this->qRevert->RegisterConsumer(HUGGLECONSUMER_REVERTQUERY);
    if (Configuration::HuggleConfiguration->Verbosity > 0)
    {
        Core::HuggleCore->AppendQuery(this->qRevert);
    }
    /// \todo LOCALIZE ME
    this->CustomStatus = "Rolling back " + edit->Page->PageName;
    Huggle::Syslog::HuggleLogs->DebugLog("Rolling back " + edit->Page->PageName);
    this->qRevert->Process();
}

void RevertQuery::Revert()
{
    // Get a list of edits made to this page
    this->qPreflight = new ApiQuery();
    this->qPreflight->SetAction(ActionQuery);
    this->qPreflight->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("ids|flags|timestamp|user|userid|content|size|sha1|comment")
                                    + "&rvlimit=20&titles=" + QUrl::toPercentEncoding(this->edit->Page->PageName);
    this->qPreflight->Process();
}

void RevertQuery::Exit()
{
    if (this->timer != NULL)
    {
        this->timer->stop();
    }
    if (this->EditQuerySoftwareRollback != NULL)
    {
        this->EditQuerySoftwareRollback->UnregisterConsumer(HUGGLECONSUMER_REVERTQUERY);
        this->EditQuerySoftwareRollback = NULL;
    }
    this->UnregisterConsumer("RevertQuery::Timer");
    if (this->qRevert != NULL)
    {
        this->qRevert->UnregisterConsumer(HUGGLECONSUMER_REVERTQUERY);
    }
}
