//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WIKIPAGETAGSFORM_HPP
#define WIKIPAGETAGSFORM_HPP

#include "definitions.hpp"
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QDialog>
#include <QCheckBox>
#include <QStringList>
#include <QString>

namespace Ui
{
    class WikiPageTagsForm;
}

namespace Huggle
{
    class WikiPage;
    class WikiPageTagsForm : public QDialog
    {
            Q_OBJECT
        public:
            explicit WikiPageTagsForm(QWidget *parent = nullptr);
            ~WikiPageTagsForm();
            void ChangePage(WikiPage *wikipage);
            WikiPage *page = nullptr;
            QList<QCheckBox*> CheckBoxes;
            Ui::WikiPageTagsForm *ui;

        private slots:
            void on_pushButton_clicked();
    };
}

#endif // WIKIPAGETAGSFORM_HPP
