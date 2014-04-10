//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "speedyform.hpp"
#include "ui_speedyform.h"

using namespace Huggle;

SpeedyForm::SpeedyForm(QWidget *parent) : QDialog(parent), ui(new Ui::SpeedyForm)
{
    this->edit = NULL;
    this->Template = NULL;
    this->qObtainText = NULL;
    this->timer = new QTimer(this);
    this->connect(this->timer, SIGNAL(timeout()), this, SLOT(OnTick()));
    this->ui->setupUi(this);
    int i=0;
    while (i < Configuration::HuggleConfiguration->ProjectConfig_SpeedyTemplates.count())
    {
        QString item = Configuration::HuggleConfiguration->ProjectConfig_SpeedyTemplates.at(i);
        // now we need to get first 2 items
        QStringList vals = item.split(";");
        if (vals.count() < 4)
        {
            Huggle::Syslog::HuggleLogs->DebugLog("Invalid csd: " + item);
            i++;
            continue;
        }
        this->ui->comboBox->addItem(vals.at(0) + ": " + vals.at(1));
        i++;
    }
}

SpeedyForm::~SpeedyForm()
{
    delete this->ui;
    this->Remove();
}

void SpeedyForm::on_pushButton_clicked()
{
    if (this->ui->comboBox->currentText() == "")
    {
        QMessageBox mb;
        mb.setText("The requested tag is not valid, please choose a different deletion reason");
        mb.setWindowTitle("Wrong csd");
        mb.exec();
        return;
    }
    this->ui->checkBox->setEnabled(false);
    this->ui->comboBox->setEnabled(false);
    this->ui->pushButton->setText("Updating");
    this->ui->pushButton->setEnabled(false);
    // first we need to retrieve the content of page if we don't have it already
    this->qObtainText = Generic::RetrieveWikiPageContents(this->edit->Page->PageName);
    this->timer->start(200);
    this->qObtainText->RegisterConsumer("SpeedyForm");
    this->qObtainText->Process();
}

void SpeedyForm::Remove()
{
    if (this->qObtainText != NULL)
    {
        this->qObtainText->UnregisterConsumer("SpeedyForm");
        this->qObtainText = NULL;
    }
    if (this->Template != NULL)
    {
        this->Template->UnregisterConsumer("SpeedyForm");
        this->Template = NULL;
    }
}

void SpeedyForm::Fail(QString reason)
{
    QMessageBox mb;
    mb.setWindowTitle("Error");
    mb.setText(reason);
    mb.exec();
    this->Remove();
    this->timer->stop();
}

void SpeedyForm::processTags()
{
    // insert the template to bottom of the page
    QStringList vals = Configuration::HuggleConfiguration->ProjectConfig_SpeedyTemplates
                       .at(this->ui->comboBox->currentIndex()).split(";");
    if (vals.count() < 4)
    {
        this->Fail("Invalid CSD tag, there is no message and wiki tag to use");
        return;
    }
    // insert a tag to page
    this->Text = "{{" + vals.at(2) + "}}\n" + this->Text;
    // store a message we later send to user (we need to check if edit is successful first)
    this->warning = vals.at(3);
    // let's modify the page now
    QString summary = Configuration::HuggleConfiguration->ProjectConfig_SpeedyEditSummary;
    summary.replace("$1", this->edit->Page->PageName);
    this->Template = WikiUtil::EditPage(this->edit->Page, this->Text, summary, false, this->base);
    this->Template->RegisterConsumer("SpeedyForm");
}

void SpeedyForm::on_pushButton_2_clicked()
{
    this->timer->stop();
    this->close();
}

void SpeedyForm::Init(WikiEdit *edit_)
{
    if (edit_ == NULL)
    {
        throw new Huggle::Exception("edit was NULL", "void SpeedyForm::Init(WikiEdit *edit_)");
    }
    this->edit = edit_;
    this->ui->label_2->setText(edit_->Page->PageName);
}

void SpeedyForm::OnTick()
{
    if (this->qObtainText != NULL)
    {
        if (!this->qObtainText->IsProcessed()) { return; }
        bool failed = false;
        this->Text = Generic::EvaluateWikiPageContents(this->qObtainText, &failed, &this->base);
        if (failed)
        {
            this->Fail(this->Text);
            return;
        }
        this->qObtainText->UnregisterConsumer("SpeedyForm");
        this->qObtainText = NULL;
        this->processTags();
        return;
    }
    if (this->Template != NULL)
    {
        if (this->Template->IsProcessed())
        {
            if(this->Template->IsFailed())
            {
                this->Fail("Unable to tag the page: " + this->Template->Result->ErrorMessage);
                return;
            }
            this->Template->UnregisterConsumer("SpeedyForm");
            this->Template = NULL;
            if (this->ui->checkBox->isChecked())
            {
                QString summary = Configuration::HuggleConfiguration->ProjectConfig_SpeedyWarningSummary;
                summary.replace("$1", this->edit->Page->PageName);
                this->warning.replace("$1", this->edit->Page->PageName);
                WikiUtil::MessageUser(this->edit->User, this->warning, "", summary, false);
            }
            this->timer->stop();
            this->ui->pushButton->setText("Finished");
        }
    }
}
