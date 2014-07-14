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

#include "definitions.hpp"

#include <QDialog>
#include <QString>
#include "apiquery.hpp"
#include "collectable_smartptr.hpp"

namespace Ui
{
    class BlockUser;
}

class QTimer;

namespace Huggle
{
    class WikiUser;
    class ApiQuery;

    //! This form can be used to block users from editing, which requires the block permission
    class BlockUser : public QDialog
    {
            Q_OBJECT
        public:
            explicit BlockUser(QWidget *parent = nullptr);
            ~BlockUser();
            void SetWikiUser(WikiUser *User);
            void CheckToken();
            void GetToken();
            void Failed(QString reason);
            void Block();
            void sendBlockNotice(ApiQuery *dependency);
        private slots:
            void on_pushButton_clicked();
            void on_pushButton_2_clicked();
            void onTick();
            void on_pushButton_3_clicked();
        private:
            void Recheck();
            Ui::BlockUser *ui;
            //! Timer that switches between steps of block workflow
            QTimer *t0;
            WikiUser *user;
            //! Query to exec api to block user
            Collectable_SmartPtr<ApiQuery> qUser;
            Collectable_SmartPtr<ApiQuery> qTokenApi;
            QString BlockToken;
            int QueryPhase;
    };
}

#endif // BLOCKUSER_H
