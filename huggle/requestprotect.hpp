//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.


#ifndef REQUESTPROTECT_H
#define REQUESTPROTECT_H

#include "definitions.hpp"
// now we need to ensure that python is included first. Don't believe it? See this:
// http://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QTimer>
#include <QDialog>
#include <QRegExp>
#include "wikipage.hpp"
#include "localization.hpp"
#include "apiquery.hpp"
#include "editquery.hpp"
#include "core.hpp"
#include "configuration.hpp"

namespace Ui
{
    class RequestProtect;
}

namespace Huggle
{
    class EditQuery;
    class ApiQuery;

    //! This can be used to request protection of a page
    class RequestProtect : public QDialog
    {
        Q_OBJECT

        public:
            explicit RequestProtect(WikiPage *wikiPage, QWidget *parent = 0);
            ~RequestProtect();
        private slots:
            void Tick();
            void on_pushButton_clicked();
        private:
            QString ProtectionType();
            void Fail(QString message);
            void RemoveQs();
            QString Timestamp;
            WikiPage *page;
            QTimer *tm;
            Ui::RequestProtect *ui;
            ApiQuery *qRFPPage;
            EditQuery *qEditRFP;
    };
}


#endif // REQUESTPROTECT_H
