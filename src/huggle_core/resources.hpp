//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef RESOURCES_HPP
#define RESOURCES_HPP

#include "definitions.hpp"

#include <QString>

#ifndef HUGGLE_NOAUDIO
class QMediaPlayer;
#endif

namespace Huggle
{
    //! Embedded resource files
    class HUGGLE_EX_CORE Resources
    {
        public:
            static void Init();
            static void Uninit();
            static void PlayExternalSoundFile(QString path);
            static void PlayEmbeddedSoundFile(QString file);
            static QString GetHtmlHeader(Huggle::WikiSite *site);
            static QString GetEmptyQueueHTML();
            static QString GetNewTabHTML();
            static QString GetResource(QString path);
            static QByteArray GetResourceAsBinary(QString path);
            static QString GetRandomProTip();
            static int GetRandom(int low, int high);
            static QString HtmlIncoming;
            //! This string contains a html header
            static QString HtmlHeader;
            static QString DiffHeader;
            static QString DiffFooter;
            static QString Html_StopFire;
            static QString Html_Default_EmptyQueuePage;
            static QString Html_NewTab;
            static QString CssRtl;
            //! This string contains a html footer
            static QString HtmlFooter;
        private:
            static int proTipCount;
#ifndef HUGGLE_NOAUDIO
            static QMediaPlayer* mediaPlayer;
#endif
    };
}

#endif // RESOURCES_HPP
