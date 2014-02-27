//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.


#ifndef REPORTUSER_H
#define REPORTUSER_H

#include "definitions.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
// seriously, Python.h is shitty enough that it requires to be
// included first. Don't believe it? See this:
// http://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QDialog>
#include <QTimer>
#include <QWebView>
#include <QMutex>
#include <QtXml>
#include <QCheckBox>
#include <QString>
#include <QList>
#include "core.hpp"
#include "blockuser.hpp"
#include "editquery.hpp"
#include "resources.hpp"
#include "apiquery.hpp"
#include "huggleweb.hpp"
#include "configuration.hpp"
#include "wikiuser.hpp"

namespace Ui
{
    class ReportUser;
}

namespace Huggle
{
    class WikiUser;
    class ApiQuery;
    class EditQuery;
    class BlockUser;

    //! Report user
    class ReportUser : public QDialog
    {
            Q_OBJECT

        public:
            explicit ReportUser(QWidget *parent = 0);
            //! Set a user
            bool SetUser(WikiUser *user);
            ~ReportUser();

        private slots:
            void Tick();
            void On_DiffTick();
            void Test();
            void on_pushButton_clicked();
            void on_pushButton_2_clicked();
            void on_tableWidget_clicked(const QModelIndex &index);
            void on_pushButton_3_clicked();
            void on_pushButton_4_clicked();
            void on_pushButton_6_clicked();
            void on_pushButton_5_clicked();

        private:
            bool CheckUser();
            void InsertUser();
            //! Stop all operations
            void Kill();
            Ui::ReportUser *ui;
            //! Reported user
            WikiUser *ReportedUser;
            //! This query is used to retrieve a history of user
            ApiQuery *qHistory;
            //! Timer is used to report the user
            QTimer *tReportUser;
            //! Timer to check the report page
            QTimer *tReportPageCheck;
            //! Used to retrieve a diff of page
            QTimer *tPageDiff;
            EditQuery *qEdit;
            QList <QCheckBox*> CheckBoxes;
            //! Text of report to send to AIV page
            QString ReportText;
            //! Content of report
            QString ReportContent;
            QString ReportTs;
            bool Loading;
            bool Messaging;
            BlockUser *BlockForm;
            //! This query is used to get a block history
            ApiQuery *qBlockHistory;
            //! This is used to retrieve current report page and write to it
            ApiQuery *qReport;
            ApiQuery *qDiff;
    };
}

#endif // REPORTUSER_H
