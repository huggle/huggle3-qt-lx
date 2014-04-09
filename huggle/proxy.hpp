//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef PROXY_H
#define PROXY_H

#include "definitions.hpp"
// now we need to ensure that python is included first
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QDialog>

namespace Ui
{
    class Proxy;
}

namespace Huggle
{
    //! Proxy
    class Proxy : public QDialog
    {
            Q_OBJECT

        public:
            explicit Proxy(QWidget *parent = 0);
            ~Proxy();

        private:
            Ui::Proxy *ui;
    };
}

#endif // PROXY_H
