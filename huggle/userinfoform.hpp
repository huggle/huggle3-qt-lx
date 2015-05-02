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
#include <QList>
#include "apiquery.hpp"
#include "collectable_smartptr.hpp"
#include "edittype.hpp"
#include "mediawikiobject.hpp"
#include "wikiedit.hpp"
class QModelIndex;

namespace Ui
{
    class UserinfoForm;
}

namespace Huggle
{
    class WikiUser;
    class WikiEdit;
    class ApiQuery;

    class HUGGLE_EX UserInfoFormHistoryItem : public MediaWikiObject
    {
        public:
            QString  Page;
            QString  Name;
            QString  Date;
            QString  Summary;
            QString  RevID;
            EditType Type = EditType_Normal;
            bool     Top;
    };

    /*!
     * \brief The UserinfoForm class is a widget that displays the information about user
     * including their history and some other information about the user
     */
    class HUGGLE_EX UserinfoForm : public QDockWidget
    {
            Q_OBJECT
        public:
            explicit UserinfoForm(QWidget *parent = nullptr);
            ~UserinfoForm();
            void ChangeUser(WikiUser *user);
            void Read();
            void Render(long revid, QString page);
            QList<UserInfoFormHistoryItem> Items;

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
