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
    #ifdef QT6_BUILD
        #include <QAudioOutput>
        QAudioOutput* Huggle::Resources::audioOutput = nullptr;
    #endif
    QMediaPlayer* Huggle::Resources::mediaPlayer = nullptr;
#endif
#include <random>
#include <chrono>

QString Huggle::Resources::DiffFooter;
QString Huggle::Resources::DiffHeader;
QString Huggle::Resources::HtmlFooter;
QString Huggle::Resources::HtmlHeader;
QString Huggle::Resources::HtmlHeader_Dark;
QString Huggle::Resources::HtmlIncoming;
QString Huggle::Resources::Html_Default_EmptyQueuePage;
QString Huggle::Resources::Html_StopFire;
QString Huggle::Resources::Html_NewTab;
QString Huggle::Resources::CssRtl;
int     Huggle::Resources::proTipCount = -100;

static int last_tip = -1;

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

QString Huggle::Resources::GetRandomProTip()
{
    // In case we don't know how many protips we have, let's count them and cache here
    if (Resources::proTipCount < 0)
    {
        int current_tip = 1;
        while (Localizations::HuggleLocalizations->KeyExists("tip" + QString::number(current_tip)))
            current_tip++;
        Resources::proTipCount = current_tip - 1;
    }

    int random = GetRandom(1, Resources::proTipCount);
    //! \todo Figure out why GetRandom returns 0 even if min is 1
    while (last_tip == random || random == 0)
        random = GetRandom(1, Resources::proTipCount);

    last_tip = random;

    return _l("tip" + QString::number(last_tip));
}

void Huggle::Resources::Init()
{
    HtmlHeader = GetResource("/huggle/resources/Resources/html/Header.html");
    HtmlHeader_Dark = GetResource("/huggle/resources/Resources/html/HeaderDark.html");
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
    #ifdef QT6_BUILD
        audioOutput = new QAudioOutput();
        mediaPlayer->setAudioOutput(audioOutput);
    #endif
#endif
}

void Huggle::Resources::Uninit()
{
#ifndef HUGGLE_NOAUDIO
    mediaPlayer->deleteLater();
    mediaPlayer = nullptr;
    #ifdef QT6_BUILD
        audioOutput->deleteLater();
        audioOutput = nullptr;
    #endif
#endif
}

void Huggle::Resources::PlayExternalSoundFile(QString path)
{
#ifndef HUGGLE_NOAUDIO
    #ifdef QT6_BUILD
        mediaPlayer->setSource(QUrl::fromLocalFile(path));
        audioOutput->setVolume(100);
        mediaPlayer->play();
    #else
        mediaPlayer->setMedia(QUrl::fromLocalFile(path));
        mediaPlayer->setVolume(100);
        mediaPlayer->play();
    #endif
#endif
}

void Huggle::Resources::PlayEmbeddedSoundFile(QString file)
{
#ifndef HUGGLE_NOAUDIO
    #ifdef QT6_BUILD
        mediaPlayer->setSource(QUrl("qrc:/huggle/sounds/" + file));
        audioOutput->setVolume(100);
        mediaPlayer->play();
    #else
        mediaPlayer->setMedia(QUrl("qrc:/huggle/sounds/" + file));
        mediaPlayer->setVolume(100);
        mediaPlayer->play();
    #endif
#endif
}

QString Huggle::Resources::GetHtmlHeader(WikiSite *site)
{
    QString Css, body_css;
    if (site->IsRightToLeft)
    {
        Css.append(Resources::CssRtl);
    }

    if (hcfg->SystemConfig_EnforceBlackAndWhiteCss)
    {
        body_css = "background-color: white;"\
                   "color: black;";
    }

    QString header;

    if (hcfg->SystemConfig_ColorScheme == 1)
        header = Resources::HtmlHeader_Dark;
    else
        header = Resources::HtmlHeader;

    return QString(header).replace("<<<CUSTOM-CSS>>>", Css)
            .replace("<<<CUSTOM-BODY-CSS>>>", body_css)
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
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(low, high);
    return dist(rng);
}
