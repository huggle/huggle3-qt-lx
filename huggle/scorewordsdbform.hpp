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
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QDialog>

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
            explicit ScoreWordsDbForm(QWidget *parent = nullptr);
            ~ScoreWordsDbForm();

        private:
            Ui::ScoreWordsDbForm *ui;
    };
}

#endif // SCOREWORDSDBFORM_H
