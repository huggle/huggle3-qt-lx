//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WHITELISTFORM_H
#define WHITELISTFORM_H

#include "definitions.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
// seriously, Python.h is shitty enough that it requires to be
// included first. Don't believe it? See this:
// http://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QDialog>
#include <QString>
#include <QTimer>
#include "configuration.hpp"

namespace Ui
{
    class WhitelistForm;
}

namespace Huggle
{
    /// \todo Currently it kind of suck when displaying huge whitelist

    //! This form can be used to display a whitelist of current wiki
    class WhitelistForm : public QDialog
    {
            Q_OBJECT

        public:
            explicit WhitelistForm(QWidget *parent = 0);
            ~WhitelistForm();
        private slots:
            void OnTick();
            void on_pushButton_clicked();

        private:
            QStringList Whitelist;
            QTimer *timer;
            Ui::WhitelistForm *ui;
    };
}

#endif // WHITELISTFORM_H
