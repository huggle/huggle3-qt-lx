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
    class BlockUser;

    //! Report user
    class ReportUser : public QDialog
    {
            Q_OBJECT

        public:
            explicit ReportUser(QWidget *parent = 0);
            //! Set a user
            bool SetUser(WikiUser *u);
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

        private:
            bool CheckUser();
            void InsertUser();
            Ui::ReportUser *ui;
            //! Reported user
            WikiUser *user;
            //! This query is used to retrieve a history of user
            ApiQuery *qHistory;
            //! Timer is used to retrieve a history for user
            QTimer *timer;
            //! Timer to check the report page
            QTimer *t2;
            //! Used to retrieve a diff of page
            QTimer *diff;
            QList <QCheckBox*> CheckBoxes;
            //! Text of report to send to AIV page
            QString ReportText;
            //! Content of report
            QString _p;
            bool Loading;
            bool Messaging;
            BlockUser *BlockForm;
            ApiQuery *tq;
            ApiQuery *qd;
    };
}

#endif // REPORTUSER_H
