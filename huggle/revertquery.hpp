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

#include <QString>
#include <QtXml>
#include <QTimer>
#include "configuration.hpp"
#include "core.hpp"
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
    class RevertQuery : public Query
    {
            Q_OBJECT
        public:
            static QString GetCustomRevertStatus(QString RevertData);
            RevertQuery();
            RevertQuery(WikiEdit *Edit);
            void Process();
            void Kill();
            ~RevertQuery();
            QString QueryTargetToString();
            bool Processed();
            //! Whether software rollback should be used instead of regular rollback
            bool UsingSR;
            QString Summary;
            //! Rollback with no check if it's a good idea or not (revert even whitelisted users, sysops etc)
            bool IgnorePreflightCheck;
            QString Token;
            bool MinorEdit;
        public slots:
            void OnTick();
        private:
            ApiQuery *qPreflight;
            ApiQuery *qRevert;
            ApiQuery *qRetrieve;
            EditQuery *EditQuerySoftwareRollback;
            WikiEdit* edit;
            QTimer *timer;
            bool RollingBack;
            bool PreflightFinished;
            void Preflight();
            void CheckPreflight();
            bool CheckRevert();
            void Cancel();
            bool ProcessRevert();
            void Rollback();
            void Revert();
            void Exit();
    };
}

#endif // REVERTQUERY_H
