//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef EDITBARITEM_HPP
#define EDITBARITEM_HPP

#include "definitions.hpp"
#include <QFrame>

namespace Ui
{
    class EditBarItem;
}

namespace Huggle
{
    class EditBarItem : public QFrame
    {
            Q_OBJECT
        public:
            explicit EditBarItem(QWidget *parent = 0);
            ~EditBarItem();
            void SetLineWidth(int width);
            void SetText(QString text);
            void SetFrame(Qt::GlobalColor colour);
            void SetPixmap(QString path);
            bool IsUser = true;
            QString Page;
            QString RevID;
            int RowId = -1;
            QString Username;

        private:
            Ui::EditBarItem *ui;

        protected:
            void mousePressEvent(QMouseEvent *event);
    };
}

#endif // EDITBARITEM_HPP
