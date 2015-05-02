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

#include <QString>
#include "apiquery.hpp"
#include "collectable_smartptr.hpp"
#include "hw.hpp"

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
    class BlockUser : public HW
    {
            Q_OBJECT
        public:
            explicit BlockUser(QWidget *parent = nullptr);
            ~BlockUser();
            /*!
            * \brief SetWikiUser Select the user/IP to block, display block expiry options from site
            * \param User User to select
            */
            void SetWikiUser(WikiUser *User);
            /*!
            * \brief Failed Show failure message
            * \param reason Reason why blocking failed
            */
            void Failed(QString reason);
            /*!
            * \brief Block Block the selected user and show result
            */
            void Block();
            /*!
            * \brief sendBlockNotice Send the relevant block notice to the user's talk page
            */
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
            int QueryPhase;
    };
}

#endif // BLOCKUSER_H
