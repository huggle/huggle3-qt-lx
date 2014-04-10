//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLETOOL_H
#define HUGGLETOOL_H

#include "definitions.hpp"
// now we need to ensure that python is included first because it simply suck :P
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QString>
#include <QTimer>
#include <QDockWidget>
#include <QFont>
#include "syslog.hpp"
#include "apiquery.hpp"
#include "wikipage.hpp"
#include "wikiedit.hpp"
#include "exception.hpp"
#include "configuration.hpp"
#include "querypool.hpp"

namespace Ui
{
    class HuggleTool;
}

namespace Huggle
{
    class ApiQuery;
    class WikiEdit;
    class WikiPage;

    //! Toolbar on top of window
    class HuggleTool : public QDockWidget
    {
            Q_OBJECT

        public:
            explicit HuggleTool(QWidget *parent = 0);
            ~HuggleTool();
            void SetTitle(QString title);
            void SetInfo(QString info);
            void SetUser(QString user);
            void SetPage(WikiPage* page);
            void RenderEdit();

        private slots:
            void on_pushButton_clicked();
            void onTick();
            void on_lineEdit_3_returnPressed();

        private:
            void FinishPage();
            void FinishEdit();
            void DeleteQuery();
            Ui::HuggleTool *ui;
            ApiQuery *query;
            //! Timer that is used to switch between events that happen when the data for page are retrieved
            QTimer *tick;
            //! Pointer used to create an instance of page before passing it to processing function
            WikiEdit *edit;
            //! Page download phase

            //! When we download a page from wiki we need to do that in several steps, this variable holds
            //! the information which step we are in
            int QueryPhase;
    };
}

#endif // HUGGLETOOL_H
