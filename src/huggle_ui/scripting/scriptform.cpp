//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "scriptform.hpp"
#include "uiscript.hpp"
#include "../uigeneric.hpp"
#include "ui_scriptform.h"
#include <huggle_core/configuration.hpp>
#include <huggle_core/resources.hpp>
#include <huggle_core/scripting/script.hpp>
#include <QFile>
#include <QFileDialog>

using namespace Huggle;

ScriptForm::ScriptForm(QWidget *parent) : QDialog(parent), ui(new Ui::ScriptForm)
{
    this->ui->setupUi(this);
    this->ui->textEdit->setText(Resources::GetResource("/huggle/text/ecma/template"));
    QString file_name = Configuration::GetExtensionsRootPath() + "new_script.js";
    int file_id = 1;
    while (QFile(file_name).exists())
    {
        file_name = Configuration::GetExtensionsRootPath() + "new_script" + QString::number(file_id++) + ".js";
    }
    this->ui->lineEdit_2->setText(file_name);
}

ScriptForm::~ScriptForm()
{
    delete this->ui;
}

void Huggle::ScriptForm::on_pushButton_2_clicked()
{
    QFileDialog file_dialog(this);
    file_dialog.setNameFilter("Java script (*.js);;All files (*)");
    file_dialog.setWindowTitle("Select script file");
    file_dialog.setFileMode(QFileDialog::FileMode::AnyFile);
    if (file_dialog.exec() == QDialog::DialogCode::Rejected)
        return;
    QString path = file_dialog.selectedFiles().at(0);
    if (!path.isEmpty())
        this->ui->lineEdit_2->setText(path);
}

void Huggle::ScriptForm::on_pushButton_clicked()
{
    QString fullpath = this->ui->lineEdit_2->text();
    QFile f(fullpath);
    if (!f.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        UiGeneric::pMessageBox(this, "Error", "Unable to open " + fullpath + " for writing", MessageBoxStyleError);
        return;
    }
    if (f.write(this->ui->textEdit->toPlainText().toUtf8()) < 0)
    {
        UiGeneric::pMessageBox(this, "Error", "Unable to write - disk full?!", MessageBoxStyleError);
        return;
    }

    f.close();

    QString er;
    UiScript *script = new UiScript();
    if (script->Load(fullpath, &er))
    {
        this->close();
    } else
    {
        UiGeneric::MessageBox("Error", "Failed to load a JS script: " + er);
        delete script;
    }
}
