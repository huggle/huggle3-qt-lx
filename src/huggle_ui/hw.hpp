//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HW_HPP
#define HW_HPP

#include <QString>
#include <QDialog>
#include <huggle_core/definitions.hpp>

namespace Huggle
{
    //! Huggle Window base class, it provides some basic save / restore layout functionality shared by all Huggle windows
    class HUGGLE_EX_UI HW : public QDialog
    {
        Q_OBJECT
        public:
            HW(QString window_name, QWidget *widget, QWidget *parent = 0);
            virtual ~HW();
            void RestoreWindow();
        private:
            QString HW_Geometry;
            QString HW_Name;
            QWidget *HW_Widget;
    };
}

#endif
