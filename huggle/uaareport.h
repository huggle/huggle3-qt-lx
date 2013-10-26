//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef UAAREPORT_H
#define UAAREPORT_H

#include <QDialog>

namespace Ui {
    class UAAReport;
}

namespace Huggle
{
    class UAAReport : public QDialog
    {
            Q_OBJECT

        public:
            explicit UAAReport(QWidget *parent = 0);
            ~UAAReport();

        private:
            Ui::UAAReport *ui;
    };
}

#endif // UAAREPORT_H
