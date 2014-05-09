//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "resources.hpp"
#include "configuration.hpp"

QString Huggle::Resources::DiffFooter;
QString Huggle::Resources::DiffHeader;
QString Huggle::Resources::HtmlFooter;
QString Huggle::Resources::HtmlHeader;
QString Huggle::Resources::HtmlIncoming;
QString Huggle::Resources::Html_StopFire;

void Huggle::Resources::Init()
{
    QFile *vf;
    vf = new QFile(":/huggle/resources/Resources/html/Header.txt");
    vf->open(QIODevice::ReadOnly);
    HtmlHeader = QString(vf->readAll());
    vf->close();
    delete vf;
    vf = new QFile(":/huggle/resources/Resources/html/DiffBeginning.txt");
    vf->open(QIODevice::ReadOnly);
    DiffHeader = QString(vf->readAll());
    vf->close();
    delete vf;
    vf = new QFile(":/huggle/resources/Resources/html/StopFire.html");
    vf->open(QIODevice::ReadOnly);
    Html_StopFire = QString(vf->readAll());
    vf->close();
    delete vf;
    vf = new QFile(":/huggle/resources/Resources/html/PageEnd.txt");
    vf->open(QIODevice::ReadOnly);
    HtmlFooter = QString(vf->readAll());
    vf->close();
    delete vf;
    vf = new QFile(":/huggle/resources/Resources/html/DiffEnd.txt");
    vf->open(QIODevice::ReadOnly);
    DiffFooter = QString(vf->readAll());
    vf->close();
    delete vf;
    vf = new QFile(":/huggle/resources/Resources/html/Message.txt");
    vf->open(QIODevice::ReadOnly);
    HtmlIncoming = QString(vf->readAll());
    vf->close();
    delete vf;
}

QString Huggle::Resources::GetHtmlHeader()
{
    QString Css = "";

    /// \todo Auto detect RTL languages (rather than hardcoded fa!)
    if( Huggle::Configuration::HuggleConfiguration->Project->Name == "fawiki" )
        Css.append( "/*GENERATED IN resources.cpp*/\n"\
                    "td.diff-context {\n"\
                    "   text-align: right;\n"\
                    "}\n\n"\
                    "td.diff-addedline {\n"\
                    "   text-align: right;\n"\
                    "}\n\n"\
                    "td.diff-deletedline {\n"\
                    "   text-align: right;\n"\
                    "}\n\n" );

    return QString( Resources::HtmlHeader ).replace( "<<<CUSTOM-CSS>>>", Css );
}
