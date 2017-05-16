//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "huggleweb.hpp"
#include "hugglewebpage.hpp"
#include "ui_huggleweb.h"

using namespace Huggle;

HuggleWeb::HuggleWeb(QWidget *parent) : GenericBrowser(parent), ui(new Ui::HuggleWeb)
{
    this->ui->setupUi(this);
    this->ui->webView->setPage(new HuggleWebEnginePage());
}

HuggleWeb::~HuggleWeb()
{
    delete this->ui;
}

void HuggleWeb::DisplayPage(const QString &url)
{
    this->ui->webView->history()->clear();
    this->source.clear();
    this->ui->webView->load(url);
}

void HuggleWeb::RenderHtml(const QString &html)
{
    this->ui->webView->history()->clear();
    this->source = html;
    this->ui->webView->setHtml(html);
}

QString HuggleWeb::RetrieveHtml()
{
    if (!this->source.isEmpty())
        return this->source;
    return "Retrieving of source code is not supported yet in Chromium library";
}
