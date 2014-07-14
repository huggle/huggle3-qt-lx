//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WAITINGFORM_H
#define WAITINGFORM_H

#include "definitions.hpp"

#include <QDialog>

namespace Ui
{
    class WaitingForm;
}

namespace Huggle
{
    //! This is universal form that is just displaying the progress bar and reason why we need to wait
    class WaitingForm : public QDialog
    {
            Q_OBJECT

        public:
            explicit WaitingForm(QWidget *parent = nullptr);
            ~WaitingForm();
            void Status(int progress);
            void Status(int progress, QString text);

        private:
            Ui::WaitingForm *ui;
            void reject();
    };
}

#endif // WAITINGFORM_H
