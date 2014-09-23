//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef EDITBAR_HPP
#define EDITBAR_HPP

#include "definitions.hpp"
#include <QDockWidget>
#include <QList>
#include <QTimer>
#include "edittype.hpp"

namespace Ui
{
    class EditBar;
}

namespace Huggle
{
    class WikiEdit;
    class WikiPage;
    class WikiPageHistoryItem;
    class UserInfoFormHistoryItem;
    class WikiUser;
    class EditBarItem;

    class EditBar : public QDockWidget
    {
            Q_OBJECT
        public:
            explicit EditBar(QWidget *parent = 0);
            ~EditBar();
            void RemoveAll();
            void Refresh();
            void RefreshPage();
            void RefreshUser();
        private slots:
            void on_pushButton_2_clicked();
            void on_pushButton_clicked();
            void on_pushButton_4_clicked();
            void on_pushButton_3_clicked();
            void OnReload();
        private:
            void InsertEdit(WikiPageHistoryItem *page, int RowId);
            void InsertUser(UserInfoFormHistoryItem *user);
            void MovePage(int size);
            void ClearUser();
            void ClearPage();
            void MoveUser(int size);
            int PageSX;
            int UserSX;
            QTimer timer;
            QList<EditBarItem*> Items;
            Ui::EditBar *ui;
    };
}

#endif // EDITBAR_HPP
