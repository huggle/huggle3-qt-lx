//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef CUSTOMMESSAGE_H
#define CUSTOMMESSAGE_H

#include <QString>
#include "hw.hpp"

namespace Ui
{
    class CustomMessage;
}

namespace Huggle
{
    class WikiUser;

    //! This form can be used to send a custom message to users
    class CustomMessage : public HW
    {
            Q_OBJECT
        public:
            explicit CustomMessage(WikiUser *User, QWidget *parent = nullptr);
            ~CustomMessage();

        private slots:
            void on_pushButton_clicked();
            void on_pushButton_2_clicked();
            bool VerifyMessage();
            void on_lineEdit_textChanged();
            void on_plainTextEdit_textChanged();

        private:
            /*!
            * \brief SetWikiUser Select the user/IP
            * \param User User to select
            */
            void SetWikiUser(WikiUser *User);
            Ui::CustomMessage *ui;
            WikiUser *user;
    };
}

#endif // CUSTOMMESSAGE_H
