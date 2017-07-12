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
#ifndef HUGGLE_NOAUDIO
    #include <QMediaPlayer>
    QMediaPlayer* Huggle::Resources::mediaPlayer = NULL;
#endif

QString Huggle::Resources::DiffFooter;
QString Huggle::Resources::DiffHeader;
QString Huggle::Resources::HtmlFooter;
QString Huggle::Resources::HtmlHeader;
QString Huggle::Resources::HtmlIncoming;
QString Huggle::Resources::Html_Default_EmptyQueuePage;
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

QByteArray Huggle::Resources::GetResourceAsBinary(QString path)
{
    QFile *vf = new QFile(":" + path);
    if (!vf->open(QIODevice::ReadOnly))
    {
        delete vf;
        throw new Huggle::Exception("Unable to open internal resource: " + path, BOOST_CURRENT_FUNCTION);
    }

    QByteArray result = vf->readAll();
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
    Html_Default_EmptyQueuePage = GetResource("/huggle/resources/Resources/html/empty.html");
    HtmlIncoming = GetResource("/huggle/resources/Resources/html/Message.html");
    CssRtl = GetResource("/huggle/resources/Resources/html/RTL.css");
#ifndef HUGGLE_NOAUDIO
    mediaPlayer = new QMediaPlayer();
#endif
}

void Huggle::Resources::Uninit()
{
#ifndef HUGGLE_NOAUDIO
    mediaPlayer->deleteLater();
    mediaPlayer = NULL;
#endif
}

void Huggle::Resources::PlayExternalSoundFile(QString path)
{
#ifndef HUGGLE_NOAUDIO
    mediaPlayer->setMedia(QUrl::fromLocalFile(path));
    mediaPlayer->setVolume(100);
    mediaPlayer->play();
#endif
}

void Huggle::Resources::PlayEmbeddedSoundFile(QString file)
{
#ifndef HUGGLE_NOAUDIO
    mediaPlayer->setMedia(QUrl("qrc:/huggle/sounds/" + file));
    mediaPlayer->setVolume(100);
    mediaPlayer->play();
#endif
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
            .replace("<<<FONT-FAMILY>>>", hcfg->SystemConfig_Font)
            .replace("<<<FONT-SIZE>>>", QString::number(hcfg->SystemConfig_FontSize));
}
