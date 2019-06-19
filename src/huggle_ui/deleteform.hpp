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

#include <huggle_core/definitions.hpp>

#include <huggle_core/apiquery.hpp>
#include <huggle_core/collectable_smartptr.hpp>
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
    class HUGGLE_EX_UI DeleteForm : public HW
    {
            Q_OBJECT
        public:
            explicit DeleteForm(QWidget *parent = nullptr);
            ~DeleteForm() override;
            /*!
             * \brief SetPage sets a page to delete
             * \param Page Pointer to a page to delete
             * \param User Pointer to user to notify, this object will be deleted by this class in destructor
             */
            void SetPage(WikiPage *Page, WikiUser *User);
        private slots:
            void on_pushButton_clicked();
            void on_pushButton_2_clicked();
            void OnTick();
        private:
            void deletePage();
            void processFailure(QString Reason);
            Ui::DeleteForm *ui;
            WikiPage *page;
            //! Query used to execute delete of a page
            Collectable_SmartPtr<ApiQuery> qDelete;
            Collectable_SmartPtr<ApiQuery> qTalk;
            //! Set the page to delete
            QTimer *tDelete;
            WikiPage *associatedTalkPage;
            WikiUser *userToNotify;
    };
}

#endif // DELETEFORM_H
