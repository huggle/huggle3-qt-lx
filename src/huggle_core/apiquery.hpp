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

#include "definitions.hpp"

#include <QString>
#include <QObject>
#include "collectable_smartptr.hpp"
#include "query.hpp"
#include "mediawikiobject.hpp"
class QNetworkReply;

namespace Huggle
{
    class ApiQueryResult;
    class RevertQuery;
    class WikiSite;

    enum Action
    {
        ActionClearHasMsg,
        ActionCompare,
        ActionQuery,
        ActionLogin,
        ActionClientLogin,
        ActionLogout,
        //ActionTokens,
        ActionPurge,
        ActionRollback,
        ActionDelete,
        ActionUndelete,
        ActionBlock,
        ActionPatrol,
        ActionReview, // FlaggedRevs
        ActionParse,
        ActionProtect,
        ActionEdit,
        ActionUnwatch,
        ActionWatch,
        ActionCustom
    };

    //! This class can be used to execute any kind of api query on any MW wiki
    class HUGGLE_EX_CORE ApiQuery : public QObject, public Query, public MediaWikiObject
    {
        //! Format in which the result will be returned
        enum Format
        {
            XML,
            JSON,
            PlainText,
            Default
        };

        enum Token
        {
            TokenLogin,
            TokenCsrf,
            TokenRollback,
            TokenPatrol,
            TokenWatch
        };

            Q_OBJECT
        public:
            //! Creates a new instance of this class and set the defaults
            explicit ApiQuery();
            explicit ApiQuery(Action action, WikiSite *site);
            ~ApiQuery();
            Action GetAction();
            QString GetFailureReason();
            //! Change the action type
            void SetAction(const Action action);
            void SetCustomActionPart(QString action, bool editing = false, bool enforce_login = false, bool is_continuous = false);
            ApiQueryResult *GetApiQueryResult();
            //! Run
            void Process();
            //! Set the raw action type, you should not use this unless you have to
            void SetAction(const QString action);
            QString DebugURL();
            //! Terminate the query
            void Kill();
            //! Get a query target as a string
            QString QueryTargetToString();
            //! Returns a type of query as a string
            QString QueryTypeToString();
            QString GetURL();
            void SetParam(QString name, QString value);
            void SetToken(Token token, QString name = "", QString value = "");
            bool EnforceLogin = true;
            //! Whether the query is going to edit any data in wiki
            bool EditingQuery = false;
            //! Whether the query will submit parameters using POST data
            bool UsingPOST = false;
            //! This is a requested format in which the result should be written in
            Format RequestFormat;
            bool IsContinuous = false;
            //! This is an url of api request, you probably don't want to change it unless
            //! you want to construct whole api request yourself
            QString URL = "";
            //! Parameters for action, for example page title
            QString Parameters = "";
            //! This is optional property which contains a label of target this query is for
            QString Target = "none";
            //! You can change this to url of different wiki than a project
            QString OverrideWiki = "";
        private slots:
            void readData();
            void finished();
        private:
            //! Generate api url
            void constructUrl();
            QString constructParameterLessUrl();
            QString getAssertPartSuffix();
            //! Check if return format is supported by huggle
            bool formatIsCurrentlySupported();
            //! This is only needed when you are using rollback
            void finishRollback();
            QHash<QString, QString> params;
            Action _action = ActionQuery;
            QString actionPart;
            QByteArray temp;
            //! Reply from qnet
            QNetworkReply *reply = nullptr;
    };

    inline bool ApiQuery::formatIsCurrentlySupported()
    {
        // other formats will be supported later
        return (this->RequestFormat == XML);
    }

    inline void ApiQuery::SetAction(const QString action)
    {
        this->actionPart = action;
    }

    inline QString ApiQuery::QueryTargetToString()
    {
        return this->Target;
    }

    inline QString ApiQuery::QueryTypeToString()
    {
        return "ApiQuery (" + this->actionPart + ")";
    }
}

#endif // APIQUERY_H
