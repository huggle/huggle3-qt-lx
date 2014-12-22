//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "speedyform.hpp"
#include <QMessageBox>
#include "configuration.hpp"
#include "core.hpp"
#include "exception.hpp"
#include "hooks.hpp"
#include "mainwindow.hpp"
#include "generic.hpp"
#include "localization.hpp"
#include "syslog.hpp"
#include "ui_speedyform.h"
#include "wikiuser.hpp"
#include "wikisite.hpp"
#include "wikiutil.hpp"

using namespace Huggle;

SpeedyForm::SpeedyForm(QWidget *parent) : QDialog(parent), ui(new Ui::SpeedyForm)
{
    this->timer = new QTimer(this);
    this->connect(this->timer, SIGNAL(timeout()), this, SLOT(OnTick()));
    this->ui->setupUi(this);
    this->ui->checkBox->setText(_l("speedy-notifycreator"));
    this->ui->label->setText(_l("speedy-reason"));
}

SpeedyForm::~SpeedyForm()
{
    delete this->ui;
}

void SpeedyForm::on_pushButton_clicked()
{
    if (!Hooks::Speedy_BeforeOK(this->edit, this))
        return;
    if (this->edit->Page->IsUserpage())
    {
        QMessageBox::StandardButton qb = QMessageBox::question(Core::HuggleCore->Main, "Request",  _l("delete-user"), QMessageBox::Yes|QMessageBox::No);
        if (qb == QMessageBox::No)
        {
            return;
        }
    }
    if (this->ui->comboBox->currentText().isEmpty())
    {
        Generic::MessageBox(_l("speedy-wrong"), "Wrong csd");
        return;
    }
    this->ui->checkBox->setEnabled(false);
    this->ui->comboBox->setEnabled(false);
    this->ui->label_3->setEnabled(false);
    this->ui->lineEdit->setEnabled(false);
    this->ui->pushButton->setText(_l("speedy-progress", this->edit->Page->PageName));
    this->ui->pushButton->setEnabled(false);
    this->Header = this->ui->comboBox->currentText();
    // first we need to retrieve the content of page if we don't have it already
    this->qObtainText = Generic::RetrieveWikiPageContents(this->edit->Page);
    this->timer->start(200);
    this->qObtainText->Process();
}

void Finalize(Query *result)
{
    SpeedyForm *form = (SpeedyForm*)result->CallbackResult;
    Hooks::Speedy_Finished(form->edit, form->Header, true);
    result->CallbackResult = nullptr;
    result->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
    form->close();
}

void SpeedyForm::Fail(QString reason)
{
    this->qObtainText.Delete();
    this->Template.Delete();
    Generic::MessageBox("Error", reason, MessageBoxStyleError);
    Hooks::Speedy_Finished(this->edit, this->ui->comboBox->currentText(), false);
    this->timer->stop();
}

void SpeedyForm::processTags()
{
    // insert the template to bottom of the page
    QStringList vals = this->edit->GetSite()->GetProjectConfig()->SpeedyTemplates
                       .at(this->ui->comboBox->currentIndex()).split(";");
    if (vals.count() < 4)
    {
        this->Fail(_l("speedy-csd-invalid"));
        return;
    }
    //! \todo make this cross wiki instead of checking random tag
    QString lower = this->Text;
    lower = lower.toLower();
    if (lower.contains("{{db"))
    {
        this->Fail(_l("speedy-csd-existing"));
        this->close();
        return;
    }
    if (this->ReplacePage)
        this->Text = this->ReplacingText;
    // insert a tag to page
    if (this->ui->lineEdit->text().isEmpty())
        this->Text = "{{" + vals.at(2) + "}}\n" + this->Text;
    else
        this->Text = "{{" + vals.at(2) + "|" + this->ui->lineEdit->text() + "}}\n" + this->Text;
    // store a message we later send to user (we need to check if edit is successful first)
    this->warning = vals.at(3);
    // let's modify the page now
    QString summary = this->edit->GetSite()->GetProjectConfig()->SpeedyEditSummary;
    summary.replace("$1", this->edit->Page->PageName);
    this->Template = WikiUtil::EditPage(this->edit->Page, this->Text, summary, false, this->base);
    this->Template->CallbackResult = (void*)this;
    this->Template->callback = (Callback)Finalize;
}

void SpeedyForm::on_pushButton_2_clicked()
{
    this->timer->stop();
    this->close();
}

void SpeedyForm::Init(WikiEdit *edit_)
{
    if (edit_ == nullptr)
    {
        throw new Huggle::NullPointerException("WikiEdit *edit_", BOOST_CURRENT_FUNCTION);
    }
    this->edit = edit_;
    foreach (QString item, this->edit->GetSite()->GetProjectConfig()->SpeedyTemplates)
    {
        // now we need to get first 2 items
        QStringList vals = item.split(";");
        if (vals.count() < 4)
        {
            Huggle::Syslog::HuggleLogs->DebugLog("Invalid csd: " + item);
            continue;
        }
        this->ui->comboBox->addItem(vals.at(0) + ": " + vals.at(1));
    }
    this->ui->label_2->setText(edit_->Page->PageName);
}

QString SpeedyForm::GetSelectedDBReason()
{
    return this->ui->comboBox->currentText();
}

QString SpeedyForm::GetSelectedTagID()
{
    QStringList vals = this->edit->GetSite()->GetProjectConfig()->SpeedyTemplates
                       .at(this->ui->comboBox->currentIndex()).split(";");
    if (vals.count() < 4)
    {
        return "";
    }
    return vals.at(2);
}

void SpeedyForm::SetMessageUserCheck(bool new_value)
{
    this->ui->checkBox->setChecked(new_value);
}

void SpeedyForm::OnTick()
{
    if (this->qObtainText != nullptr)
    {
        if (!this->qObtainText->IsProcessed()) { return; }
        bool failed = false;
        this->Text = Generic::EvaluateWikiPageContents(this->qObtainText, &failed, &this->base);
        if (failed)
        {
            this->Fail(this->Text);
            return;
        }
        this->qObtainText = nullptr;
        this->processTags();
        return;
    }
    if (this->Template != nullptr)
    {
        if (this->Template->IsProcessed())
        {
            if(this->Template->IsFailed())
            {
                this->Fail(_l("speedy-fail", this->Template->Result->ErrorMessage));
                return;
            }
            this->Template = nullptr;
            if (this->ui->checkBox->isChecked())
            {
                QString summary = this->edit->GetSite()->GetProjectConfig()->SpeedyWarningSummary;
                summary.replace("$1", this->edit->Page->PageName);
                this->warning.replace("$1", this->edit->Page->PageName);
                WikiUtil::MessageUser(this->edit->User, this->warning, "", summary, false);
            }
            this->timer->stop();
            this->ui->pushButton->setText(_l("speedy-finished"));
        }
    }
}
