//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef SESSIONFORM_H
#define SESSIONFORM_H

#include "definitions.hpp"

#include "hw.hpp"

namespace Ui
{
    class SessionForm;
}

namespace Huggle
{
    //! Session info

    //! Display which user, project, what rights and flags your session have
    class HUGGLE_EX SessionForm : public HW
    {
            Q_OBJECT
        public:
            explicit SessionForm(QWidget *parent = nullptr);
            ~SessionForm();

        private slots:
            void on_pushButton_clicked();
            void on_comboBox_currentIndexChanged(int index);

        private:
            void Reload(int x);
            Ui::SessionForm *ui;
    };
}

#endif // SESSIONFORM_H
