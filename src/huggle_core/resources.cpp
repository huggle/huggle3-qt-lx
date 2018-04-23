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
#include "localization.hpp"
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
QString Huggle::Resources::Html_NewTab;
QString Huggle::Resources::CssRtl;

int last_tip = -1;

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

#define HUGGLE_PROTIP_COUNT 14
QString Huggle::Resources::GetRandomProTip()
{
    int random = GetRandom(1, HUGGLE_PROTIP_COUNT);
    //! \todo Figure out why GetRandom returns 0 even if min is 1
    while (last_tip == random || random == 0)
        random = GetRandom(1, HUGGLE_PROTIP_COUNT);

    last_tip = random;

    return _l("tip" + QString::number(last_tip));
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
    Html_NewTab = GetResource("/huggle/resources/Resources/html/empty_tab.html");
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
    QString Css;
    if (Configuration::HuggleConfiguration->Project->IsRightToLeft)
    {
        Css.append(Resources::CssRtl);
    }

    return QString(Resources::HtmlHeader).replace("<<<CUSTOM-CSS>>>", Css)
            .replace("<<<FONT-FAMILY>>>", hcfg->SystemConfig_Font)
            .replace("<<<FONT-SIZE>>>", QString::number(hcfg->SystemConfig_FontSize));
}

QString Huggle::Resources::GetNewTabHTML()
{
    return QString(Resources::Html_NewTab).replace("<<<TITLE>>>", _l("newtab-title"))
                                          .replace("<<<PROTIP>>>", "<b>" + _l("protip") + ":</b> " + GetRandomProTip())
                                          .replace("<<<TEXT>>>", _l("newtab-text"));
}

QString Huggle::Resources::GetEmptyQueueHTML()
{
    return QString(Resources::Html_Default_EmptyQueuePage)
            .replace("<<<TITLE>>>", _l("queue-empty-title"))
            .replace("<<<PROTIP>>>", "<b>" + _l("protip") + ":</b> " + GetRandomProTip())
            .replace("<<<TEXT>>>", _l("queue-empty-text"));
}

int Huggle::Resources::GetRandom(int low, int high)
{
    static bool init = false;
    if (!init)
    {
        init = true;
        QTime time = QTime::currentTime();
        qsrand((uint)time.msec());
    }
    return qrand() % ((high + 1) - low) + low;
}
