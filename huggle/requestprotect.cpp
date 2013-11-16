//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "requestprotect.hpp"
#include "ui_requestprotect.h"
#if !PRODUCTION_BUILD
using namespace Huggle;

RequestProtect::RequestProtect(QWidget *parent) : QDialog(parent), ui(new Ui::RequestProtect)
{
    ui->setupUi(this);
}

RequestProtect::~RequestProtect()
{
    delete ui;
}

#endif
