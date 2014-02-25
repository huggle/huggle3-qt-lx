//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "config.hpp"
// now we need to ensure that python is included first, because it
// simply suck :P
// seriously, Python.h is shitty enough that it requires to be
// included first. Don't believe it? See this:
// http://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QDialog>
#include <QList>
#include "configuration.hpp"
#include "iextension.hpp"
#include "hugglequeuefilter.hpp"
#ifdef PYTHONENGINE
#include "pythonengine.hpp"
#endif

namespace Ui
{
    class Preferences;
}

namespace Huggle
{
    class HuggleQueueFilter;
#ifdef PYTHONENGINE
    namespace Python
    {
        class PythonScript;
    }
#endif
    //! Preferences window
    class Preferences : public QDialog
    {
            Q_OBJECT

        public:
            explicit Preferences(QWidget *parent = 0);
            ~Preferences();
            void EnableQueues();
            void Disable();

        private slots:
            void on_pushButton_clicked();
            void on_pushButton_2_clicked();
            void on_listWidget_itemSelectionChanged();
            void on_checkBox_clicked();
            void on_pushButton_6_clicked();
            void on_pushButton_5_clicked();
            void on_pushButton_4_clicked();
            void on_pushButton_3_clicked();

        private:
            void Reload();
            Ui::Preferences *ui;
    };
}

#endif // PREFERENCES_H
