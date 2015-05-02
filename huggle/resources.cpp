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
#include "resources.hpp"
#include "wikisite.hpp"
#include <QFile>

QString Huggle::Resources::DiffFooter;
QString Huggle::Resources::DiffHeader;
QString Huggle::Resources::HtmlFooter;
QString Huggle::Resources::HtmlHeader;
QString Huggle::Resources::HtmlIncoming;
QString Huggle::Resources::Html_StopFire;
QString Huggle::Resources::CssRtl;

QString Huggle::Resources::GetResource(QString path)
{
    QFile *vf = new QFile(":" + path);
    if (!vf->open(QIODevice::ReadOnly))
    {
        delete vf;
        throw new Huggle::Exception("Unable to open internal resource: " + path, BOOST_CURRENT_FUNCTION);
    }

    QString result = QString(vf->readAll());
    vf->close();
    delete vf;
    return result;
}

void Huggle::Resources::Init()
{
    HtmlHeader = GetResource("/huggle/resources/Resources/html/Header.html");
    DiffHeader = GetResource("/huggle/resources/Resources/html/DiffBeginning.html");
    Html_StopFire = GetResource("/huggle/resources/Resources/html/StopFire.html");
    HtmlFooter = GetResource("/huggle/resources/Resources/html/PageEnd.html");
    DiffFooter = GetResource("/huggle/resources/Resources/html/DiffEnd.html");
    HtmlIncoming = GetResource("/huggle/resources/Resources/html/Message.html");
    CssRtl = GetResource("/huggle/resources/Resources/html/RTL.css");
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
