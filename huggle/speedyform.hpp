//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef SPEEDYFORM_H
#define SPEEDYFORM_H

#include "definitions.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
// seriously, Python.h is shitty enough that it requires to be
// included first. Don't believe it? See this:
// http://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QTimer>
#include <QDialog>
#include "exception.hpp"
#include "wikiedit.hpp"
#include "apiquery.hpp"
#include "generic.hpp"
#include "editquery.hpp"
#include "wikiutil.hpp"
#include "wikiuser.hpp"
#include "message.hpp"
#include "configuration.hpp"

namespace Ui
{
    class SpeedyForm;
}

namespace Huggle
{
    class WikiEdit;
    class WikiUser;
    class ApiQuery;
    class EditQuery;

    /*!
     * \brief The window that is used to report a page for deletion
     */
    class SpeedyForm : public QDialog
    {
            Q_OBJECT
        public:
            explicit SpeedyForm(QWidget *parent = 0);
            ~SpeedyForm();
            void Init(WikiEdit *edit_);
            QString Text;

        private slots:
            void OnTick();
            void on_pushButton_2_clicked();
            void on_pushButton_clicked();

        private:
            void Remove();
            void Fail(QString reason);
            void processTags();
            EditQuery *Template;
            ApiQuery *qObtainText;
            QString base;
            WikiEdit *edit;
            QString warning;
            QTimer *timer;
            Ui::SpeedyForm *ui;
    };
}
#endif // SPEEDYFORM_H


