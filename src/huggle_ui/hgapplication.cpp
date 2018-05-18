//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hgapplication.hpp"
#include <huggle_core/exception.hpp>
#include <huggle_core/core.hpp>

using namespace Huggle;

bool HgApplication::notify(QObject *receiver, QEvent *event)
{
    bool done = true;
    try
    {
        done = QApplication::notify(receiver, event);
    }catch (Huggle::Exception *ex)
    {
        Core::HandleException(ex);
        delete ex;
    }catch (Huggle::Exception &ex)
    {
        Core::HandleException(&ex);
    }
    return done;
}
