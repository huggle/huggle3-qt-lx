//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef PROTECTPAGE_H
#define PROTECTPAGE_H

#include <QDialog>

namespace Ui {
class ProtectPage;
}

namespace Huggle
{
    class ProtectPage : public QDialog
    {
        Q_OBJECT

    public:
        explicit ProtectPage(QWidget *parent = 0);
        ~ProtectPage();

    private:
        Ui::ProtectPage *ui;
    };
}

#endif // PROTECTPAGE_H
