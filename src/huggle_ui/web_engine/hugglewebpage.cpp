//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglewebpage.hpp"
#include <QDesktopServices>

using namespace Huggle;

HuggleWebEnginePage::HuggleWebEnginePage(QObject *parent) : QWebEnginePage(parent)
{
}

bool HuggleWebEnginePage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    if (type == QWebEnginePage::NavigationTypeLinkClicked)
    {
        QDesktopServices::openUrl(url);
        return false;
    }
    return true;
}
