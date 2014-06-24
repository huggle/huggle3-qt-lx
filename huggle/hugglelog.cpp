//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglelog.hpp"
#include <QDateTime>
#include <QCursor>
#include <QMutex>
#include "localization.hpp"
#include "exception.hpp"
#include "huggleweb.hpp"
#include "syslog.hpp"
#include "ui_hugglelog.h"

using namespace Huggle;

HuggleLog::HuggleLog(QWidget *parent) : QDockWidget(parent), ui(new Ui::HuggleLog)
{
    this->ui->setupUi(this);
    this->setWindowTitle(_l("logs-widget-name"));
    this->lock = new QMutex(QMutex::Recursive);
    this->ui->textEdit->resize(this->ui->textEdit->width(), 60);
    this->Modified = false;
}

HuggleLog::~HuggleLog()
{
    delete this->lock;
    delete this->ui;
}

void HuggleLog::InsertText(HuggleLog_Line line)
{
    this->lock->lock();
    // we don't want to have too much text here because that makes the rendering too slow
    while (this->Text.count() > 82)
    {
        this->Text.removeAt(0);
    }
    this->Text.append(line);
    this->lock->unlock();
    this->Modified = true;
}

QString HuggleLog::Format(HuggleLog_Line line)
{
    QString color = "";
    switch (line.Type)
    {
        case HuggleLogType_Debug:
            color = "green";
            break;
        case HuggleLogType_Warn:
            color = "orange";
            break;
        case HuggleLogType_Normal:
            break;
        case HuggleLogType_Error:
            color = "red";
            break;
    }

    if (color.isEmpty())
    {
        return "<font color=blue>" + line.Date + "</font>" + "<font>&nbsp;&nbsp;" + HuggleWeb::Encode(line.Text) + "</font>";
    } else
    {
        return "<font color=blue>" + line.Date + "</font>" + "<font color=" + color + ">&nbsp;&nbsp;" + HuggleWeb::Encode(line.Text) + "</font>";
    }
}

void HuggleLog::Render()
{
    if (this->Modified)
    {
        int id = 0;
        QString t = "";
        this->lock->lock();
        while (id < this->Text.count())
        {
            t = Format(this->Text.at(id)) + "<br>\n" + t;
            id++;
        }
        this->Modified = false;
        this->lock->unlock();
        this->ui->textEdit->setHtml(t);
    }
}

HuggleLog_Line::HuggleLog_Line(HuggleLog_Line *line)
{
    this->Type = line->Type;
    this->Date = line->Date;
    this->Text = line->Text;
}

HuggleLog_Line::HuggleLog_Line(const HuggleLog_Line &line)
{
    this->Type = line.Type;
    this->Date = line.Date;
    this->Text = line.Text;
}

HuggleLog_Line::HuggleLog_Line(QString text, QString date)
{
    this->Type = HuggleLogType_Normal;
    this->Text = text;
    this->Date = date;
}
