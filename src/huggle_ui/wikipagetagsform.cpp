//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "wikipagetagsform.hpp"
#include <QMessageBox>
#include <huggle_core/apiquery.hpp>
#include <huggle_core/configuration.hpp>
#include <huggle_core/exception.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/generic.hpp>
#include <huggle_core/querypool.hpp>
#include <huggle_core/localization.hpp>
#include <huggle_core/wikisite.hpp>
#include <huggle_core/wikiedit.hpp>
#include <huggle_core/wikipage.hpp>
#include <huggle_core/wikiutil.hpp>
#include "uigeneric.hpp"
#include "ui_wikipagetagsform.h"

using namespace Huggle;
WikiPageTagsForm::WikiPageTagsForm(QWidget *parent, WikiPage *wikipage) : HW("wptf", this, parent),  ui(new Ui::WikiPageTagsForm)
{
    this->ui->setupUi(this);
    QStringList header;
    header << "" << _l("tag-tags") << _l("tag-parameter") << _l("description");
    this->ui->tableWidget->setColumnCount(4);
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->checkBox->setText(_l("tag-insertatend"));
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui->tableWidget->setShowGrid(false);
    this->RestoreWindow();
    if (wikipage == nullptr)
        throw new Huggle::NullPointerException("WikiPage *wikipage", BOOST_CURRENT_FUNCTION);
    // we copy the page now
    this->page = new WikiPage(wikipage);
    this->setWindowTitle(_l("tag-title", page->PageName));
    this->ui->checkBox_2->setText(_l("wikipagetagsform-group", this->page->GetSite()->ProjectConfig->GroupTag));

    // Check if page has content, if not retrieve it
    if (this->page->HasContent())
        this->displayTags();
    else
        this->getPageContents();
}

WikiPageTagsForm::~WikiPageTagsForm()
{
    delete this->page;
    delete this->ui;
}

static void Finish(Query *result)
{
    ((WikiPageTagsForm*)result->CallbackResult)->close();
    
    result->DecRef();
    result->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
}

static void Fail(Query *result)
{
    QMessageBox mb;
    mb.setWindowTitle(_l("page-tag-fail"));
    mb.setText(_l("page-tag-error", result->GetFailureReason()));
    mb.exec();
    result->DecRef();
    result->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
}

// This function will remove a tag from page text, it's case
// insensitive and it removes even tags with parameters
static QString ClearTag(QString tag, QString value)
{
    QString temp = value.toUpper();
    QString up = tag.toUpper();
    if (temp.contains("{{" + up))
    {
        int start = temp.indexOf("{{" + up);
        if (!value.mid(start).contains("}}"))
        {
            // this means that there is some syntax error in page, which is not our problem
            // we just skip that broken tag
            return value;
        }
        int end = value.indexOf("}}", start) + 2;
        // if there is a newline we should remove it as well
        if (value.size() > end && value.at(end + 1) == QChar('\n'))
            end++;
        
        value.remove(start, end - start);
    }
    return value;
}

void Huggle::WikiPageTagsForm_FinishRead(Query *result)
{
    ApiQuery *retrieve = (ApiQuery*)result;
    bool failed = false;
    QString t_;
    QString text = WikiUtil::EvaluateWikiPageContents(retrieve, &failed, &t_);
    if (failed)
    {
        UiGeneric::MessageBox(_l("page-tag-fail"), _l("page-tag-error", text));
        result->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
        return;
    }
    WikiPageTagsForm *form = (WikiPageTagsForm *)result->CallbackResult;
    if (form->initializing)
    {
        // We are just loading the form now
        form->page->SetContent(text);
        form->displayTags();
        form->toggleEnable(true);
        result->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
        HUGGLE_DEBUG1("Successfuly got content for " + form->page->PageName);
        return;
    }
    int selected_tags_count = 0;
    QString text2 = text;
    result->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
    for (int i = 0; i < form->ui->tableWidget->rowCount(); i++)
    {
        if (form->CheckBoxes[form->page->GetSite()->GetProjectConfig()->Tags.at(i)]->isChecked())
        {
            selected_tags_count++;
        }
    }
    bool multiple_tags_selected = selected_tags_count > 1;
    // append all tags to top of page or bottom, depending on preference of user, if requested group into one template
    if (form->ui->checkBox_2->isChecked() && !form->ui->checkBox->isChecked() && multiple_tags_selected)
    {
        text = "{{" + retrieve->GetSite()->ProjectConfig->GroupTag + "|";
    }
    else if (form->ui->checkBox_2->isChecked() && form->ui->checkBox->isChecked() && multiple_tags_selected)
    {
        text += "\n{{" + retrieve->GetSite()->ProjectConfig->GroupTag + "|"; //need to endline first in case the article has no extra lines at the bottom
    }
    int current_row = 0;
    while (current_row < form->ui->tableWidget->rowCount())
    {
        if (form->page->GetSite()->GetProjectConfig()->Tags.count() > current_row && form->Arguments.count() > current_row && form->CheckBoxes.count() > current_row)
        {
            QString tag = form->page->GetSite()->GetProjectConfig()->Tags.at(current_row);
            if (!form->CheckBoxes.contains(tag))
                throw new Huggle::Exception("No such a tag in hash table", BOOST_CURRENT_FUNCTION);
            if (form->CheckBoxes[tag]->isChecked())
            {
                if (!form->Arguments.contains(tag))
                    throw new Huggle::Exception("No such a tag in hash table", BOOST_CURRENT_FUNCTION);
                QString key = "{{" + form->page->GetSite()->GetProjectConfig()->Tags.at(current_row) + "}}";
                if (form->page->GetSite()->GetProjectConfig()->TagsArgs.contains(tag) && !form->page->GetSite()->GetProjectConfig()->TagsArgs[tag].isEmpty())
                {
                    if (form->Arguments[tag]->text() == form->page->GetSite()->GetProjectConfig()->TagsArgs[tag])
                    {
                        UiGeneric::pMessageBox(form, "Error", "This tag requires a parameter, but you didn't provide any for it: " + tag);
                        return;
                    }
                    key = "{{" + tag + "|" + form->Arguments[tag]->text() + "}}";
                }
                if (!text.contains(key))
                {
                    // in case there is a different version of this tag in text, we need to remove it first
                    // cannot remove {{multiple issues}} for now
                    if (!form->ui->checkBox->isChecked())
                    {
                        text2 = ClearTag(tag, text2);
                    }
                    else
                    {
                        text = ClearTag(tag, text);
                    }
                    text += "\n" + key;
                }
            }
            else
            {
                text = ClearTag(tag, text);
            }
        }
        current_row++;
    }
    if (form->ui->checkBox_2->isChecked() && !form->ui->checkBox->isChecked() && multiple_tags_selected)
    {
        text += "\n}}\n" + text2;
    }
    else if (multiple_tags_selected)
    {
        text += "\n}}\n";
    }
    Collectable_SmartPtr<EditQuery> e = WikiUtil::EditPage(form->page, text, Configuration::HuggleConfiguration->GenerateSuffix(
                                                                                                                                form->page->GetSite()->GetProjectConfig()->TaggingSummary, form->page->GetSite()->GetProjectConfig()), true, t_);
    e->FailureCallback = (Callback)Fail;
    e->SuccessCallback = (Callback)Finish;
    e->CallbackResult = (void*)form;
    // this is just making sure that this edit will not get deleted by GC, it's removed in callback function
    // it probably isn't needed
    e->IncRef();
    QueryPool::HugglePool->AppendQuery(e);
}

void Huggle::WikiPageTagsForm::on_pushButton_clicked()
{
    this->getPageContents();
}

void WikiPageTagsForm::toggleEnable(bool enable)
{
    this->ui->pushButton->setEnabled(enable);
    this->ui->checkBox->setEnabled(enable);
    this->ui->tableWidget->setEnabled(enable);
    this->ui->checkBox_2->setEnabled(enable);
}

void WikiPageTagsForm::displayTags()
{
    QStringList keys = this->page->GetSite()->GetProjectConfig()->Tags;
    int rx = 0;
    foreach (QString item, keys)
    {
        QString key = item;
        QString description = _l("page-tag-nodescription");
        if (this->page->GetSite()->GetProjectConfig()->TagsDesc.contains(key))
            description = this->page->GetSite()->GetProjectConfig()->TagsDesc[key];
        this->ui->tableWidget->insertRow(rx);
        QCheckBox *Item = new QCheckBox(this);
        if (this->page->Contents.toLower().contains("{{" + key.toLower()))
            Item->setChecked(true);
        if (this->CheckBoxes.contains(key) || this->Arguments.contains(key))
            throw new Huggle::Exception("Tag \""+ key.toLower() + "\" is already in hash list, please check in the config page that it's not duplicated", BOOST_CURRENT_FUNCTION);
        this->CheckBoxes.insert(key, Item);
        QLineEdit *line = new QLineEdit(this);
        this->Arguments.insert(key, line);
        if (this->page->GetSite()->GetProjectConfig()->TagsArgs.contains(key) && !this->page->GetSite()->GetProjectConfig()->TagsArgs[key].isEmpty())
        {
            line->setText(this->page->GetSite()->GetProjectConfig()->TagsArgs[key]);
        }
        else
        {
            line->setText(_l("page-tag-noparameters"));
            line->setEnabled(false);
        }
        this->ui->tableWidget->setCellWidget(rx, 0, Item);
        this->ui->tableWidget->setItem(rx, 1, new QTableWidgetItem("{{" + key + "}}"));
        this->ui->tableWidget->setCellWidget(rx, 2, line);
        this->ui->tableWidget->setItem(rx, 3, new QTableWidgetItem(description));
        rx++;
    }
    // We finished loading this form
    this->initializing = false;
    this->ui->tableWidget->resizeColumnsToContents();
    this->ui->tableWidget->resizeRowsToContents();
}

void WikiPageTagsForm::getPageContents()
{
    // Disable UI until we get the contents of page
    this->toggleEnable(false);
    ApiQuery *retrieve = WikiUtil::RetrieveWikiPageContents(this->page, false);
    retrieve->FailureCallback = (Callback)Fail;
    retrieve->CallbackResult = (void*)this;
    retrieve->SuccessCallback = (Callback)Huggle::WikiPageTagsForm_FinishRead;
    QueryPool::HugglePool->AppendQuery(retrieve);
    retrieve->Process();
}
