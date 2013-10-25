//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.


#ifndef BLOCKUSER_H
#define BLOCKUSER_H

#include <QDialog>
#include <QCheckBox>
#include <QString>
#include <QtXml>
#include <QTimer>
#include "core.h"
#include "apiquery.h"
#include "configuration.h"
#include "wikiuser.h"

namespace Ui {
class BlockUser;
}

namespace Huggle
{
    class WikiUser;

    class BlockUser : public QDialog
    {
        Q_OBJECT

    public:
        explicit BlockUser(QWidget *parent = 0);
        ~BlockUser();
        void SetWikiUser(WikiUser* User);
    private slots:
        void on_pushButton_clicked();
        void on_pushButton_2_clicked();
        void onTick();
    private:
        Ui::BlockUser *ui;
        QTimer *t0;
        WikiUser *user;
        ApiQuery *tb;
        ApiQuery *b;
        ApiQuery *dependency;
        QString blocktoken;
        int QueryPhase;
        void CheckToken();
        void GetToken();
        void Failed(QString reason);
        void Block();
        void sendBlockNotice();
    };
}

#endif // BLOCKUSER_H
