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

#include <QDockWidget>
#include <QString>
#include "wikiuser.h"

namespace Ui
{
    class UserinfoForm;
}

namespace Huggle
{
    class WikiUser;

    /*!
     * \brief The UserinfoForm class is a widget that displays the information about user
     * including their history and some other information about the user
     */

    /// \todo History of edits doesn't work yet, it needs to be done - !release blocker!
    class UserinfoForm : public QDockWidget
    {
        Q_OBJECT

    public:
        explicit UserinfoForm(QWidget *parent = 0);
        ~UserinfoForm();
        void ChangeUser(WikiUser *user);

    private:
        Ui::UserinfoForm *ui;
        WikiUser *User;
    };
}

#endif // USERINFOFORM_H
