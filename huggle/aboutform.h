//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef ABOUTFORM_H
#define ABOUTFORM_H

#include <QDialog>
#include "configuration.h"

namespace Ui {
class AboutForm;
}

//! Form that can be displayed from help menu, should list all developers
class AboutForm : public QDialog
{
    Q_OBJECT
    
public:
    explicit AboutForm(QWidget *parent = 0);
    ~AboutForm();
    
private slots:
    void on_pushButton_clicked();

private:
    Ui::AboutForm *ui;
};

#endif // ABOUTFORM_H
