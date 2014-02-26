//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef SCOREWORDSDBFORM_H
#define SCOREWORDSDBFORM_H

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
#include "configuration.hpp"

namespace Ui
{
    class ScoreWordsDbForm;
}

namespace Huggle
{
    //! Scorewords form

    //! This form should display a list of score words
    class ScoreWordsDbForm : public QDialog
    {
            Q_OBJECT

        public:
            explicit ScoreWordsDbForm(QWidget *parent = 0);
            ~ScoreWordsDbForm();

        private:
            Ui::ScoreWordsDbForm *ui;
    };
}

#endif // SCOREWORDSDBFORM_H
