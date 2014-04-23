//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "huggleweb.hpp"
#include "ui_huggleweb.h"

using namespace Huggle;

HuggleWeb::HuggleWeb(QWidget *parent) : QFrame(parent), ui(new Ui::HuggleWeb)
{
    this->ui->setupUi(this);
    this->CurrentPage = Localizations::HuggleLocalizations->Localize("browser-none");
}

HuggleWeb::~HuggleWeb()
{
    delete this->ui;
}

QString HuggleWeb::CurrentPageName()
{
    return this->CurrentPage;
}

void HuggleWeb::DisplayPreFormattedPage(WikiPage *page)
{
    if (page == NULL)
    {
        throw new Exception("WikiPage *page must not be NULL", "void HuggleWeb::DisplayPreFormattedPage(WikiPage *page)");
    }
    this->ui->webView->history()->clear();
    this->ui->webView->load(QString(Configuration::GetProjectScriptURL() + "index.php?title=" + page->PageName + "&action=render"));
    this->CurrentPage = page->PageName;
}

void HuggleWeb::DisplayPreFormattedPage(QString url)
{
    this->ui->webView->history()->clear();
    url += "&action=render";
    this->ui->webView->load(url);
    this->CurrentPage = this->ui->webView->title();
    this->ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(this->ui->webView, SIGNAL(linkClicked(QUrl)), this, SLOT(Click(QUrl)));
}

void HuggleWeb::DisplayPage(const QString &url)
{
    this->ui->webView->load(url);
    this->ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(this->ui->webView, SIGNAL(linkClicked(QUrl)), this, SLOT(Click(QUrl)));
}

void HuggleWeb::RenderHtml(const QString &html)
{
    this->ui->webView->setContent(html.toUtf8());
}

QString HuggleWeb::Encode(const QString &string)
{
    QString encoded;
    for(int i=0;i<string.size();++i)
    {
        QChar ch = string.at(i);
        if(ch.unicode() > 255)
        {
            encoded += QString("&#%1;").arg((int)ch.unicode());
        }
        else
        {
            encoded += ch;
        }
    }
    encoded = encoded.replace("<", "&lt;");
    encoded = encoded.replace(">", "&gt;");
    return encoded;
}

void HuggleWeb::Click(const QUrl &page)
{
    QDesktopServices::openUrl(page);
}

void HuggleWeb::DisplayDiff(WikiEdit *edit)
{
    this->ui->webView->history()->clear();
    if (edit == NULL)
        throw new Exception("The page of edit was NULL in HuggleWeb::DisplayDiff(*edit)");
    if (edit->Page == NULL)
        throw new Exception("The page of edit was NULL in HuggleWeb::DisplayDiff(*edit)");
    if (edit->NewPage)
    {
        this->ui->webView->setHtml(Localizations::HuggleLocalizations->Localize("browser-load"));
        this->DisplayPreFormattedPage(edit->Page);
        return;
    }
    if (edit->DiffText == "")
    {
        Huggle::Syslog::HuggleLogs->WarningLog("unable to retrieve diff for edit " + edit->Page->PageName + " fallback to web rendering");
        this->ui->webView->setHtml(Localizations::HuggleLocalizations->Localize("browser-load"));
        this->ui->webView->load(QString(Configuration::GetProjectScriptURL() + "index.php?title=" + edit->Page->PageName + "&diff="
                                        + QString::number(edit->Diff) + "&action=render"));
        return;
    }
    QString HTML = Resources::HtmlHeader;
    if (Configuration::HuggleConfiguration->NewMessage)
    {
        // we display a notification that user received a new message
        HTML += Resources::HtmlIncoming;
    }
    HTML += Resources::DiffHeader + "<tr></td colspan=2>";
    if (Configuration::HuggleConfiguration->UserConfig_DisplayTitle)
    {
        HTML += "<p><font size=20px>" + Encode(edit->Page->PageName) + "</font></p>";
    }

    QString Summary;
    QString size;

    if (edit->Size > 0)
    {
        size = "+" + QString::number(edit->Size);
    } else
    {
         size = QString::number(edit->Size);
    }

    if (edit->Summary == "")
    {
        Summary = "<font color=red> " + Localizations::HuggleLocalizations->Localize("browser-miss-summ") + "</font>";
    } else
    {
        Summary = Encode(edit->Summary);
    }

    Summary += "<b> Size change: " + size + "</b>";
    HTML += "<b>" + Localizations::HuggleLocalizations->Localize("summary") + ":</b> " + Summary +
            "</td></tr>" + edit->DiffText + Resources::DiffFooter + Resources::HtmlFooter;

    this->ui->webView->setHtml(HTML);
}

QString HuggleWeb::RetrieveHtml()
{
    return this->ui->webView->page()->mainFrame()->toHtml();
}
