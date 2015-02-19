//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "scorewordsdbform.hpp"
#include "configuration.hpp"
#include "localization.hpp"
#include "ui_scorewordsdbform.h"

using namespace Huggle;

ScoreWordsDbForm::ScoreWordsDbForm(QWidget *parent) : QDialog(parent), ui(new Ui::ScoreWordsDbForm)
{
    this->ui->setupUi(this);
    this->ui->tableWidget->setColumnCount(3);
    QStringList header;
    header << _l("score-score") << _l("score-word") << _l("score-range");
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
#if QT_VERSION >= 0x050000
// Qt5 code
    this->ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    this->ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    this->ui->tableWidget->setShowGrid(false);
    int x = 0;
    foreach (ScoreWord word, hcfg->ProjectConfig->ScoreWords)
    {
        this->ui->tableWidget->insertRow(x);
        this->ui->tableWidget->setItem(x, 0, new QTableWidgetItem(QString::number(word.score)));
        this->ui->tableWidget->setItem(x, 1, new QTableWidgetItem(word.word));
        this->ui->tableWidget->setItem(x++, 2, new QTableWidgetItem(_l("score-only-whole-word")));
    }
    foreach (ScoreWord word, hcfg->ProjectConfig->ScoreParts)
    {
        this->ui->tableWidget->insertRow(x);
        this->ui->tableWidget->setItem(x, 0, new QTableWidgetItem(QString::number(word.score)));
        this->ui->tableWidget->setItem(x, 1, new QTableWidgetItem(word.word));
        this->ui->tableWidget->setItem(x, 2, new QTableWidgetItem(_l("score-take-any-word")));
        x++;
    }
    foreach (ScoreWord word, hcfg->ProjectConfig->NoTalkScoreParts)
    {
        this->ui->tableWidget->insertRow(x);
        this->ui->tableWidget->setItem(x, 0, new QTableWidgetItem(QString::number(word.score)));
        this->ui->tableWidget->setItem(x, 1, new QTableWidgetItem(word.word));
        this->ui->tableWidget->setItem(x, 2, new QTableWidgetItem(_l("score-take-any-no-talk-word")));
        x++;
    }
    foreach (ScoreWord word, hcfg->ProjectConfig->NoTalkScoreWords)
    {
        this->ui->tableWidget->insertRow(x);
        this->ui->tableWidget->setItem(x, 0, new QTableWidgetItem(QString::number(word.score)));
        this->ui->tableWidget->setItem(x, 1, new QTableWidgetItem(word.word));
        this->ui->tableWidget->setItem(x, 2, new QTableWidgetItem(_l("score-take-whole-no-talk-word")));
        x++;
    }
    this->ui->tableWidget->resizeRowsToContents();
}

ScoreWordsDbForm::~ScoreWordsDbForm()
{
    delete this->ui;
}
