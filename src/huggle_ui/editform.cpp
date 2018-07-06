//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editform.hpp"
#include "ui_editform.h"

using namespace Huggle;

EditForm::EditForm(QWidget *parent) : HW("editform", this, parent), ui(new Ui::EditForm)
{
    this->ui->setupUi(this);
    this->RestoreWindow();
}

EditForm::~EditForm()
{
    delete this->ui;
}
