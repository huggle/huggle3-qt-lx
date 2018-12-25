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
    //! Key/value node for data from API queries
    //! \todo Currently value is provided even for nodes that shouldn't have it
    class HUGGLE_EX_CORE ApiQueryResultNode
    {
          public:
            ApiQueryResultNode();
            ~ApiQueryResultNode();
            /*!
            * \brief GetAttribute Return the specified attribute if it exists, otherwise return the default
            * \param name Name of attribute
            * \param default_val Value to return if the attribute is not found
            * \return Value of attribute or default value
            */
            QString GetAttribute(const QString &name, const QString &default_val = "");
            //! Name of attribute
            QString Name;
            //! Value of attribute
            QString Value;
            //! Hashtable of attribtues
            QHash<QString, QString> Attributes;
            QList<ApiQueryResultNode*> ChildNodes;
    };

    //! Api queries have their own result class so that we can use it to parse them

    //! this is a universal result class that uses same format for all known
    //! formats we are going to use, including XML or JSON, so that it shouldn't
    //! matter which one we use, we always get this structure as output
    class HUGGLE_EX_CORE ApiQueryResult : public QueryResult
    {
        public:
            ApiQueryResult();
            //! Frees the results from memory
            ~ApiQueryResult() override;
            /*!
            * \brief Process Process the data into Nodes and handle any warnings / errors
            */
            void Process();
            /*!
            * \brief GetNode Get the first node with the specified name
            * IMPORTANT: do not delete this node, it's a pointer to item in a list which get deleted in destructor of class
            * \param node_name Name of node
            * \return The specified node or a null pointer if none found
            */
            ApiQueryResultNode *GetNode(const QString& node_name);
            /*!
            * \brief GetNodes Get all nodes with the specified name
            * IMPORTANT: do not delete these nodes, they point to items in a list which get deleted in destructor of class
            * \param node_name Name of node
            * \return QList of pointers to found nodes
            */
            QList<ApiQueryResultNode*> GetNodes(const QString& node_name);
            QString GetNodeValue(const QString &node_name, const QString &default_value = "");
            /*!
            * \brief HasWarnings Return if the API has returned any warnings
            * \return True if there are warnings, false otherwise
            */
            bool HasWarnings();
            //! List of result nodes unsorted with no hierarchy
            QList<ApiQueryResultNode*> Nodes;
            ApiQueryResultNode *Root;
            //! Warning from API query
            QString Warning;
            //! If any error was encountered during the query
            bool HasErrors = false;
    };
}

#endif // APIQUERYRESULT_HPP
