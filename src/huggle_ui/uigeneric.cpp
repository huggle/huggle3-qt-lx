//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "uigeneric.hpp"
#include "reportuser.hpp"

using namespace Huggle;

void UiGeneric::DisplayContributionBrowser(WikiUser *User, QWidget *parent)
{
    // We are using ReportUser as a contribution browser because we already have all the code for contribs
    // in there, the second parameter in constructors switches between standard report form and just
    // the contribution browser.
    ReportUser *report = new ReportUser(parent, true);
    report->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    report->SetUser(User);
    report->show();
}
