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
#include <QCheckBox>
#include <QString>
#include <QList>
#include "core.h"
#include "apiquery.h"
#include "configuration.h"
#include "wikiuser.h"

namespace Ui {
class ReportUser;
}

//! Report user
class ReportUser : public QDialog
{
    Q_OBJECT

public:
    explicit ReportUser(QWidget *parent = 0);
    bool SetUser(WikiUser *u);
    ApiQuery *q;
    ~ReportUser();
    //! Content of report
    QString _p;
private slots:
    void Tick();
    void Test();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_tableWidget_clicked(const QModelIndex &index);
    void on_pushButton_3_clicked();

private:
    Ui::ReportUser *ui;
    WikiUser *user;
    QTimer *timer;
    QTimer *t2;
    QList <QCheckBox*> CheckBoxes;
    QString report;
    bool Loading;
    bool Messaging;
    bool CheckUser();
    void InsertUser();
    ApiQuery *tq;
};

#endif // REPORTUSER_H
