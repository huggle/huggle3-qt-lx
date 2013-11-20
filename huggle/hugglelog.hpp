//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLELOG_H
#define HUGGLELOG_H

#include <QString>
#include <QCursor>
#include <QDockWidget>
#include <QDateTime>
#include "exception.hpp"
#include "huggleweb.hpp"
#include "syslog.hpp"

namespace Ui
{
    class HuggleLog;
}

namespace Huggle
{
    //! This window contains all the messages that are stored in ring log
    class HuggleLog : public QDockWidget
    {
            Q_OBJECT
        public:
            explicit HuggleLog(QWidget *parent = 0);
            void InsertText(QString text);
            QString Format(QString date, QString text);
            ~HuggleLog();
        private:
            Ui::HuggleLog *ui;
    };
}

#endif // HUGGLELOG_H
