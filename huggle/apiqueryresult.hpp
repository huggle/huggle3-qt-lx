//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef APIQUERYRESULT_HPP
#define APIQUERYRESULT_HPP

#include "definitions.hpp"
#include <QString>
#include <QList>
#include <QHash>
#include "queryresult.hpp"

namespace Huggle
{
    //! \todo Currently value is provided even for nodes that shouldn't have it
    class HUGGLE_EX ApiQueryResultNode
    {
          public:
            QString GetAttribute(QString name, QString default_val = "");
            QString Name;
            QString Value;
            QHash<QString, QString> Attributes;
    };

    //! Api queries have their own result class so that we can use it to parse them

    //! this is a universal result class that uses same format for all known
    //! formats we are going to use, including XML or JSON, so that it shouldn't
    //! matter which one we use, we always get this structure as output
    class HUGGLE_EX ApiQueryResult : public QueryResult
    {
        public:
            ApiQueryResult();
            ~ApiQueryResult();
            void Process();
            ApiQueryResultNode *GetNode(QString node_name);
            QList<ApiQueryResultNode*> GetNodes(QString node_name);
            bool HasWarnings();
            QList<ApiQueryResultNode*> Nodes;
            QString Warning;
            bool HasErrors = false;
    };
}

#endif // APIQUERYRESULT_HPP
