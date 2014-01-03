//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEMASSROLLBACK_H
#define HUGGLEMASSROLLBACK_H

#include <QDialog>

namespace Ui
{
    class HuggleMassRollback;
}

namespace Huggle
{
    //! Window that allows users to mass rollback the edits done by a certain user
    class HuggleMassRollback : public QDialog
    {
            Q_OBJECT

        public:
            explicit HuggleMassRollback(QWidget *parent = 0);
            ~HuggleMassRollback();

        private:
            Ui::HuggleMassRollback *ui;
    };
}

#endif // HUGGLEMASSROLLBACK_H
