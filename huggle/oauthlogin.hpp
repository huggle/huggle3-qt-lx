//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef OAUTHLOGIN_H
#define OAUTHLOGIN_H

#include "definitions.hpp"
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QDialog>

namespace Ui
{
    class OAuthLogin;
}

namespace Huggle
{
    //! This form is not being used
    class OAuthLogin : public QDialog
    {
            Q_OBJECT

        public:
            explicit OAuthLogin(QWidget *parent = nullptr);
            ~OAuthLogin();

        private:
            Ui::OAuthLogin *ui;
    };
}

#endif // OAUTHLOGIN_H
