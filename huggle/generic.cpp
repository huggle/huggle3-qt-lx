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
#include "exception.hpp"

using namespace Huggle;

bool Generic::ReportPreFlightCheck()
{
    if (!Configuration::HuggleConfiguration->AskUserBeforeReport)
        return true;
    QMessageBox::StandardButton q = QMessageBox::question(NULL, "Report user"
                  , "This user has already reached warning level 4, so no further templates will be "\
                    "delivered to them. You can report them now, but please, make sure that they already reached the proper "\
                    "number of recent warnings! You can do so by clicking the \"talk page\" button in following form. "\
                    "Keep in mind that this form and this warning is displayed no matter if your revert was successful "\
                    "or not, so you might conflict with other users here (double check if user isn't already reported) "\
                    "Do you want to report this user?"
                  , QMessageBox::Yes|QMessageBox::No);
    return (q != QMessageBox::No);
}

ApiQuery *Generic::RetrieveWikiPageContents(QString page, bool parse)
{
    ApiQuery *query = new ApiQuery(ActionQuery);
    query->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") +
                        "&titles=" + QUrl::toPercentEncoding(page);
    if (parse)
        query->Parameters += "&rvparse";
    return query;
}

QString Generic::EvaluateWikiPageContents(ApiQuery *query, bool *failed, QString *ts, QString *comment, QString *user,
                                          int *revid, int *reason)
{
    if (!failed)
    {
        throw new Huggle::Exception("failed was NULL", "QString Generic::EvaluateWikiPageContents(ApiQuery *query, "\
                                    "bool *failed, QDateTime *base, QString *comment, QString *user, int *revid)");
    }
    if (query == NULL)
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
    if (user != NULL && e.attributes().contains("user"))
    {
        *user = e.attribute("user");
    }
    if (comment != NULL && e.attributes().contains("comment"))
    {
        *comment = e.attribute("comment");
    }
    if (ts != NULL && e.attributes().contains("timestamp"))
    {
        *ts = e.attribute("timestamp");
    }
    if (revid != NULL && e.attributes().contains("revid"))
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
