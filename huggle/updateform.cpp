//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "updateform.hpp"
#include "ui_updateform.h"
using namespace Huggle;

UpdateForm::UpdateForm(QWidget *parent) : QDialog(parent), ui(new Ui::UpdateForm)
{
    this->ui->setupUi(this);
    this->qData = NULL;
    this->t = new QTimer(this);
}

UpdateForm::~UpdateForm()
{
    delete this->ui;
    delete this->t;
}

void UpdateForm::Check()
{
    this->qData = new WebserverQuery();
    this->qData->URL = "http://tools.wmflabs.org/huggle/updater/?version=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->HuggleVersion)
            + "&os=" + QUrl::toPercentEncoding(Configuration::HuggleConfiguration->Platform);
    this->qData->Process();
    connect(this->t, SIGNAL(timeout()), this, SLOT(OnTick()));
    this->t->start(60);
}

void Huggle::UpdateForm::on_pushButton_2_clicked()
{
    this->close();
}

void UpdateForm::OnTick()
{
    if (!this->qData->Processed())
    {
        return;
    }

    this->t->stop();
}
