//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "scriptingmanager.hpp"
#include "ui_scriptingmanager.h"
#include "../uigeneric.hpp"
#include "uiscript.hpp"
#include <QFile>
#include <QFileDialog>
#include <huggle_core/apiquery.hpp>
#include <huggle_core/generic.hpp>
#include <huggle_core/webserverquery.hpp>
#include <huggle_core/scripting/script.hpp>

using namespace Huggle;

ScriptingManager::ScriptingManager(QWidget *parent) : QDialog(parent), ui(new Ui::ScriptingManager)
{
    this->ui->setupUi(this);
    QStringList headers;
    headers << "Name" << "Author" << "Version" << "Is working" << "Description" << "Path";
    this->ui->tableWidget->setColumnCount(headers.count());
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setHorizontalHeaderLabels(headers);
    this->ui->tableWidget->setShowGrid(false);
    this->ui->tableWidget->resizeRowsToContents();
    this->Reload();
}

ScriptingManager::~ScriptingManager()
{
    delete this->ui;
}

void ScriptingManager::Reload()
{
    while(this->ui->tableWidget->rowCount())
    {
        this->ui->tableWidget->removeRow(0);
    }

    foreach (Script *sx, Script::GetScripts())
    {
        int row = this->ui->tableWidget->rowCount();
        this->ui->tableWidget->insertRow(row);
        this->ui->tableWidget->setItem(row, 0, new QTableWidgetItem(sx->GetName()));
        this->ui->tableWidget->setItem(row, 1, new QTableWidgetItem(sx->GetAuthor()));
        this->ui->tableWidget->setItem(row, 2, new QTableWidgetItem(sx->GetVersion()));
        this->ui->tableWidget->setItem(row, 3, new QTableWidgetItem(Generic::Bool2String(sx->IsWorking())));
        this->ui->tableWidget->setItem(row, 4, new QTableWidgetItem(sx->GetDescription()));
        this->ui->tableWidget->setItem(row, 5, new QTableWidgetItem(sx->GetPath()));
    }
    this->ui->tableWidget->resizeColumnsToContents();
    this->ui->tableWidget->resizeRowsToContents();
}

void ScriptingManager::LoadFile(QString path)
{
    UiScript *script = new UiScript();
    QString er;
    if (!script->Load(path, &er))
    {
        UiGeneric::MessageBox("Failed to load", "Unable to load script " + path + ": " + er, MessageBoxStyleError);
        delete script;
        return;
    }
}

void ScriptingManager::on_bLoad_clicked()
{
    QFileDialog file_dialog(this);
    file_dialog.setNameFilter("Java script (*.js);;All files (*)");
    file_dialog.setWindowTitle("Open script files");
    file_dialog.setFileMode(QFileDialog::FileMode::ExistingFiles);
    if (file_dialog.exec() == QDialog::DialogCode::Rejected)
        return;
    QStringList files = file_dialog.selectedFiles();
    foreach (QString s, files)
        this->LoadFile(s);
    this->Reload();
}

void ScriptingManager::on_bReload_clicked()
{
    QList<Script*> old_scripts = Script::GetScripts();
    foreach (Script *script, old_scripts)
    {
        QString name = script->GetName();
        QString path = script->GetPath();
        script->Unload();
        delete script;
        QString error;
        UiScript *s = new UiScript();
        if (!s->Load(path, &error))
        {
            UiGeneric::MessageBox("Failed to reload script", "Failed to reload " + name + ": " + error, MessageBoxStyleError);
            delete s;
            continue;
        }
    }
    this->Reload();
}
