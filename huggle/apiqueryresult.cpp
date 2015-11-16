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
    this->Root = nullptr;
}

ApiQueryResult::~ApiQueryResult()
{
    this->Nodes.clear();
    delete this->Root;
}

static void ProcessChildXMLNodes(ApiQueryResultNode *hiearchy_root, ApiQueryResult *result, QDomNodeList nodes)
{
    int id = 0;
    while (id < nodes.count())
    {
        QDomElement element = nodes.at(id++).toElement();
        if (element.tagName().isEmpty())
            continue;
        if (element.tagName() != "warnings")
        {
            ApiQueryResultNode *node = new ApiQueryResultNode();
            hiearchy_root->ChildNodes.append(node);
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
                ProcessChildXMLNodes(node, result, element.childNodes());

            node->Value = element.text();
            if (node->Name == "error")
            {
                QString code = node->GetAttribute("code");
                result->SetError(HUGGLE_EUNKNOWN, "code: " + code + " details: " + node->Value);
                HUGGLE_DEBUG1("Query failed: " + code + " details: " + node->Value);
                HUGGLE_DEBUG(result->Data, 8);
            }
        }
        if (element.tagName() == "warnings" && !hcfg->SystemConfig_SuppressWarnings)
        {
            // there are some warnings which we need to find now
            int cn = 0;
            while (element.childNodes().count() > cn)
            {
                QDomElement warning = element.childNodes().at(cn++).toElement();
                Syslog::HuggleLogs->WarningLog("API query (" + warning.tagName() + "): " + warning.text());
                result->Warning = warning.text();
            }
            HUGGLE_DEBUG(result->Data, 5);
        }
    }
}

void ApiQueryResult::Process()
{
    if (this->Data.isEmpty())
        throw new Huggle::Exception("There is no data to be processed", BOOST_CURRENT_FUNCTION);
    if (this->IsFailed())
        throw new Huggle::Exception("Not processing a failed result", BOOST_CURRENT_FUNCTION);

    QDomDocument result;
    result.setContent(this->Data);
    this->Root = new ApiQueryResultNode();
    this->Root->Name = "Huggle_ApiQueryResultRoot";
    ProcessChildXMLNodes(this->Root, this, result.childNodes());
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

QList<ApiQueryResultNode*> ApiQueryResult::GetNodes(QString node_name)
{
    QList<ApiQueryResultNode*> result;

    foreach (ApiQueryResultNode *node, this->Nodes)
    {
        if (node->Name == node_name)
            result.append(node);
    }

    return result;
}

bool ApiQueryResult::HasWarnings()
{
    return !this->Warning.isEmpty();
}

ApiQueryResultNode::ApiQueryResultNode()
{
    this->Name = "Huggle_None";
}

ApiQueryResultNode::~ApiQueryResultNode()
{
    HUGGLE_CLEAR_PQ_LIST(this->ChildNodes);
}

QString ApiQueryResultNode::GetAttribute(QString name, QString default_val)
{
    if (!this->Attributes.contains(name))
        return default_val;

    // we get the value user requested from local hash
    return this->Attributes[name];
}
