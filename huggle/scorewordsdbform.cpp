//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "scorewordsdbform.hpp"
#include "ui_scorewordsdbform.h"
using namespace Huggle;

ScoreWordsDbForm::ScoreWordsDbForm(QWidget *parent) : QDialog(parent), ui(new Ui::ScoreWordsDbForm)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnCount(2);
    QStringList header;
    header << "Score" << "Word";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
#if QT_VERSION >= 0x050000
// Qt5 code
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    ui->tableWidget->setShowGrid(false);
}

ScoreWordsDbForm::~ScoreWordsDbForm()
{
    delete ui;
}
