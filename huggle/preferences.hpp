//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "definitions.hpp"

#include <QDialog>
#include <QList>
#include "iextension.hpp"
#include "hugglequeuefilter.hpp"

namespace Ui
{
    class Preferences;
}

namespace Huggle
{
    class HuggleQueueFilter;
    //! Preferences window
    class Preferences : public QDialog
    {
            Q_OBJECT
        public:
            explicit Preferences(QWidget *parent = 0);
            ~Preferences();
            void EnableQueues();
            void Disable();
        private slots:
            void on_pushButton_clicked();
            void on_pushButton_2_clicked();
            void on_listWidget_itemSelectionChanged();
            void on_checkBox_clicked();
            void on_pushButton_6_clicked();
            void on_pushButton_5_clicked();
            void on_pushButton_4_clicked();
            void on_pushButton_3_clicked();
            void on_checkBox_26_clicked();
            void on_checkBox_27_clicked();
            void RecordKeys(int row, int column);
            void on_pushButton_7_clicked();

        private:
            void Reload();
            //! Used to reload shortcuts only
            void Reload2();
            bool IgnoreConflicts = false;
            bool RewritingForm = false;
            bool ModifiedForm = false;
            Ui::Preferences *ui;
    };
}

#endif // PREFERENCES_H
