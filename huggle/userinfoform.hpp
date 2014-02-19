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

#include "config.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
// seriously, Python.h is shitty enough that it requires to be
// included first. Don't believe it? See this:
// http://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QDockWidget>
#include <QString>
#include "core.hpp"
#include "exception.hpp"
#include "wikiedit.hpp"
#include "apiquery.hpp"
#include "wikiuser.hpp"

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
            explicit UserinfoForm(QWidget *parent = 0);
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
            ApiQuery *qContributions;
            QTimer *timer;
            WikiEdit *edit;
    };
}

#endif // USERINFOFORM_H
