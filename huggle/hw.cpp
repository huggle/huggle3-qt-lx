//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "configuration.hpp"
#include "exception.hpp"
#include "generic.hpp"
#include "hw.hpp"
#include "localization.hpp"
#include "syslog.hpp"
#include <QStringList>
#include <QWidget>
#include <QFile>

using namespace Huggle;

HW::HW(QString window_name, QWidget *widget, QWidget *parent) : QDialog(parent)
{
    this->HW_Name = window_name;
    this->HW_Geometry = Configuration::GetConfigurationPath() + this->HW_Name + "_geometry";
    this->HW_Widget = widget;
}

HW::~HW()
{
    QFile *layout = new QFile(this->HW_Geometry);
    if (!layout->open(QIODevice::ReadWrite | QIODevice::Truncate))
        throw new Huggle::Exception("Unable to write geometry to a config file for " + this->HW_Name, BOOST_CURRENT_FUNCTION);
    else
        layout->write(this->HW_Widget->saveGeometry());
    layout->close();
}

void HW::RestoreWindow()
{
    if (QFile().exists(this->HW_Geometry))
    {
        QFile *layout = new QFile(this->HW_Geometry);
        if (!layout->open(QIODevice::ReadOnly))
            Syslog::HuggleLogs->ErrorLog(_l("main-config-geom-fail", this->HW_Name));
        else if (!this->HW_Widget->restoreGeometry(layout->readAll()))
            HUGGLE_DEBUG1("Failed to restore layout of " + this->HW_Name);
        layout->close();
        delete layout;
    }
}


