//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editform.hpp"
#include "uigeneric.hpp"
#include "ui_editform.h"
#ifdef HUGGLE_WEBEN
    #include "web_engine/huggleweb.hpp"
#else
    #include "webkit/huggleweb.hpp"
#endif
#include <huggle_core/apiqueryresult.hpp>
#include <huggle_core/apiquery.hpp>
#include <huggle_core/configuration.hpp>
#include <huggle_core/editquery.hpp>
#include <huggle_core/resources.hpp>
#include <huggle_core/wikiutil.hpp>
#include <huggle_core/wikisite.hpp>
#include <huggle_core/wikipage.hpp>
#include <huggle_core/querypool.hpp>
#include <huggle_core/localization.hpp>
#include <huggle_core/generic.hpp>

using namespace Huggle;

void ContentFail(Query *query)
{
    Collectable_SmartPtr<ApiQuery> api_query = (ApiQuery*)query;
    api_query->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
    EditForm *f = (EditForm*)query->CallbackOwner;
    f->Failure(query->GetFailureReason());
}

void ContentFinish(Query *query)
{
    Collectable_SmartPtr<ApiQuery> api_query = (ApiQuery*)query;
    api_query->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
    EditForm *f = (EditForm*)query->CallbackOwner;
    bool failed = false;
    QString ts;
    // Get the source text of page
    QString page_text = WikiUtil::EvaluateWikiPageContents((ApiQuery*)query, &failed, &ts);
    if (failed)
    {
        f->Failure(page_text);
        return;
    }
    f->RenderSource(page_text, ts);
}

EditForm::EditForm(WikiPage *wp, QWidget *parent) : HW("editform", this, parent), ui(new Ui::EditForm)
{
    this->ui->setupUi(this);
    // Naughty hack
    QFont font("Monospace, \"Andale Mono\", \"Courier New\";");
    font.setStyleHint(QFont::TypeWriter);
    this->ui->textEdit->setFont(font);
    this->page = new WikiPage(wp);
    this->webView = new HuggleWeb(this);
    this->ui->verticalLayout_2->addWidget(this->webView);
    this->setWindowTitle("Editing " + this->page->PageName + " on " + this->page->GetSite()->Name);
    this->RestoreWindow();
    this->ui->textEdit->setText("Loading source...");
    this->renderText("<h1>Loading source...</h1>");
    this->ui->checkBox->setEnabled(false);
    this->ui->lineEdit->setEnabled(false);
    this->ui->textEdit->setEnabled(false);
    this->ui->pushButton_2->setEnabled(false);
    this->ui->pushButton->setEnabled(false);
    this->contentQuery = WikiUtil::RetrieveWikiPageContents(this->page);
    QueryPool::HugglePool->AppendQuery(this->contentQuery);
    this->contentQuery->CallbackOwner = this;
    this->contentQuery->SuccessCallback = (Callback)  ContentFinish;
    this->contentQuery->FailureCallback = (Callback)  ContentFail;

    this->contentQuery->Process();
}

EditForm::~EditForm()
{
    delete this->webView;
    delete this->ui;
    delete this->page;
}

void EditForm::Failure(QString reason)
{
    UiGeneric::pMessageBox(this, "Error", reason, MessageBoxStyleError, true);
    this->close();
}

void EditForm::RenderSource(QString code, QString time)
{
    this->ui->checkBox->setEnabled(true);
    this->ui->lineEdit->setEnabled(true);
    this->ui->textEdit->setEnabled(true);
    this->ui->pushButton_2->setEnabled(true);
    this->ui->pushButton->setEnabled(true);
    this->ui->textEdit->setText(code);
    this->pageTime = time;
    this->renderText("<h2>Click preview to see amazing stuff here</h2>");
}

void EditForm::DisplayPreview(QString html)
{
    // Apply CSS
    html = Resources::GetHtmlHeader(this->page->GetSite()) + html + Resources::HtmlFooter;
    this->webView->RenderHtml(html);
    this->ui->pushButton->setEnabled(true);
}

void EditForm::FinishEdit()
{
    UiGeneric::pMessageBox(this, "Edited", "Page was successfuly edited");
    this->close();
}

void EditForm::FailEdit(QString reason)
{
    UiGeneric::pMessageBox(this, "Error", "Unable to edit page: " + reason, MessageBoxStyleError);
    this->webView->RenderHtml(":-(");
    this->ui->pushButton->setEnabled(true);
    this->ui->pushButton_2->setEnabled(true);
    this->ui->textEdit->setEnabled(true);
    this->ui->checkBox->setEnabled(true);
    this->ui->lineEdit->setEnabled(true);
}

void EditFail(Query *query)
{
    Collectable_SmartPtr<EditQuery> edit_query = (EditQuery*)  query;
    edit_query->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
    EditForm *f = (EditForm*)query->CallbackOwner;
    f->FailEdit(query->GetFailureReason());
}

void EditSuccess(Query *query)
{
    Collectable_SmartPtr<EditQuery> edit_query = (EditQuery*)  query;
    edit_query->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
    EditForm *f = (EditForm*)query->CallbackOwner;
    f->FinishEdit();
}

void Huggle::EditForm::on_pushButton_2_clicked()
{
    this->ui->pushButton->setEnabled(false);
    this->ui->pushButton_2->setEnabled(false);
    this->ui->textEdit->setEnabled(false);
    this->ui->checkBox->setEnabled(false);
    this->ui->lineEdit->setEnabled(false);
    this->renderText("<h2>Saving...</h2>");
    this->editQuery = WikiUtil::EditPage(this->page, this->ui->textEdit->toPlainText(),
                                         this->ui->lineEdit->text(),
                                         this->ui->checkBox->isChecked(),
                                         this->pageTime);
    this->editQuery->SuccessCallback = (Callback)EditSuccess;
    this->editQuery->FailureCallback = (Callback)EditFail;
    this->editQuery->CallbackOwner = this;
}

void ParseFail(Query *query)
{
    Collectable_SmartPtr<ApiQuery> api_query = (ApiQuery*)query;
    api_query->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
    EditForm *f = (EditForm*)query->CallbackOwner;
    f->DisplayPreview("Error: " + api_query->GetFailureReason());
}

void ParseFinish(Query *query)
{
    Collectable_SmartPtr<ApiQuery> api_query = (ApiQuery*)query;
    api_query->UnregisterConsumer(HUGGLECONSUMER_CALLBACK);
    EditForm *f = (EditForm*)query->CallbackOwner;
    QString text = api_query->GetApiQueryResult()->GetNode("text")->Value;
    f->DisplayPreview(text);
}

void Huggle::EditForm::on_pushButton_clicked()
{
    this->ui->pushButton->setEnabled(false);
    this->renderText("<h2>Loading preview...</h2>");
    this->parseQuery = new ApiQuery(ActionParse, this->page->GetSite());
    this->parseQuery->Target = "Parsing wikitext for " + this->page->PageName;
    this->parseQuery->UsingPOST = true;
    this->parseQuery->SuccessCallback = (Callback) ParseFinish;
    this->parseQuery->FailureCallback = (Callback) ParseFail;
    this->parseQuery->Parameters = "&title=" + QUrl::toPercentEncoding(this->page->PageName) + "&text=" +
                                    QUrl::toPercentEncoding(this->ui->textEdit->toPlainText()) +
                                    "&contentmodel=wikitext";
    this->parseQuery->CallbackOwner = this;
    QueryPool::HugglePool->AppendQuery(this->parseQuery);
    this->parseQuery->Process();
}

void EditForm::renderText(QString text)
{
    this->webView->RenderHtml(Resources::GetHtmlHeader(this->page->GetSite()) + text + Resources::HtmlFooter);
}
