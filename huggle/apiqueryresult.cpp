//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "apiqueryresult.hpp"
#include "configuration.hpp"
#include "exception.hpp"
#include "syslog.hpp"
#include <QtXml>

using namespace Huggle;

ApiQueryResult::ApiQueryResult()
{

}

ApiQueryResult::~ApiQueryResult()
{
    while (this->Nodes.count())
    {
        delete this->Nodes.at(0);
        this->Nodes.removeAt(0);
    }
}

static void ProcessChildXMLNodes(ApiQueryResult *result, QDomNodeList nodes)
{
    int id = 0;
    while (id < nodes.count())
    {
        QDomElement element = nodes.at(id).toElement();
        id++;
        if (element.tagName().isEmpty())
            continue;
        ApiQueryResultNode *node = new ApiQueryResultNode();
        node->Name = element.tagName();
        int attr = 0;
        while (attr < element.attributes().count())
        {
            QDomAttr ca = element.attributes().item(attr).toAttr();
            attr++;
            if (!node->Attributes.contains(ca.name()))
                node->Attributes.insert(ca.name(), ca.value());
            else
                Syslog::HuggleLogs->WarningLog("Invalid xml node (present multiple times) " + ca.name() + " in " + element.tagName());
        }
        result->Nodes.append(node);
        if (element.childNodes().count())
            ProcessChildXMLNodes(result, element.childNodes());

        node->Value = element.text();
        if (node->Name == "error")
        {
            QString code = node->GetAttribute("code");
            result->SetError(HUGGLE_EUNKNOWN, "code: " + code + " details: " + node->Value);
            HUGGLE_DEBUG1("Query failed: " + code + " details: " + node->Value);
            HUGGLE_DEBUG(result->Data, 8);
        }
        if (node->Name == "warnings" && !hcfg->SystemConfig_SuppressWarnings)
        {
            // there are some warnings which we need to find now
            int cn = 0;
            while (element.childNodes().count() > cn)
            {
                QDomElement warning = element.childNodes().at(cn).toElement();
                Syslog::HuggleLogs->WarningLog("API query (" + warning.tagName() + "): " + warning.text());
                // let's go next
                cn++;
            }
            HUGGLE_DEBUG(result->Data, 5);
        }
    }
}

void ApiQueryResult::Process()
{
    if (this->Data.isEmpty())
        throw new Huggle::Exception("There is no data to be processed", "void ApiQueryResult::Process()");
    if (this->IsFailed())
        throw new Huggle::Exception("Not processing a failed result", "void ApiQueryResult::Process()");

    QDomDocument result;
    result.setContent(this->Data);
    ProcessChildXMLNodes(this, result.childNodes());
}

ApiQueryResultNode *ApiQueryResult::GetNode(QString node_name)
{
    foreach (ApiQueryResultNode *node, this->Nodes)
    {
        if (node->Name == node_name)
            return node;
    }
    return nullptr;
}

QList<ApiQueryResultNode *> ApiQueryResult::GetNodes(QString node_name)
{
    QList<ApiQueryResultNode  *> result;

    foreach (ApiQueryResultNode *node, this->Nodes)
    {
        if (node->Name == node_name)
            result.append(node);
    }

    return result;
}

QString ApiQueryResultNode::GetAttribute(QString name, QString default_val)
{
    if (!this->Attributes.contains(name))
        return default_val;

    // we get the value user requested from local hash
    return this->Attributes[name];
}
