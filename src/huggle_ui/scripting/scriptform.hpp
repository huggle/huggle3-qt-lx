//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef SCRIPTFORM_HPP
#define SCRIPTFORM_HPP

#include <huggle_core/definitions.hpp>
#include <QDialog>

namespace Ui
{
    class ScriptForm;
}

namespace Huggle
{
    class JSHighlighter;

    class ScriptForm : public QDialog
    {
            Q_OBJECT

        public:
            explicit ScriptForm(QWidget *parent = nullptr);
            ~ScriptForm();
            void EditScript(const QString& path, const QString& script_name);

        private slots:
            void on_pushButton_2_clicked();
            void on_pushButton_clicked();

        private:
            QString editingName;
            bool editing = false;
            JSHighlighter *highlighter = nullptr;
            Ui::ScriptForm *ui;
    };
}

#endif // SCRIPTFORM_HPP
