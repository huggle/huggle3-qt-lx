//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglelog.hpp"
#include "ui_hugglelog.h"

using namespace Huggle;

HuggleLog::HuggleLog(QWidget *parent) : QDockWidget(parent), ui(new Ui::HuggleLog)
{
    this->ui->setupUi(this);
    this->setWindowTitle(Localizations::HuggleLocalizations->Localize("system-widget-name"));
    this->ui->textEdit->resize(this->ui->textEdit->width(), 60);
}

void HuggleLog::InsertText(QString text)
{
    QString t = this->ui->textEdit->toHtml();
    // first we need to split the lines
    if (text.contains("\n"))
    {
        QStringList lines = text.split("\n");
        QStringList formatted;
        // now we format each line
        int x = 0;
        while (x < lines.count())
        {
            QString l0 = lines.at(x);
            if (l0.contains("   "))
            {
                QString date = l0.mid(0, l0.indexOf("   "));
                QString line = l0.mid(l0.indexOf("   ") + 2);
                l0 = this->Format(date, line);
            }
            formatted.append(l0);
            x++;
        }
        x = 0;
        QString temp = "";
        while (x < formatted.count())
        {
            temp += formatted.at(x) + "<br>\n";
            x++;
        }
        t.prepend(temp);
    } else
    {
        // reformat line
        if (text.contains("   "))
        {
            QString date = text.mid(0, text.indexOf("   "));
            QString line = text.mid(text.indexOf("   ") + 2);
            text = this->Format(date, line);
        }
        t.prepend(text);
    }

    this->ui->textEdit->setHtml(t);
}

QString HuggleLog::Format(QString date, QString text)
{
    QString color = "";
    if (text.contains("DEBUG"))
    {
        color = "green";
    } else if(text.contains("WARNING"))
    {
        color = "orange";
    } else if (text.contains("ERROR"))
    {
        color = "red";
    }

    if (color == "")
    {
        return "<font color=blue>" + date + "</font>" + "<font>" + HuggleWeb::Encode(text) + "</font>";
    } else
    {
        return "<font color=blue>" + date + "</font>" + "<font color=" + color + ">" + HuggleWeb::Encode(text) + "</font>";
    }
}

HuggleLog::~HuggleLog()
{
    delete this->ui;
}
