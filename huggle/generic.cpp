//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "generic.hpp"
#include <QMessageBox>
#include "apiquery.hpp"
#include "apiqueryresult.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "localization.hpp"
#include "wikiedit.hpp"
#include "wikipage.hpp"

#ifdef MessageBox
    // fix GCC for windows headers port
    #undef MessageBox
#endif

using namespace Huggle;

// we need to preload this thing so that we don't need to create this string so frequently and toast teh PC
static QString options_ = QUrl::toPercentEncoding("timestamp|user|comment|content");

QString Generic::Bool2String(bool b)
{
    if (b)
    {
        return "true";
    }
    return "false";
}

bool Generic::SafeBool(QString value, bool defaultvalue)
{
    if (value.toLower() == "true")
    {
        return true;
    }
    return defaultvalue;
}

bool Generic::ReportPreFlightCheck()
{
    if (!Configuration::HuggleConfiguration->AskUserBeforeReport)
        return true;
    QMessageBox::StandardButton q = QMessageBox::question(nullptr, _l("report-tu"), _l("report-warn"), QMessageBox::Yes|QMessageBox::No);
    return (q != QMessageBox::No);
}

ApiQuery *Generic::RetrieveWikiPageContents(QString page, WikiSite *site, bool parse)
{
    WikiPage pt(page);
    pt.Site = site;
    return RetrieveWikiPageContents(&pt, parse);
}

ApiQuery *Generic::RetrieveWikiPageContents(WikiPage *page, bool parse)
{
    ApiQuery *query = new ApiQuery(ActionQuery, page->Site);
    query->Target = "Retrieving contents of " + page->PageName;
    query->Parameters = "prop=revisions&rvlimit=1&rvprop=" + options_ + "&titles=" + QUrl::toPercentEncoding(page->PageName);
    if (parse)
        query->Parameters += "&rvparse";
    return query;
}

QString Generic::EvaluateWikiPageContents(ApiQuery *query, bool *failed, QString *ts, QString *comment, QString *user,
                                          long *revid, int *reason, QString *title)
{
    if (!failed)
    {
        throw new Huggle::NullPointerException("bool *failed", BOOST_CURRENT_FUNCTION);
    }
    if (query == nullptr)
    {
        if (reason) { *reason = EvaluatePageErrorReason_NULL; }
        *failed = true;
        return "Query was NULL";
    }
    if (!query->IsProcessed())
    {
        if (reason) { *reason = EvaluatePageErrorReason_Running; }
        *failed = true;
        return "Query didn't finish";
    }
    if (query->IsFailed())
    {
        if (reason) { *reason = EvaluatePageErrorReason_Unknown; }
        *failed = true;
        return query->Result->ErrorMessage;
    }
    ApiQueryResultNode *rev = query->GetApiQueryResult()->GetNode("rev");
    ApiQueryResultNode *page = query->GetApiQueryResult()->GetNode("page");
    if (page != nullptr)
    {
        if (title && page->Attributes.contains("title"))
            *title = page->Attributes["title"];

        if (page->Attributes.contains("missing"))
        {
            if (reason) { *reason = EvaluatePageErrorReason_Missing; }
            *failed = true;
            return "Page is missing";
        }
    }
    if (rev == nullptr)
    {
        if (reason) { *reason = EvaluatePageErrorReason_NoRevs; }
        *failed = true;
        return "No revisions were provided for this page";
    }
    if (user && rev->Attributes.contains("user"))
        *user = rev->Attributes["user"];

    if (comment && rev->Attributes.contains("comment"))
        *comment = rev->Attributes["comment"];

    if (ts && rev->Attributes.contains("timestamp"))
        *ts = rev->Attributes["timestamp"];

    if (revid)
    {
        if (rev->Attributes.contains("revid"))
            *revid = rev->Attributes["revid"].toInt();
        else
            *revid = WIKI_UNKNOWN_REVID;
    }
    *failed = false;
    return rev->Value;
}

void Generic::DeveloperError()
{
    Generic::MessageBox("Function is restricted now", "You can't perform this action in"\
                        " developer mode, because you aren't logged into the wiki");
}

QString Generic::ShrinkText(QString text, unsigned int size, bool html, unsigned int minimum)
{
    if (size < minimum)
    {
        throw new Huggle::Exception("Parameter size must be more than 2", BOOST_CURRENT_FUNCTION);
    }
    // let's copy the text into new variable so that we don't break the original
    // who knows how these mutable strings are going to behave in qt :D
    QString text_ = text;
    unsigned int length = (unsigned int)text_.length();
    if (length > size)
    {
        text_ = text_.mid(0, size - minimum);
        unsigned int cd = minimum;
        while (cd > 0)
        {
            text_ = text_ + ".";
            cd--;
        }
    } else while ((unsigned int)text_.length() < size)
    {
        text_ += " ";
    }
    if (html)
    {
        text_.replace(" ", "&nbsp;");
    }
    return text_;
}

int Generic::MessageBox(QString title, QString text, MessageBoxStyle st, bool enforce_stop, QWidget *parent)
{
    QMessageBox *mb = new QMessageBox(parent);
    mb->setWindowTitle(title);
    mb->setText(text);
    int return_value = -1;
    switch (st)
    {
        case MessageBoxStyleQuestion:
            mb->setIcon(QMessageBox::Question);
            mb->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            mb->setDefaultButton(QMessageBox::Yes);
            return_value = mb->exec();
            break;
        case MessageBoxStyleNormal:
            mb->setIcon(QMessageBox::Information);
            goto exec;
        case MessageBoxStyleError:
            mb->setIcon(QMessageBox::Critical);
            goto exec;
        case MessageBoxStyleWarning:
            mb->setIcon(QMessageBox::Warning);
            goto exec;
    }
    delete mb;
    return return_value;
    exec:
        if (enforce_stop)
        {
            return_value = mb->exec();
            delete mb;
        }
        else
        {
            mb->setAttribute(Qt::WA_DeleteOnClose, true);
            mb->show();
        }
        return return_value;
}

int Generic::pMessageBox(QWidget *parent, QString title, QString text, MessageBoxStyle st, bool enforce_stop)
{
    return Generic::MessageBox(title, text, st, enforce_stop, parent);
}
