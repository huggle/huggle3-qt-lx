//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef APIQUERY_H
#define APIQUERY_H

#include <QString>
#include <QtNetwork/QtNetwork>
#include <QUrl>
#include <QtXml/QtXml>
#include <QObject>
#include <QThread>
#include "query.h"
#include "configuration.h"
#include "core.h"
#include "exception.h"

namespace Huggle
{
    enum Action
    {
        ActionQuery,
        ActionLogin,
        ActionLogout,
        ActionTokens,
        ActionPurge,
        ActionRollback,
        ActionDelete,
        ActionUndelete,
        ActionEdit
    };

    enum Format
    {
        XML,
        JSON,
        PlainText,
        Default
    };

    //! This class can be used to execute any kind of api query on any wiki
    class ApiQuery : public Query
    {
        Q_OBJECT

    private:
        QString ActionPart;
        //! Reply from qnet
        QNetworkReply *reply;
        //! Generate api url
        void ConstructUrl();
        //! Check if return format is supported by huggle
        bool FormatIsCurrentlySupported();
        //! This is only needed when you are using rollback
        void FinishRollback();
    private slots:
        void ReadData();
        void Finished();
    public:
        //! Creates a new instance of this class and set the defaults
        explicit ApiQuery();
        //! Whether the query will submit parameters using POST data
        bool UsingPOST;
        //! This is a requested format in which the result should be written in
        Format RequestFormat;
        //! This is an url of api request, you probably don't want to change it unless
        //! you want to construct whole api request yourself
        QString URL;
        //! Parameters for action, for example page title
        QString Parameters;
        //! Run
        void Process();
        //! Change the action type
        void SetAction(Action action);
        //! Set the raw action type, you should not use this unless you have to
        void SetAction(QString action);
        //! Terminate the query
        void Kill();
        //! Get a query target as a string
        QString QueryTargetToString();
        //! Returns a type of query as a string
        QString QueryTypeToString();
        //! This is optional property which contains a label of target this query is for
        QString Target;
        //! You can change this to url of different wiki than a project
        QString OverrideWiki;
    };
}

#endif // APIQUERY_H
