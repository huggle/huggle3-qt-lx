//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HGAPPLICATION_H
#define HGAPPLICATION_H

#include <huggle_core/definitions.hpp>
#include <QApplication>

namespace Huggle
{
    //! Override of qapplication so that we can reimplement notify
    class HUGGLE_EX_UI HgApplication : public QApplication
    {
        public:
            HgApplication(int& argc, char** argv) : QApplication(argc, argv) {}
            bool notify(QObject* receiver, QEvent* event);
    };
}

#endif // HGAPPLICATION_H
