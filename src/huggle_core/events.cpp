//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "events.hpp"

using namespace Huggle;

Events *Events::Global = nullptr;

Events::Events()
{

}

Events::~Events()
{

}

void Events::on_WEGood(WikiEdit *e)
{
    emit this->WikiEdit_OnGood(e);
}

void Events::on_WENewHistoryItem(HistoryItem *hi)
{
    emit this->WikiEdit_OnNewHistoryItem(hi);
}

void Events::on_WEWarningSent(WikiUser *u, byte_ht wl)
{
    emit this->WikiEdit_OnWarning(u, wl);
}

void Events::on_WESuspicious(WikiEdit *e)
{
    emit this->WikiEdit_OnSuspicious(e);
}

void Events::on_QueryPoolFinishWEPreprocess(WikiEdit *e)
{
    emit this->QueryPool_FinishPreprocess(e);
}

void Events::on_QueryPoolFinishWEPostprocess(WikiEdit *e)
{
    emit this->QueryPool_FinishPostprocess(e);
}

void Events::on_QueryPoolRemove(Query *q)
{
    emit this->QueryPool_Remove(q);
}

void Events::on_QueryPoolUpdate(Query *q)
{
    emit this->QueryPool_Update(q);
}

void Events::on_Report(WikiUser *u)
{
    emit this->Reporting_Report(u);
}

void Events::on_SReport(WikiUser *u)
{
    emit this->Reporting_SilentReport(u);
}

void Events::on_WERevert(WikiEdit *e)
{
    emit this->WikiEdit_OnRevert(e);
}

void Events::on_UpdateUser(WikiUser *wiki_user)
{
    emit this->WikiUser_Updated(wiki_user);
}

void Events::on_SMessage(QString title, QString text)
{
    emit this->System_Message(title, text);
}

void Events::on_SWarning(QString title, QString text)
{
    emit this->System_WarningMessage(title, text);
}

void Events::on_SError(QString title, QString text)
{
    emit this->System_ErrorMessage(title, text);
}

bool Events::on_SYesNoQs(QString title, QString text, bool d)
{
    bool result = d;
    emit this->System_YesNoQuestion(title, text, &result);
    return result;
}
