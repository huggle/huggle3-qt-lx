//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editquery.hpp"

using namespace Huggle;

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
    if (qToken != NULL)
    {
        qToken->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
    }
}

void EditQuery::Process()
{
    this->Status = StatusProcessing;
    this->StartTime = QDateTime::currentDateTime();
    qToken = new ApiQuery();
    qToken->SetAction(ActionQuery);
    qToken->Parameters = "prop=info&intoken=edit&titles=" + QUrl::toPercentEncoding(page);
    qToken->Target = Localizations::HuggleLocalizations->Localize("editquery-token", page);
    qToken->RegisterConsumer(HUGGLECONSUMER_EDITQUERY);
    Core::HuggleCore->AppendQuery(qToken);
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
            this->Result->ErrorMessage = Localizations::HuggleLocalizations->Localize("editquery-token-error") + ": " + qToken->Result->ErrorMessage;
            this->qToken->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
            this->qToken = NULL;
            return true;
        }
        QDomDocument d;
        d.setContent(qToken->Result->Data);
        QDomNodeList l = d.elementsByTagName("page");
        if (l.count() == 0)
        {
            this->Result = new QueryResult();
            this->Result->Failed = true;
            this->Result->ErrorMessage = Localizations::HuggleLocalizations->Localize("editquery-token-error");
            Core::HuggleCore->DebugLog("Debug message for edit: " + qToken->Result->Data);
            this->qToken->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
            this->qToken = NULL;
            return true;
        }
        QDomElement element = l.at(0).toElement();
        if (!element.attributes().contains("edittoken"))
        {
            this->Result = new QueryResult();
            this->Result->Failed = true;
            this->Result->ErrorMessage = Localizations::HuggleLocalizations->Localize("editquery-token-error");
            Core::HuggleCore->DebugLog("Debug message for edit: " + qToken->Result->Data);
            this->qToken->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
            this->qToken = NULL;
            return true;
        }
        _Token = element.attribute("edittoken");
        qToken->Lock();
        qToken->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
        qToken = NULL;
        qEdit = new ApiQuery();
        qEdit->Target = "Writing " + page;
        qEdit->UsingPOST = true;
        qEdit->RegisterConsumer(HUGGLECONSUMER_EDITQUERY);
        qEdit->SetAction(ActionEdit);
        qEdit->Parameters = "title=" + QUrl::toPercentEncoding(page) + "&text=" + QUrl::toPercentEncoding(text) +
                "&summary=" + QUrl::toPercentEncoding(this->summary) + "&token=" + QUrl::toPercentEncoding(_Token);
        Core::HuggleCore->AppendQuery(qEdit);
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
                    if (Core::HuggleCore->Main != NULL)
                    {
                        HistoryItem item;
                        item.Result = Localizations::HuggleLocalizations->Localize("successful");
                        item.Type = HistoryEdit;
                        item.Target = this->page;
                        Core::HuggleCore->Main->_History->Prepend(item);
                    }
                    Core::HuggleCore->Log(Localizations::HuggleLocalizations->Localize("editquery-success", page));
                }
            }
        }
        Result = new QueryResult();
        qEdit->UnregisterConsumer(HUGGLECONSUMER_EDITQUERY);
        qEdit = NULL;
    }
    return true;
}
