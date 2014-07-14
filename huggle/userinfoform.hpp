//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef USERINFOFORM_H
#define USERINFOFORM_H

#include "definitions.hpp"

#include <QDockWidget>
#include <QString>
#include "apiquery.hpp"
#include "collectable_smartptr.hpp"
#include "wikiedit.hpp"

namespace Ui
{
    class UserinfoForm;
}

namespace Huggle
{
    class WikiUser;
    class WikiEdit;
    class ApiQuery;

    /*!
     * \brief The UserinfoForm class is a widget that displays the information about user
     * including their history and some other information about the user
     */
    class UserinfoForm : public QDockWidget
    {
            Q_OBJECT
        public:
            explicit UserinfoForm(QWidget *parent = nullptr);
            ~UserinfoForm();
            void ChangeUser(WikiUser *user);
            void Read();

        private slots:
            void OnTick();
            void on_pushButton_clicked();
            void on_tableWidget_clicked(const QModelIndex &index);

        private:
            Ui::UserinfoForm *ui;
            WikiUser *User;
            Collectable_SmartPtr<WikiEdit> edit;
            Collectable_SmartPtr<ApiQuery> qContributions;
            QTimer *timer;

    };
}

#endif // USERINFOFORM_H
