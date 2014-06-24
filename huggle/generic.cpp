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
#include <QtXml>
#include "apiquery.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "localization.hpp"
#include "wikiedit.hpp"

using namespace Huggle;

// we need to preload this thing so that we don't need to create this string so frequently and toast teh PC
static QString options_ = QUrl::toPercentEncoding("timestamp|user|comment|content");

bool Generic::ReportPreFlightCheck()
{
    if (!Configuration::HuggleConfiguration->AskUserBeforeReport)
        return true;
    QMessageBox::StandardButton q = QMessageBox::question(nullptr, _l("report-tu"), _l("report-warn"), QMessageBox::Yes|QMessageBox::No);
    return (q != QMessageBox::No);
}

ApiQuery *Generic::RetrieveWikiPageContents(QString page, bool parse)
{
    ApiQuery *query = new ApiQuery(ActionQuery);
    query->Target = "Retrieving contents of " + page;
    query->Parameters = "prop=revisions&rvlimit=1&rvprop=" + options_ + "&titles=" + QUrl::toPercentEncoding(page);
    if (parse)
        query->Parameters += "&rvparse";
    return query;
}

QString Generic::EvaluateWikiPageContents(ApiQuery *query, bool *failed, QString *ts, QString *comment, QString *user,
                                          int *revid, int *reason, QString *title)
{
    if (!failed)
    {
        throw new Huggle::Exception("failed was NULL", "QString Generic::EvaluateWikiPageContents(ApiQuery *query, "\
                                    "bool *failed, QDateTime *base, QString *comment, QString *user, int *revid)");
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
    QDomDocument d;
    d.setContent(query->Result->Data);
    QDomNodeList page = d.elementsByTagName("rev");
    QDomNodeList code = d.elementsByTagName("page");
    if (code.count() > 0)
    {
        QDomElement e = code.at(0).toElement();
        if (title && e.attributes().contains("title"))
        {
            *title = e.attribute("title");
        }
        if (e.attributes().contains("missing"))
        {
            if (reason) { *reason = EvaluatePageErrorReason_Missing; }
            *failed = true;
            return "Page is missing";
        }
    }
    if (page.count() == 0)
    {
        if (reason) { *reason = EvaluatePageErrorReason_NoRevs; }
        *failed = true;
        return "No revisions were provided for this page";
    }
    QDomElement e = page.at(0).toElement();
    if (user && e.attributes().contains("user"))
    {
        *user = e.attribute("user");
    }
    if (comment && e.attributes().contains("comment"))
    {
        *comment = e.attribute("comment");
    }
    if (ts && e.attributes().contains("timestamp"))
    {
        *ts = e.attribute("timestamp");
    }
    if (revid && e.attributes().contains("revid"))
    {
        *revid = e.attribute("revid").toInt();
    }
    *failed = false;
    return e.text();
}

void Generic::DeveloperError()
{
    QMessageBox *mb = new QMessageBox();
    mb->setWindowTitle("Function is restricted now");
    mb->setText("You can't perform this action in developer mode, because you aren't logged into the wiki");
    mb->exec();
    delete mb;
}

QString Generic::ShrinkText(QString text, int size, bool html)
{
    if (size < 2)
    {
        throw new Huggle::Exception("Parameter size must be more than 2", "QString Core::ShrinkText(QString text, int size)");
    }
    // let's copy the text into new variable so that we don't break the original
    // who knows how these mutable strings are going to behave in qt :D
    QString text_ = text;
    int length = text_.length();
    if (length > size)
    {
        text_ = text_.mid(0, size - 2);
        text_ = text_ + "..";
    } else while (text_.length() < size)
    {
        text_ += " ";
    }
    if (html)
    {
        text_.replace(" ", "&nbsp;");
    }
    return text_;
}
