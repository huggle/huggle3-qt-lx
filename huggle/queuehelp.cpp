//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "queuehelp.hpp"
#include "ui_queuehelp.h"

using namespace Huggle;

QueueHelp::QueueHelp(QWidget *parent) : QDialog(parent), ui(new Ui::QueueHelp)
{
    this->ui->setupUi(this);
}

QueueHelp::~QueueHelp()
{
    delete this->ui;
}
