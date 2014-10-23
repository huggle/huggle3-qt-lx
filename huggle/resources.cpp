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
#include "wikisite.hpp"

QString Huggle::Resources::DiffFooter;
QString Huggle::Resources::DiffHeader;
QString Huggle::Resources::HtmlFooter;
QString Huggle::Resources::HtmlHeader;
QString Huggle::Resources::HtmlIncoming;
QString Huggle::Resources::Html_StopFire;
QString Huggle::Resources::CssRtl;

void Huggle::Resources::Init()
{
    QFile *vf;
    vf = new QFile(":/huggle/resources/Resources/html/Header.html");
    vf->open(QIODevice::ReadOnly);
    HtmlHeader = QString(vf->readAll());
    vf->close();
    delete vf;
    vf = new QFile(":/huggle/resources/Resources/html/DiffBeginning.html");
    vf->open(QIODevice::ReadOnly);
    DiffHeader = QString(vf->readAll());
    vf->close();
    delete vf;
    vf = new QFile(":/huggle/resources/Resources/html/StopFire.html");
    vf->open(QIODevice::ReadOnly);
    Html_StopFire = QString(vf->readAll());
    vf->close();
    delete vf;
    vf = new QFile(":/huggle/resources/Resources/html/PageEnd.html");
    vf->open(QIODevice::ReadOnly);
    HtmlFooter = QString(vf->readAll());
    vf->close();
    delete vf;
    vf = new QFile(":/huggle/resources/Resources/html/DiffEnd.html");
    vf->open(QIODevice::ReadOnly);
    DiffFooter = QString(vf->readAll());
    vf->close();
    delete vf;
    vf = new QFile(":/huggle/resources/Resources/html/Message.html");
    vf->open(QIODevice::ReadOnly);
    HtmlIncoming = QString(vf->readAll());
    vf->close();
    delete vf;
    vf = new QFile(":/huggle/resources/Resources/html/RTL.css");
    vf->open(QIODevice::ReadOnly);
    CssRtl = QString(vf->readAll());
    vf->close();
    delete vf;
}

QString Huggle::Resources::GetHtmlHeader()
{
    QString Css = "";
    //if( Huggle::Configuration::HuggleConfiguration->Project->Name == "fawiki" )
    //    Css.append( Resources::CssRtl );
    //if( Huggle::Configuration::HuggleConfiguration->Project->Name == "arwiki" )
    //    Css.append( Resources::CssRtl );
    if (Configuration::HuggleConfiguration->Project->IsRightToLeft)
    {
        Css.append(Resources::CssRtl);
    }

    return QString(Resources::HtmlHeader).replace("<<<CUSTOM-CSS>>>", Css)
            .replace("<<<FONT-FAMILY>>>", hcfg->UserConfig->Font)
            .replace("<<<FONT-SIZE>>>", QString::number(hcfg->UserConfig->FontSize));
}
