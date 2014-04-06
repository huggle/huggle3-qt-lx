//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WARNINGLIST_HPP
#define WARNINGLIST_HPP

#include "definitions.hpp"
// now we need to ensure that python is included first, because it simply suck don't believe it? See this:
// http://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QDialog>
#include "wikiedit.hpp"
#include "warnings.hpp"
#include "configuration.hpp"
#include "huggleparser.hpp"
#include "localization.hpp"

namespace Ui
{
    class WarningList;
}

namespace Huggle
{
    //! Widget that allows user to pick a warning to send to user
    class WarningList : public QDialog
    {
            Q_OBJECT

        public:
            explicit WarningList(WikiEdit *edit, QWidget *parent = 0);
            ~WarningList();

        private slots:
            void on_pushButton_clicked();

        private:
            WikiEdit *wikiEdit;
            Ui::WarningList *ui;
    };
}

#endif // WARNINGLIST_HPP
