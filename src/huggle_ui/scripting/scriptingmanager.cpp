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
#include "scriptform.hpp"
#include "ui_scriptingmanager.h"
#include "../uigeneric.hpp"
#include "uiscript.hpp"
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
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

void ScriptingManager::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    QPoint global = this->ui->tableWidget->mapToGlobal(pos);
    QAction *unload = new QAction("Unload", &menu);
    QAction *reload = new QAction("Reload", &menu);
    QAction *delete_file = new QAction("Delete from disk", &menu);
    menu.addAction(unload);
    menu.addAction(reload);
    menu.addAction(delete_file);
    QAction *selection = menu.exec(global);
    if (selection == unload)
    {
        this->unloadSelectSc();
    } else if (selection == delete_file)
    {
        this->deleteSelectSc();
    } else if (selection == reload)
    {
        this->reloadSelectSc();
    }
}

void ScriptingManager::on_pushScript_clicked()
{
    ScriptForm sf;
    sf.exec();
    this->Reload();
}

void ScriptingManager::unloadSelectSc()
{
    QList<int> selected = selectedRows();
    foreach (int i, selected)
    {
        QString script_name = this->ui->tableWidget->item(i, 0)->text();
        Script *script = Script::GetScriptByName(script_name);
        if (!script)
        {
            UiGeneric::pMessageBox(this, "Error", "Unable to unload " + script_name + " script not found in memory", MessageBoxStyleError);
            continue;
        }
        script->Unload();
        delete script;
    }
    this->Reload();
}

void ScriptingManager::deleteSelectSc()
{
    if (UiGeneric::pMessageBox(this, "Delete files", "Are you sure you want to permanently delete selected files?", MessageBoxStyleQuestion)
            == QMessageBox::No)
        return;

    QList<int> selected = selectedRows();
    foreach (int i, selected)
    {
        QString script_name = this->ui->tableWidget->item(i, 0)->text();
        Script *script = Script::GetScriptByName(script_name);
        if (!script)
        {
            UiGeneric::pMessageBox(this, "Error", "Unable to unload " + script_name + " script not found in memory", MessageBoxStyleError);
            continue;
        }
        QString path = script->GetPath();
        script->Unload();
        delete script;
        QFile file(path);
        if (!file.remove())
            UiGeneric::pMessageBox(this, "Error", "Unable to remove " + path, MessageBoxStyleError);
    }
    this->Reload();
}

void ScriptingManager::reloadSelectSc()
{
    QList<int> selected = selectedRows();
    foreach (int i, selected)
    {
        QString script_name = this->ui->tableWidget->item(i, 0)->text();
        QString error;
        Script *script = Script::GetScriptByName(script_name);
        if (!script)
        {
            UiGeneric::pMessageBox(this, "Error", "Unable to reload " + script_name + " script not found in memory", MessageBoxStyleError);
            continue;
        }
        QString file = script->GetPath();
        script->Unload();
        delete script;
        UiScript *s = new UiScript();
        if (!s->Load(file, &error))
        {
            UiGeneric::MessageBox("Failed to reload script", "Failed to reload " + script_name + ": " + error, MessageBoxStyleError);
            delete s;
            continue;
        }
    }
    this->Reload();
}

QList<int> ScriptingManager::selectedRows()
{
    QList<int> selection;
    foreach (QTableWidgetItem *i, this->ui->tableWidget->selectedItems())
    {
        if (!selection.contains(i->row()))
            selection.append(i->row());
    }
    return selection;
}
