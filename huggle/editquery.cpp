//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editquery.h"

EditQuery::EditQuery()
{
    this->summary = "";
    this->Result = NULL;
    this->qEdit = NULL;
    this->Minor = false;
    this->page = "";
    this->qToken = NULL;
    this->text = "";
    this->Type = QueryEdit;
}

EditQuery::~EditQuery()
{

}

void EditQuery::Process()
{
    qToken = new ApiQuery();
    qToken->SetAction(ActionQuery);
    qToken->Parameters = "prop=info&intoken=edit&titles=" + page;
    qToken->Target = "Retrieving token to edit " + page;
    qToken->DeleteLater = true;
    Core::RunningQueries.append(qToken);
    qToken->Process();
}

bool EditQuery::Processed()
{
    if (this->Result != NULL)
    {
        return true;
    }
    if (qToken != NULL)
    {
        if (!qToken->Processed())
        {
            return false;
        }
        if (qToken->Result->Failed)
        {
            this->Result = new QueryResult();
            this->Result->Failed = true;
            this->Result->ErrorMessage = "Unable to retrieve edit token, error was: " + qToken->Result->ErrorMessage;
            this->qToken->DeleteLater = false;
            return true;
        }
        QDomDocument d;
        d.setContent(qToken->Result->Data);
        QDomNodeList l = d.elementsByTagName("page");
        if (l.count() == 0)
        {
            this->Result = new QueryResult();
            this->Result->Failed = true;
            this->Result->ErrorMessage = "Unable to retrieve edit token";
            this->qToken->DeleteLater = false;
            return true;
        }
        QDomElement element = l.at(0).toElement();
        if (!element.attributes().contains("edittoken"))
        {
            this->Result = new QueryResult();
            this->Result->Failed = true;
            this->Result->ErrorMessage = "Unable to retrieve edit token";
            this->qToken->DeleteLater = false;
            return true;
        }
        _Token = element.attribute("edittoken");
        qToken->SafeDelete();
        qToken->DeleteLater = false;
        qToken = NULL;
        qEdit = new ApiQuery();
        qEdit->Target = "Writing " + page;
        qEdit->UsingPOST = true;
        qEdit->DeleteLater = true;
        qEdit->SetAction(ActionEdit);
        qEdit->Parameters = "title=" + QUrl::toPercentEncoding(page) + "&text=" + QUrl::toPercentEncoding(text) +
                "&summary=" + QUrl::toPercentEncoding(this->summary) + "&token=" + QUrl::toPercentEncoding(_Token);
        Core::RunningQueries.append(qEdit);
        qEdit->Process();
        return false;
    }
    if (qEdit != NULL)
    {
        if (!qEdit->Processed())
        {
            return false;
        }
        QDomDocument d;
        d.setContent(qEdit->Result->Data);
        QDomNodeList l = d.elementsByTagName("edit");
        if (l.count() > 0)
        {
            QDomElement element = l.at(0).toElement();
            if (element.attributes().contains("result"))
            {
                if (element.attribute("result") == "Success")
                {
                    Core::Log("Successfuly edit " + page);
                }
            }
        }
        Result = new QueryResult();
        qEdit->DeleteLater = false;
        qEdit = NULL;
    }
    return true;
}
