//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "huggleweb.hpp"
#include <QDesktopServices>
#include "ui_huggleweb.h"

using namespace Huggle;

HuggleWeb::HuggleWeb(QWidget *parent) : GenericBrowser(parent), ui(new Ui::HuggleWeb)
{
    this->ui->setupUi(this);
}

HuggleWeb::~HuggleWeb()
{
    delete this->ui;
}

void HuggleWeb::DisplayPage(const QString &url)
{
    this->ui->webView->load(url);
    this->ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(this->ui->webView, &QWebView::linkClicked, this, &HuggleWeb::Click);
}

void HuggleWeb::RenderHtml(const QString &html)
{
    this->ui->webView->setContent(html.toUtf8());
}

void HuggleWeb::Click(const QUrl &page)
{
    QDesktopServices::openUrl(page);
}

QString HuggleWeb::RetrieveHtml()
{
    return this->ui->webView->page()->mainFrame()->toHtml();
}
