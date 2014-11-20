//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef DELETEFORM_H
#define DELETEFORM_H

#include "definitions.hpp"

#include <QDialog>
#include <QTimer>
#include <QString>
#include "apiquery.hpp"
#include "collectable_smartptr.hpp"

namespace Ui
{
    class DeleteForm;
}

namespace Huggle
{
    class WikiPage;
    class ApiQuery;
    class WikiUser;

    //! This is a delete form
    class HUGGLE_EX DeleteForm : public QDialog
    {
            Q_OBJECT

        public:
            explicit DeleteForm(QWidget *parent = nullptr);
            ~DeleteForm();
            void SetPage(WikiPage *Page, WikiUser *User);
        private slots:
            void on_pushButton_clicked();
            void on_pushButton_2_clicked();
            void OnTick();
        private:
            void GetToken();
            void Delete();
            void CheckDeleteToken();
            void Failed(QString Reason);
            Ui::DeleteForm *ui;
            WikiPage *page;
            QString DeleteToken;
            QString DeleteToken2;
            //! Query used to execute delete of a page
            Collectable_SmartPtr<ApiQuery> qDelete;
            Collectable_SmartPtr<ApiQuery> qTalk;
            //! This is used to retrieve a token
            Collectable_SmartPtr<ApiQuery> qToken;
            Collectable_SmartPtr<ApiQuery> qTokenOfTalkPage;
            //! Set the page to delete
            QTimer *tDelete;
            WikiPage *TalkPage;
            WikiUser *PageUser;
            //! This is used to figure out what are we doing now in timer signal
            int delQueryPhase;
    };
}

#endif // DELETEFORM_H
