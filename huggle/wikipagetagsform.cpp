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
#include "wikipage.hpp"
#include "configuration.hpp"
#include "syslog.hpp"
#include "generic.hpp"
#include "querypool.hpp"
#include "localization.hpp"
#include "wikiedit.hpp"
#include "wikiutil.hpp"
#include "ui_wikipagetagsform.h"

using namespace Huggle;
WikiPageTagsForm::WikiPageTagsForm(QWidget *parent) : QDialog(parent),  ui(new Ui::WikiPageTagsForm)
{
    ui->setupUi(this);
    QStringList header;
    header << "" << _l("tag-tags") << _l("description");
    this->ui->tableWidget->setColumnCount(3);
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
#if QT_VERSION >= 0x050000
// Qt5 code
    this->ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    this->ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    this->ui->tableWidget->setShowGrid(false);
}

WikiPageTagsForm::~WikiPageTagsForm()
{
    delete this->page;
    delete this->ui;
}

void WikiPageTagsForm::ChangePage(WikiPage *wikipage)
{
    this->page = new WikiPage(wikipage);
    this->setWindowTitle(_l("tag-title", page->PageName));
    // fill it up with tags
    QStringList keys = Configuration::HuggleConfiguration->ProjectConfig->Tags;
    int rx = 0;
    foreach (QString item, keys)
    {
        QString key = item;
        QString description = "there is no description for this tag :/";
        if (key.contains(";"))
        {
            description = key.mid(key.indexOf(";") + 1);
            key = key.mid(0, key.indexOf(";"));
        }
        this->ui->tableWidget->insertRow(rx);
        QCheckBox *Item = new QCheckBox(this);
        if (this->page->Contents.contains(key))
            Item->setChecked(true);
        this->CheckBoxes.append(Item);
        this->ui->tableWidget->setCellWidget(rx, 0, Item);
        this->ui->tableWidget->setItem(rx, 1, new QTableWidgetItem("{{" + key + "}}"));
        this->ui->tableWidget->setItem(rx, 2, new QTableWidgetItem(description));
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
    mb.setWindowTitle("Failed to tag page");
    mb.setText("Unable to tag the page, error was: " + result->Result->ErrorMessage);
    mb.exec();
    result->DecRef();
    result->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
}

static void FinishRead(Query *result)
{
    ApiQuery *retrieve = (ApiQuery*)result;
    bool success = false;
    QString t_;
    QString text = Generic::EvaluateWikiPageContents(retrieve, &success, &t_);
    if (success)
    {
        QMessageBox mb;
        mb.setWindowTitle("Failed to tag page");
        mb.setText("Unable to tag the page, error was: " + text);
        mb.exec();
        result->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
        return;
    }
    // append all tags to top of page
    int xx = 0;
    WikiPageTagsForm *form = (WikiPageTagsForm *)result->CallbackResult;
    while (xx < form->ui->tableWidget->rowCount())
    {
        if (Configuration::HuggleConfiguration->ProjectConfig->Tags.count() > xx && form->CheckBoxes.count() > xx )
        {
            QString key = "{{" + Configuration::HuggleConfiguration->ProjectConfig->Tags.at(xx) + "}}";
            if (form->CheckBoxes.at(xx)->isChecked())
            {
                if (!text.contains(key))
                    text = key + "\n" + text;
            } else
            {
                if (text.contains(key + "\n"))
                    text.replace(key + "\n", "");
            }
        }
        xx++;
    }
    Collectable_SmartPtr<EditQuery> e = WikiUtil::EditPage(form->page, text, Configuration::HuggleConfiguration->GenerateSuffix
                                        (Configuration::HuggleConfiguration->ProjectConfig->TaggingSummary),
                                      true, t_);
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
    // first get the contents of the page
    ApiQuery *retrieve = Generic::RetrieveWikiPageContents(this->page->PageName, false);
    retrieve->FailureCallback = (Callback)Fail;
    retrieve->CallbackResult = (void*)this;
    retrieve->callback = (Callback)FinishRead;
    QueryPool::HugglePool->AppendQuery(retrieve);
    retrieve->Process();
}
