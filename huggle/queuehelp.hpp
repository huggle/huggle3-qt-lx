//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef QUEUEHELP_HPP
#define QUEUEHELP_HPP

#include "definitions.hpp"

#include <QDialog>

namespace Ui
{
    class QueueHelp;
}

namespace Huggle
{
    class QueueHelp : public QDialog
    {
            Q_OBJECT

        public:
            explicit QueueHelp(QWidget *parent = 0);
            ~QueueHelp();

        private:
            Ui::QueueHelp *ui;
    };
}

#endif // QUEUEHELP_HPP
