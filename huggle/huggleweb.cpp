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
#include "exception.hpp"
#include "localization.hpp"
#include "syslog.hpp"
#include "configuration.hpp"
#include "wikipage.hpp"
#include "wikiedit.hpp"
#include "resources.hpp"
#include "ui_huggleweb.h"

using namespace Huggle;

HuggleWeb::HuggleWeb(QWidget *parent) : QFrame(parent), ui(new Ui::HuggleWeb)
{
    this->ui->setupUi(this);
    this->CurrentPage = _l("browser-none");
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
    if (page == nullptr)
    {
        throw new Huggle::Exception("WikiPage *page must not be NULL", "void HuggleWeb::DisplayPreFormattedPage(WikiPage *page)");
    }
    this->ui->webView->history()->clear();
    this->ui->webView->load(QString(Configuration::GetProjectScriptURL() + "index.php?title=" + page->PageName + "&action=render"));
    this->CurrentPage = page->PageName;
    this->ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(this->ui->webView, SIGNAL(linkClicked(QUrl)), this, SLOT(Click(QUrl)));
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
    if (edit == nullptr)
        throw new Huggle::Exception("The page of edit was NULL in HuggleWeb::DisplayDiff(*edit)");
    if (edit->Page == nullptr)
        throw new Huggle::Exception("The page of edit was NULL in HuggleWeb::DisplayDiff(*edit)");
    if (edit->NewPage && !edit->Page->Contents.size())
    {
        this->ui->webView->setHtml(_l("browser-load"));
        this->DisplayPreFormattedPage(edit->Page);
        return;
    } else if (edit->NewPage)
    {
        this->DisplayNewPageEdit(edit);
        return;
    }
    if (!edit->DiffText.length())
    {
        Huggle::Syslog::HuggleLogs->WarningLog("unable to retrieve diff for edit " + edit->Page->PageName + " fallback to web rendering");
        this->ui->webView->setHtml(_l("browser-load"));
        this->ui->webView->load(QString(Configuration::GetProjectScriptURL() + "index.php?title=" + edit->Page->PageName + "&diff="
                                        + QString::number(edit->Diff) + "&action=render"));
        return;
    }
    QString HTML = Resources::GetHtmlHeader();
    if (Configuration::HuggleConfiguration->NewMessage)
    {
        // we display a notification that user received a new message
        HTML += Resources::HtmlIncoming;
    }
    HTML += Resources::DiffHeader + "<tr></td colspan=2>";
    if (Configuration::HuggleConfiguration->UserConfig->DisplayTitle)
    {
        HTML += "<p><font size=20px>" + Encode(edit->Page->PageName) + "</font></p>";
    }
    QString Summary;
    QString size;
    if (edit->Size > 0)
        size = "<font color=green>+" + QString::number(edit->Size) + "</font>";
    else
        size = "<font color=\"red\">" + QString::number(edit->Size) + "</font>";


    if (edit->Summary.isEmpty())
    {
        Summary = "<font color=red> " + _l("browser-miss-summ") + "</font>";
    } else
    {
        Summary = Encode(edit->Summary);
    }
    Summary += "<b> Size change: " + size + "</b>";
    HTML += "<b>" + _l("summary") + ":</b> " + Summary +
            "</td></tr>" + edit->DiffText + Resources::DiffFooter + Resources::HtmlFooter;
    this->ui->webView->setHtml(HTML);
}

void HuggleWeb::DisplayNewPageEdit(WikiEdit *edit)
{
    if (!edit)
        throw new Exception("Edit must not be NULL");

    QString HTML = Resources::GetHtmlHeader();
    if (Configuration::HuggleConfiguration->NewMessage)
    {
        // we display a notification that user received a new message
        HTML += Resources::HtmlIncoming;
    }
    if (Configuration::HuggleConfiguration->UserConfig->DisplayTitle)
    {
        HTML += "<p><font size=20px>" + Encode(edit->Page->PageName) + "</font></p>";
    }
    QString Summary;
    if (!edit->Summary.size())
    {
        Summary = "<font color=red> " + _l("browser-miss-summ") + "</font>";
    } else
    {
        Summary = Encode(edit->Summary);
    }
    HTML += "<b>" + _l("summary") + ":</b> " + Summary + "<br>" +
            edit->Page->Contents + Resources::HtmlFooter;
    this->ui->webView->setHtml(HTML);
}

QString HuggleWeb::RetrieveHtml()
{
    return this->ui->webView->page()->mainFrame()->toHtml();
}
