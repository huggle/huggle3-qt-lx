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

#include "apiquery.hpp"
#include "collectable_smartptr.hpp"
#include "hw.hpp"
#include <QTimer>
#include <QString>

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
    class HUGGLE_EX DeleteForm : public HW
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
            void Delete();
            void Failed(QString Reason);
            Ui::DeleteForm *ui;
            WikiPage *page;
            //! Query used to execute delete of a page
            Collectable_SmartPtr<ApiQuery> qDelete;
            Collectable_SmartPtr<ApiQuery> qTalk;
            //! Set the page to delete
            QTimer *tDelete;
            WikiPage *TalkPage;
            WikiUser *PageUser;
    };
}

#endif // DELETEFORM_H
