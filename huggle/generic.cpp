//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "generic.hpp"

using namespace Huggle;

ApiQuery *Generic::RetrieveWikiPageContents(QString page)
{
    ApiQuery *query = new ApiQuery(ActionQuery);
    query->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") +
                        "&titles=" + QUrl::toPercentEncoding(page);
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
