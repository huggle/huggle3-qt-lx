//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "loadingform.hpp"
#include "login.hpp"
#include "exception.hpp"
#include "core.hpp"
#include "ui_loadingform.h"

using namespace Huggle;

LoadingForm::LoadingForm(QWidget *parent) : QDialog(parent), ui(new Ui::LoadingForm)
{
    this->ui->setupUi(this);
    this->ui->tableWidget->setColumnCount(2);
    this->ui->tableWidget->horizontalHeader()->setVisible(false);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui->tableWidget->setShowGrid(false);
#if QT_VERSION >= 0x050000
// Qt5 code
        this->ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
        this->ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    this->ui->tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

void LoadingForm::Info(QString text)
{
    this->ui->label_2->setText(text);
}

void LoadingForm::ModifyIcon(int row, LoadingForm_Icon it)
{
    // fix me
    if (this == nullptr)
        return;
    if (this->ui->tableWidget->rowCount() < row + 1)
    {
        throw new Huggle::Exception("There is no such an item in list");
    }
    QIcon icon;
    switch (it)
    {
        case LoadingForm_Icon_Failed:
            icon = QIcon(":/huggle/pictures/Resources/user-block.png");
            break;
        case LoadingForm_Icon_Success:
            icon = QIcon(":/huggle/pictures/Resources/done.png");
            break;
        case LoadingForm_Icon_Loading:
            icon = QIcon(":/huggle/pictures/Resources/browser-next.png");
            break;
        case LoadingForm_Icon_Waiting:
            icon = QIcon(":/huggle/pictures/Resources/sc.png");
            break;
    }
    this->ui->tableWidget->setItem(row, 1, new QTableWidgetItem(icon, ""));
    this->ui->tableWidget->resizeRowToContents(row);
}

void LoadingForm::Insert(int row, QString text, LoadingForm_Icon icon)
{
    if (this->ui->tableWidget->rowCount() < row + 1)
    {
        this->ui->tableWidget->insertRow(row);
    }
    QIcon x_;
    switch (icon)
    {
        case LoadingForm_Icon_Failed:
            x_ = QIcon(":/huggle/pictures/Resources/user-block.png");
            break;
        case LoadingForm_Icon_Success:
            x_ = QIcon(":/huggle/pictures/Resources/done.png");
            break;
        case LoadingForm_Icon_Loading:
            x_ = QIcon(":/huggle/pictures/Resources/browser-next.png");
            break;
        case LoadingForm_Icon_Waiting:
            x_ = QIcon(":/huggle/pictures/Resources/sc.png");
            break;
    }
    this->ui->tableWidget->setItem(row, 0, new QTableWidgetItem(text));
    this->ui->tableWidget->setItem(row, 1, new QTableWidgetItem(x_, ""));
    this->ui->tableWidget->resizeRowToContents(row);
}

LoadingForm::~LoadingForm()
{
    delete ui;
}

void Huggle::LoadingForm::on_pushButton_clicked()
{
    this->ui->pushButton->setEnabled(false);
    Core::HuggleCore->fLogin->CancelLogin();
}

void LoadingForm::reject()
{
    // this function replaces the original that hides the window so that
    // it's not possible to reject this window
}
