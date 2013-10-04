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

#include <QDialog>

namespace Ui {
class Preferences;
}

namespace Huggle
{
    //! Preferences window
    class Preferences : public QDialog
    {
        Q_OBJECT

    public:
        explicit Preferences(QWidget *parent = 0);
        ~Preferences();

    private slots:
        void on_pushButton_clicked();

    private:
        Ui::Preferences *ui;
    };
}

#endif // PREFERENCES_H
