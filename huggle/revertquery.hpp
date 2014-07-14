//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef REVERTQUERY_H
#define REVERTQUERY_H

#include "definitions.hpp"

#include <QString>
#include <QDateTime>
#include <QTimer>
#include "editquery.hpp"
#include "wikiedit.hpp"
#include "apiquery.hpp"
#include "collectable_smartptr.hpp"
#include "mediawikiobject.hpp"

namespace Huggle
{
    class ApiQuery;
    class EditQuery;
    class WikiEdit;
    class WikiSite;

    /*!
     * \brief The RevertQuery class can be used to rollback any edit
     */
    class RevertQuery : public QObject, public Query, public MediaWikiObject
    {
            Q_OBJECT
        public:
            static QString GetCustomRevertStatus(QString RevertData);

            RevertQuery();
            RevertQuery(WikiEdit *Edit);
            RevertQuery(WikiEdit *Edit, WikiSite *site);
            ~RevertQuery();
            void Process();
            //! In case you want to revert only last edit, set this to true
            void SetLast();
            void Kill();
            QString QueryTargetToString();
            bool IsProcessed();
            void SetUsingSR(bool software_rollback);
            bool IsUsingSR();
            WikiSite *GetSite();
            //! Time when a query was issued (this is set externaly)
            QDateTime Date;
            QString Summary = "";
            //! Rollback with no check if it's a good idea or not (revert even whitelisted users, sysops etc)
            bool IgnorePreflightCheck = false;
            QString Token = "";
            bool MinorEdit = false;
        public slots:
            void OnTick();
        private:
            void DisplayError(QString error, QString reason = "");
            void Preflight();
            void CheckPreflight();
            bool CheckRevert();
            void Cancel();
            bool ProcessRevert();
            void Rollback();
            void Revert();
            void Exit();
            //! Whether software rollback should be used instead of regular rollback
            bool UsingSR = false;
            Collectable_SmartPtr<ApiQuery> qPreflight;
            Collectable_SmartPtr<ApiQuery> qRevert;
            Collectable_SmartPtr<ApiQuery> qHistoryInfo;
            Collectable_SmartPtr<ApiQuery> qRetrieve;
            Collectable_SmartPtr<EditQuery> eqSoftwareRollback;
            Collectable_SmartPtr<WikiEdit> edit;
            QTimer *timer = nullptr;
            //! Revert only and only last edit
            bool OneEditOnly = false;
            bool RollingBack = false;
            bool PreflightFinished = false;
            int SR_RevID;
            int SR_Depth;
            QString SR_Target = "";
    };

    inline WikiSite *RevertQuery::GetSite()
    {
        if (this->Site == nullptr)
            return (this->edit->GetSite());

        // we know the site and despite it may be inconsistent we return it because that is what
        // programmer wanted (by inconsistent I mean the query could have different site
        // than the edit now had) :o
        return this->Site;
    }
}

#endif // REVERTQUERY_H
