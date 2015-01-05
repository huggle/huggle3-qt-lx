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
#include "apiquery.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "syslog.hpp"
#include "generic.hpp"
#include "querypool.hpp"
#include "localization.hpp"
#include "wikisite.hpp"
#include "wikiedit.hpp"
#include "wikipage.hpp"
#include "wikiutil.hpp"
#include "ui_wikipagetagsform.h"

using namespace Huggle;
WikiPageTagsForm::WikiPageTagsForm(QWidget *parent) : QDialog(parent),  ui(new Ui::WikiPageTagsForm)
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
}

WikiPageTagsForm::~WikiPageTagsForm()
{
    delete this->page;
    delete this->ui;
}

void WikiPageTagsForm::ChangePage(WikiPage *wikipage)
{
    if (wikipage == nullptr)
        throw new Huggle::NullPointerException("WikiPage *wikipage", BOOST_CURRENT_FUNCTION);

    // we copy the page now
    this->page = new WikiPage(wikipage);
    this->setWindowTitle(_l("tag-title", page->PageName));
    // fill it up with tags
    QStringList keys = wikipage->GetSite()->GetProjectConfig()->Tags;
    //! \todo Currently we parse the tags from diff instead of page text
    int rx = 0;
    foreach (QString item, keys)
    {
        QString key = item;
        QString description = _l("page-tag-nodescription");
        if (wikipage->GetSite()->GetProjectConfig()->TagsDesc.contains(key))
            description = wikipage->GetSite()->GetProjectConfig()->TagsDesc[key];
        this->ui->tableWidget->insertRow(rx);
        QCheckBox *Item = new QCheckBox(this);
        if (this->page->Contents.toLower().contains("{{" + key.toLower()))
            Item->setChecked(true);
        if (this->CheckBoxes.contains(key) || this->Arguments.contains(key))
            throw new Huggle::Exception("Tag is already in hash list", BOOST_CURRENT_FUNCTION);
        this->CheckBoxes.insert(key, Item);
        QLineEdit *line = new QLineEdit(this);
        this->Arguments.insert(key, line);
        if (wikipage->GetSite()->GetProjectConfig()->TagsArgs.contains(key) && !wikipage->GetSite()->GetProjectConfig()->TagsArgs[key].isEmpty())
        {
            line->setText(wikipage->GetSite()->GetProjectConfig()->TagsArgs[key]);
        }
        else
        {
            line->setText("No parameters");
            line->setEnabled(false);
        }
        this->ui->tableWidget->setCellWidget(rx, 0, Item);
        this->ui->tableWidget->setItem(rx, 1, new QTableWidgetItem("{{" + key + "}}"));
        this->ui->tableWidget->setCellWidget(rx, 2, line);
        this->ui->tableWidget->setItem(rx, 3, new QTableWidgetItem(description));
        rx++;
    }
    this->ui->tableWidget->resizeColumnsToContents();
    this->ui->tableWidget->resizeRowsToContents();
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
    mb.setText(_l("page-tag-error", result->Result->ErrorMessage));
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
    bool success = false;
    QString t_;
    QString text = Generic::EvaluateWikiPageContents(retrieve, &success, &t_);
    if (success)
    {
        Generic::MessageBox(_l("page-tag-fail"), _l("page-tag-error", text));
        result->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
        return;
    }
    // append all tags to top of page or bottom, depending on preference of user
    int xx = 0;
    WikiPageTagsForm *form = (WikiPageTagsForm *)result->CallbackResult;
    result->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
    while (xx < form->ui->tableWidget->rowCount())
    {
        if (form->page->GetSite()->GetProjectConfig()->Tags.count() > xx && form->Arguments.count() > xx && form->CheckBoxes.count() > xx)
        {
            QString tag = form->page->GetSite()->GetProjectConfig()->Tags.at(xx);
            if (!form->CheckBoxes.contains(tag))
                throw new Huggle::Exception("No such a tag in hash table", BOOST_CURRENT_FUNCTION);
            if (form->CheckBoxes[tag]->isChecked())
            {
                if (!form->Arguments.contains(tag))
                    throw new Huggle::Exception("No such a tag in hash table", BOOST_CURRENT_FUNCTION);
                QString key = "{{" + form->page->GetSite()->GetProjectConfig()->Tags.at(xx) + "}}";
                if (!form->page->GetSite()->GetProjectConfig()->TagsArgs.contains(tag))
                    throw new Huggle::Exception("No such a tag in args hash: " + tag, BOOST_CURRENT_FUNCTION);
                if (!form->page->GetSite()->GetProjectConfig()->TagsArgs[tag].isEmpty())
                {
                    if (form->Arguments[tag]->text() == form->page->GetSite()->GetProjectConfig()->TagsArgs[tag])
                    {
                        Generic::pMessageBox(form, "Error", "This tag requires a parameter, but you didn't provide any for it: " + tag);
                        return;
                    }
                    key = "{{" + tag + "|" + form->Arguments[tag]->text() + "}}";
                }
                if (!text.contains(key))
                {
                    // in case there is a different version of this tag in text, we need to remove it first
                    text = ClearTag(tag, text);
                    if (!form->ui->checkBox->isChecked())
                        text = key + "\n" + text;
                    else
                        text += "\n" + key;
                }
            } else
            {
                text = ClearTag(tag, text);
            }
        }
        xx++;
    }
    Collectable_SmartPtr<EditQuery> e = WikiUtil::EditPage(form->page, text, Configuration::HuggleConfiguration->GenerateSuffix(
                                                           form->page->GetSite()->GetProjectConfig()->TaggingSummary, form->page->GetSite()->GetProjectConfig()), true, t_);
    e->FailureCallback = (Callback)Fail;
    e->callback = (Callback)Finish;
    e->CallbackResult = (void*)form;
    // this is just making sure that this edit will not get deleted by GC, it's removed in callback function
    // it probably isn't needed
    e->IncRef();
    QueryPool::HugglePool->AppendQuery(e);
}

void Huggle::WikiPageTagsForm::on_pushButton_clicked()
{
    this->ui->pushButton->setEnabled(false);
    this->ui->checkBox->setEnabled(false);
    this->ui->tableWidget->setEnabled(false);
    // first get the contents of the page
    ApiQuery *retrieve = Generic::RetrieveWikiPageContents(this->page, false);
    retrieve->FailureCallback = (Callback)Fail;
    retrieve->CallbackResult = (void*)this;
    retrieve->callback = (Callback)Huggle::WikiPageTagsForm_FinishRead;
    QueryPool::HugglePool->AppendQuery(retrieve);
    retrieve->Process();
}
