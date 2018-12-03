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
#include <huggle_core/configuration.hpp>
#include <huggle_core/core.hpp>
#include <huggle_core/exception.hpp>
#include <huggle_core/generic.hpp>
#include <huggle_core/localization.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/wikiuser.hpp>
#include <huggle_core/wikisite.hpp>
#include <huggle_core/wikiutil.hpp>
#include "uigeneric.hpp"
#include "uihooks.hpp"
#include "mainwindow.hpp"
#include "ui_speedyform.h"

using namespace Huggle;

SpeedyForm::SpeedyForm(QWidget *parent) : HW("speedyform", this, parent), ui(new Ui::SpeedyForm)
{
    this->timer = new QTimer(this);
    this->connect(this->timer, SIGNAL(timeout()), this, SLOT(OnTick()));
    this->ui->setupUi(this);
    this->ui->checkBox->setText(_l("speedy-notifycreator"));
    this->ui->label->setText(_l("speedy-reason"));
    this->RestoreWindow();
}

SpeedyForm::~SpeedyForm()
{
    delete this->ui;
}

void SpeedyForm::on_pushButton_clicked()
{
    ProjectConfiguration::SpeedyOption speedy_option = this->edit->GetSite()->GetProjectConfig()->SpeedyTemplates.at(this->ui->comboBox->currentIndex());
    if (this->ui->lineEdit->text().isEmpty() && !speedy_option.Parameter.isEmpty())
    {
        UiGeneric::MessageBox(_l("error"), _l("speedy-parameters-fail"), MessageBoxStyleError, false, this);
        return;
    }
    if (!UiHooks::Speedy_BeforeOK(this->edit, this))
        return;
    if (this->edit->Page->IsUserpage())
    {
        QMessageBox::StandardButton qb = QMessageBox::question(MainWindow::HuggleMain, "Request",  _l("delete-user"), QMessageBox::Yes|QMessageBox::No);
        if (qb == QMessageBox::No)
        {
            return;
        }
    }
    if (this->ui->comboBox->currentText().isEmpty())
    {
        UiGeneric::MessageBox(_l("speedy-wrong"), "Wrong csd");
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
    this->qObtainText = WikiUtil::RetrieveWikiPageContents(this->edit->Page);
    this->timer->start(HUGGLE_TIMER);
    this->qObtainText->Process();
}

void Finalize(Query *result)
{
    SpeedyForm *form = reinterpret_cast<SpeedyForm*>(result->CallbackResult);
    UiHooks::Speedy_Finished(form->edit, form->Header, true);
    result->CallbackResult = nullptr;
    result->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
    form->close();
}

void SpeedyForm::Fail(QString reason)
{
    this->qObtainText.Delete();
    this->Template.Delete();
    UiGeneric::MessageBox("Error", reason, MessageBoxStyleError);
    UiHooks::Speedy_Finished(this->edit, this->ui->comboBox->currentText(), false);
    this->timer->stop();
}

void SpeedyForm::processTags()
{
    // insert the template to bottom of the page
    ProjectConfiguration::SpeedyOption speedy_option = this->edit->GetSite()->GetProjectConfig()->SpeedyTemplates.at(this->ui->comboBox->currentIndex());
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
        this->Text = "{{" + speedy_option.Template + "}}\n" + this->Text;
    else
        this->Text = "{{" + speedy_option.Template + "|" + this->ui->lineEdit->text() + "}}\n" + this->Text;
    // store a message we later send to user (we need to check if edit is successful first)
    this->warning = speedy_option.Msg;
    // let's modify the page now
    QString summary = this->edit->GetSite()->GetProjectConfig()->SpeedyEditSummary;
    summary.replace("$1", this->edit->Page->PageName);
    this->Template = WikiUtil::EditPage(this->edit->Page, this->Text, summary, false, this->base);
    this->Template->CallbackResult = reinterpret_cast<void*>(this);
    this->Template->SuccessCallback = reinterpret_cast<Callback>(Finalize);
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
    foreach (ProjectConfiguration::SpeedyOption item, this->edit->GetSite()->GetProjectConfig()->SpeedyTemplates)
    {
        this->ui->comboBox->addItem(item.Tag + ": " + item.Info);
    }
    this->ui->label_2->setText(edit_->Page->PageName);
}

QString SpeedyForm::GetSelectedDBReason()
{
    return this->ui->comboBox->currentText();
}

QString SpeedyForm::GetSelectedTagID()
{
    return this->edit->GetSite()->GetProjectConfig()->SpeedyTemplates.at(this->ui->comboBox->currentIndex()).Template;
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
        this->Text = WikiUtil::EvaluateWikiPageContents(this->qObtainText, &failed, &this->base);
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
                this->Fail(_l("speedy-fail", this->Template->GetFailureReason()));
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

void Huggle::SpeedyForm::on_comboBox_currentIndexChanged(int index)
{
    ProjectConfiguration::SpeedyOption speedy_option = this->edit->GetSite()->GetProjectConfig()->SpeedyTemplates.at(index);
    this->ui->checkBox->setChecked(speedy_option.Notify);
    this->ui->checkBox->setEnabled(speedy_option.Notify);
    this->ui->lineEdit->setText(speedy_option.Parameter);
    this->ui->lineEdit->setEnabled(!speedy_option.Parameter.isEmpty());
}
