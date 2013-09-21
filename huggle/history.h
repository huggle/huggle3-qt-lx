//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HISTORY_H
#define HISTORY_H

#include <QDockWidget>

namespace Ui {
class History;
}

class History : public QDockWidget
{
    Q_OBJECT
    
public:
    explicit History(QWidget *parent = 0);
    ~History();
    
private:
    Ui::History *ui;
};

#endif // HISTORY_H
