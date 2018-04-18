//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEWEBPAGE_H
#define HUGGLEWEBPAGE_H

#include "../definitions.hpp"

#include <QWebEnginePage>

namespace Huggle
{
    //! This is to allow us to overload some of the WebEngine code
    class HuggleWebEnginePage : public QWebEnginePage
    {
        Q_OBJECT
        public:
            bool acceptNavigationRequest(const QUrl & url, QWebEnginePage::NavigationType type, bool isMainFrame);
    };
}

#endif
