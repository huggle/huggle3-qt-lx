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

    /*!
     * \brief The EditBar class can be used to list different edit information of pages like 
     * the time and username of the last edit or do the same from the users perspective 
     * (list the pages edited by a user)
     */
    class HUGGLE_EX EditBar : public QDockWidget
    {
            Q_OBJECT
        public: 
	    /*!
	     * \brief EditBar Creates a new instance of the EditBar class
	     * \param parent The parent widget in which the EditBar is contained
	     */
            explicit EditBar(QWidget *parent = 0);
            ~EditBar();
	    /*!
	     * \brief RemoveAll Removes all pages and users from the EditBar
	     */
            void RemoveAll();
	    /*!
	     * \brief Refresh Reloads all pages and users from the EditBar
	     */
            void Refresh();
	    /*!
	     * \brief RefreshPage Reloads all pages from the EditBar
	     */
            void RefreshPage();
	    /*!
	     * \brief RefreshUser Reloads all users from the EditBar
	     */
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
