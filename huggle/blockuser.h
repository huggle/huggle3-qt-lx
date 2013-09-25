//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.


#ifndef BLOCKUSER_H
#define BLOCKUSER_H

#include <QDialog>

namespace Ui {
class BlockUser;
}

class BlockUser : public QDialog
{
    Q_OBJECT

public:
    explicit BlockUser(QWidget *parent = 0);
    ~BlockUser();

private:
    Ui::BlockUser *ui;
};

#endif // BLOCKUSER_H
