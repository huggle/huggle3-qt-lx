//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "speedyform.hpp"
#include <QCloseEvent>
#include <QMessageBox>
#include <QUrl>
#include <huggle_core/apiquery.hpp>
#include <huggle_core/configuration.hpp>
#include <huggle_core/core.hpp>
#include <huggle_core/editquery.hpp>
#include <huggle_core/exception.hpp>
#include <huggle_core/gc.hpp>
#include <huggle_core/generic.hpp>
#include <huggle_core/localization.hpp>
#include <huggle_core/message.hpp>
#include <huggle_core/projectconfiguration.hpp>
#include <huggle_core/query.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/wikiedit.hpp>
#include <huggle_core/wikipage.hpp>
#include <huggle_core/wikisite.hpp>
#include <huggle_core/wikiuser.hpp>
#include <huggle_core/wikiutil.hpp>
#include "mainwindow.hpp"
#include "uigeneric.hpp"
#include "uihooks.hpp"
#include "ui_speedyform.h"

using namespace Huggle;

SpeedyForm::SpeedyForm(QWidget *parent) : HW("speedyform", this, parent), ui(new Ui::SpeedyForm)
{
    this->timer = new QTimer(this);
    connect(this->timer, &QTimer::timeout, this, &SpeedyForm::OnTick);
    this->ui->setupUi(this);
    this->ui->cbSendWarning->setText(_l("speedy-notifycreator"));
    this->ui->lbReason->setText(_l("speedy-reason"));
    this->RestoreWindow();
}

SpeedyForm::~SpeedyForm()
{
    DetachCallback(this->qObtainText);
    DetachCallback(this->Template);
    DetachCallback(this->qFounder);
    delete this->ui;
}

void SpeedyForm::on_btnTag_clicked()
{
    ProjectConfiguration::SpeedyOption speedy_option = this->edit->GetSite()->GetProjectConfig()->SpeedyTemplates.at(this->ui->cbReason->currentIndex());
    if (this->ui->leParameter->text().isEmpty() && !speedy_option.Parameter.isEmpty())
    {
        UiGeneric::MessageBox(_l("error"), _l("speedy-parameters-fail"), MessageBoxStyleError, false, this);
        return;
    }
    if (!UiHooks::Speedy_BeforeOK(this->edit, this))
        return;
    if (this->edit->Page->IsUserpage())
    {
        QMessageBox::StandardButton qb = QMessageBox::question(MainWindow::HuggleMain, _l("request"),  _l("delete-user"), QMessageBox::Yes|QMessageBox::No);
        if (qb == QMessageBox::No)
            return;
    }
    if (this->ui->cbReason->currentText().isEmpty())
    {
        UiGeneric::MessageBox(_l("speedy-wrong"), "Wrong csd");
        return;
    }
    this->ui->cbSendWarning->setEnabled(false);
    this->ui->cbReason->setEnabled(false);
    this->ui->lbParameter->setEnabled(false);
    this->ui->leParameter->setEnabled(false);
    this->ui->btnTag->setText(_l("speedy-progress", this->edit->Page->PageName));
    this->ui->btnTag->setEnabled(false);
    this->ui->btnCancel->setEnabled(false);
    this->Header = this->ui->cbReason->currentText();
    this->busy = true;

    this->qObtainText = WikiUtil::RetrieveWikiPageContents(this->edit->Page);
    this->qObtainText->CallbackOwner = this;
    this->qObtainText->SuccessCallback = &SpeedyForm::ContentSuccess;
    this->qObtainText->FailureCallback = &SpeedyForm::ContentFailure;
    this->qObtainText->Process();
}

void *SpeedyForm::ContentSuccess(Query *query)
{
    SpeedyForm *form = ReleaseCallback(query);
    if (!form)
        return nullptr;

    bool failed = false;
    form->Text = WikiUtil::EvaluateWikiPageContents(static_cast<ApiQuery*>(query), &failed, &form->base);
    form->qObtainText.Delete();
    if (failed)
        form->Fail(form->Text);
    else
        form->processTags();
    return nullptr;
}

void *SpeedyForm::ContentFailure(Query *query)
{
    SpeedyForm *form = ReleaseCallback(query);
    if (!form)
        return nullptr;

    QString reason = query->GetFailureReason();
    form->qObtainText.Delete();
    form->Fail(reason);
    return nullptr;
}

void *SpeedyForm::EditSuccess(Query *query)
{
    SpeedyForm *form = ReleaseCallback(query);
    if (!form)
        return nullptr;

    form->Template.Delete();
    form->tagSucceeded();
    return nullptr;
}

void SpeedyForm::tagSucceeded()
{
    if (!this->ui->cbSendWarning->isChecked())
        this->Finish();
    else if (this->edit->Page->FounderKnown())
        this->notifyFounder(this->edit->Page->GetFounder());
    else
        this->retrieveFounder();
}

void *SpeedyForm::EditFailure(Query *query)
{
    SpeedyForm *form = ReleaseCallback(query);
    if (!form)
        return nullptr;

    QString reason = query->GetFailureReason();
    form->Template.Delete();
    form->Fail(reason);
    return nullptr;
}

void *SpeedyForm::FounderSuccess(Query *query)
{
    SpeedyForm *form = ReleaseCallback(query);
    if (!form)
        return nullptr;

    bool failed = false;
    QString error;
    QString founder = WikiUtil::EvaluatePageFounder(static_cast<ApiQuery*>(query)->GetApiQueryResult(), &failed, &error);
    form->qFounder.Delete();
    if (failed)
    {
        form->Fail(_l("speedy-creator-fail", error), true);
        return nullptr;
    }

    form->edit->Page->SetFounder(founder);
    form->notifyFounder(founder);
    return nullptr;
}

void *SpeedyForm::FounderFailure(Query *query)
{
    SpeedyForm *form = ReleaseCallback(query);
    if (!form)
        return nullptr;

    QString reason = query->GetFailureReason();
    form->qFounder.Delete();
    form->Fail(_l("speedy-creator-fail", reason), true);
    return nullptr;
}

SpeedyForm *SpeedyForm::ReleaseCallback(Query *query)
{
    SpeedyForm *form = static_cast<SpeedyForm*>(query->CallbackOwner);
    query->CallbackOwner = nullptr;
    query->SuccessCallback = nullptr;
    query->FailureCallback = nullptr;
    // Query completion code can continue using the query after invoking its callback.
    QTimer::singleShot(0, [query]()
    {
        query->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
    });
    return form;
}

void SpeedyForm::DetachCallback(Query *query)
{
    if (!query)
        return;
    query->CallbackOwner = nullptr;
    query->SuccessCallback = nullptr;
    query->FailureCallback = nullptr;
}

void SpeedyForm::Fail(const QString &reason, bool pageTagged)
{
    this->busy = false;
    this->timer->stop();
    this->ui->btnTag->setText(_l("speedy-failed"));
    this->ui->btnCancel->setEnabled(true);
    QString messageText = pageTagged ? _l("speedy-notification-fail", reason) : _l("speedy-fail", reason);
    UiGeneric::MessageBox(_l("error"), messageText, MessageBoxStyleError, false, this);
    // The hook reports whether the page was tagged; creator notification is best-effort.
    UiHooks::Speedy_Finished(this->edit, this->Header, pageTagged);
}

void SpeedyForm::Finish()
{
    this->busy = false;
    this->timer->stop();
    this->ui->btnTag->setText(_l("speedy-finished"));
    UiHooks::Speedy_Finished(this->edit, this->Header, true);
    this->close();
}

void SpeedyForm::processTags()
{
    ProjectConfiguration::SpeedyOption speedy_option = this->edit->GetSite()->GetProjectConfig()->SpeedyTemplates.at(this->ui->cbReason->currentIndex());
    QString lower = this->Text.toLower();
    if (lower.contains("{{db"))
    {
        this->Fail(_l("speedy-csd-existing"));
        return;
    }
    if (this->ReplacePage)
        this->Text = this->ReplacingText;
    if (this->ui->leParameter->text().isEmpty())
        this->Text = "{{" + speedy_option.Template + "}}\n" + this->Text;
    else
        this->Text = "{{" + speedy_option.Template + "|" + this->ui->leParameter->text() + "}}\n" + this->Text;
    this->warning = speedy_option.Msg;
    QString summary = this->edit->GetSite()->GetProjectConfig()->SpeedyEditSummary;
    summary.replace("$1", this->edit->Page->PageName);
    this->Template = WikiUtil::EditPage(this->edit->Page, this->Text, summary, false, this->base);
    this->Template->CallbackOwner = this;
    this->Template->SuccessCallback = &SpeedyForm::EditSuccess;
    this->Template->FailureCallback = &SpeedyForm::EditFailure;
    if (this->Template->IsProcessed())
    {
        bool failed = this->Template->IsFailed();
        QString reason;
        if (failed)
            reason = this->Template->GetFailureReason();
        DetachCallback(this->Template);
        this->Template.Delete();
        if (failed)
            this->Fail(reason);
        else
            this->tagSucceeded();
    }
}

void SpeedyForm::retrieveFounder()
{
    this->ui->btnTag->setText(_l("speedy-notify-progress"));
    this->qFounder = new ApiQuery(ActionQuery, this->edit->GetSite());
    this->qFounder->Parameters = "prop=revisions&titles=" + QUrl::toPercentEncoding(this->edit->Page->PageName) +
                                "&rvdir=newer&rvlimit=1&rvprop=" + QUrl::toPercentEncoding("ids|user|timestamp");
    this->qFounder->Target = this->edit->Page->PageName + " (retrieving founder)";
    this->qFounder->CallbackOwner = this;
    this->qFounder->SuccessCallback = &SpeedyForm::FounderSuccess;
    this->qFounder->FailureCallback = &SpeedyForm::FounderFailure;
    this->qFounder->Process();
}

void SpeedyForm::notifyFounder(const QString &founder)
{
    if (founder.isEmpty())
    {
        this->Fail(_l("speedy-creator-empty"), true);
        return;
    }

    this->ui->btnTag->setText(_l("speedy-notify-progress"));
    QString summary = this->edit->GetSite()->GetProjectConfig()->SpeedyWarningSummary;
    summary.replace("$1", this->edit->Page->PageName);
    this->warning.replace("$1", this->edit->Page->PageName);
    WikiUser creator(founder, this->edit->GetSite());
    this->message = WikiUtil::MessageUser(&creator, this->warning, "", summary, false, nullptr, false, false, true);
    if (this->message == nullptr)
    {
        this->Fail(_l("speedy-notification-create-fail"), true);
        return;
    }
    this->timer->start(HUGGLE_TIMER);
}

void SpeedyForm::on_btnCancel_clicked()
{
    if (this->busy)
        return;
    this->timer->stop();
    this->close();
}

void SpeedyForm::Init(WikiEdit *edit_)
{
    if (edit_ == nullptr)
        throw new Huggle::NullPointerException("WikiEdit *edit_", BOOST_CURRENT_FUNCTION);

    this->edit = edit_;
    for (const ProjectConfiguration::SpeedyOption& item : this->edit->GetSite()->GetProjectConfig()->SpeedyTemplates)
        this->ui->cbReason->addItem(item.Tag + ": " + item.Info);
    this->ui->lbInfo->setText(edit_->Page->PageName);
    this->ui->cbSendWarning->setChecked(edit_->GetSite()->GetProjectConfig()->Speedy_WarningOnByDefault);
    if (!edit_->GetSite()->GetProjectConfig()->Speedy_EnableWarnings)
    {
        this->ui->cbSendWarning->setChecked(false);
        this->ui->cbSendWarning->setEnabled(false);
    }
}

QString SpeedyForm::GetSelectedDBReason()
{
    return this->ui->cbReason->currentText();
}

QString SpeedyForm::GetSelectedTagID()
{
    return this->edit->GetSite()->GetProjectConfig()->SpeedyTemplates.at(this->ui->cbReason->currentIndex()).Template;
}

void SpeedyForm::SetMessageUserCheck(bool new_value)
{
    this->ui->cbSendWarning->setChecked(new_value);
}

void SpeedyForm::OnTick()
{
    if (this->message == nullptr || !this->message->IsFinished())
        return;

    if (this->message->IsFailed())
    {
        QString reason = this->message->ErrorText;
        this->message.Delete();
        this->Fail(reason, true);
    } else
    {
        this->message.Delete();
        this->Finish();
    }
}

bool SpeedyForm::IsBusy() const
{
    return this->busy;
}

void SpeedyForm::closeEvent(QCloseEvent *event)
{
    if (this->busy)
    {
        event->ignore();
        return;
    }
    HW::closeEvent(event);
}

void Huggle::SpeedyForm::on_cbReason_currentIndexChanged(int index)
{
    ProjectConfiguration::SpeedyOption speedy_option = this->edit->GetSite()->GetProjectConfig()->SpeedyTemplates.at(index);
    this->ui->cbSendWarning->setChecked(speedy_option.Notify);
    this->ui->cbSendWarning->setEnabled(speedy_option.Notify);
    this->ui->leParameter->setText(speedy_option.Parameter);
    this->ui->leParameter->setEnabled(!speedy_option.Parameter.isEmpty());
}
