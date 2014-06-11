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
// now we need to ensure that python is included first, because it simply suck
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QString>
#include <QDateTime>
#include <QTimer>
#include "editquery.hpp"
#include "wikiedit.hpp"
#include "apiquery.hpp"

namespace Huggle
{
    class ApiQuery;
    class EditQuery;
    class WikiEdit;

    /*!
     * \brief The RevertQuery class can be used to rollback any edit
     */
    class RevertQuery : public QObject, public Query
    {
            Q_OBJECT

        public:
            static QString GetCustomRevertStatus(QString RevertData);

            RevertQuery();
            RevertQuery(WikiEdit *Edit);
            ~RevertQuery();
            void Process();
            //! In case you want to revert only last edit, set this to true
            void SetLast();
            void Kill();
            QString QueryTargetToString();
            bool IsProcessed();
            void SetUsingSR(bool software_rollback);
            bool IsUsingSR();
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
            QString SR_EditToken = "";
            ApiQuery *qPreflight = nullptr;
            ApiQuery *qRevert = nullptr;
            ApiQuery *qHistoryInfo = nullptr;
            ApiQuery *qRetrieve = nullptr;
            ApiQuery *qSR_PageToken = nullptr;
            EditQuery *eqSoftwareRollback = nullptr;
            WikiEdit *edit;
            QTimer *timer = nullptr;
            //! Revert only and only last edit
            bool OneEditOnly = false;
            bool RollingBack = false;
            bool PreflightFinished = false;
            int SR_RevID;
            int SR_Depth;
            QString SR_Target = "";
    };
}

#endif // REVERTQUERY_H
