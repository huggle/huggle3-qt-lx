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

class ApiQuery : public Query
{
    Q_OBJECT

private:
    QString ActionPart;
    QNetworkReply *reply;
    void ConstructUrl();
    bool FormatIsCurrentlySupported();
    void FinishRollback();
private slots:
    void ReadData();
    void Finished();
public:
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
    void Process();
    void SetAction(Action action);
    void SetAction(QString action);
    void Kill();
    QString QueryTargetToString();
    QString QueryTypeToString();
    QString Target;
    QString OverrideWiki;
};

#endif // APIQUERY_H
