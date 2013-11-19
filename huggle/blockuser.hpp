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
#include "core.hpp"
#include "apiquery.hpp"
#include "configuration.hpp"
#include "wikiuser.hpp"

#if !PRODUCTION_BUILD
namespace Ui
{
    class BlockUser;
}

namespace Huggle
{
    class WikiUser;

    /// \todo This form has to send a message to user who was blocked, until that is fixed it must not be included in production build

    //! This form can be used to block users from editing, which requires the block permission
    class BlockUser : public QDialog
    {
            Q_OBJECT

        public:
            explicit BlockUser(QWidget *parent = 0);
            ~BlockUser();
            void SetWikiUser(WikiUser *User);
        private slots:
            void on_pushButton_clicked();
            void on_pushButton_2_clicked();
            void onTick();
        private:
            Ui::BlockUser *ui;
            /// \todo DOCUMENT ME
            QTimer *t0;
            WikiUser *user;
            /// \todo DOCUMENT ME
            ApiQuery *tb;
            /// \todo DOCUMENT ME
            ApiQuery *b;
            /// \todo What is this and why it even exist - WTF describe me or remove me
            ApiQuery *Dependency;
            QString BlockToken;
            int QueryPhase;
            void CheckToken();
            void GetToken();
            void Failed(QString reason);
            void Block();
            void sendBlockNotice(ApiQuery *dependency);
    };
}

#endif // BLOCKUSER_H
#endif
