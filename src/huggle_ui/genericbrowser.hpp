//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEGENERICBROWSERWEB_H
#define HUGGLEGENERICBROWSERWEB_H

#include <huggle_core/definitions.hpp>

#include <QFrame>
#include <huggle_core/collectable_smartptr.hpp>
#include <huggle_core/wikiedit.hpp>

namespace Ui
{
    class HuggleWeb;
}

namespace Huggle
{
    class WikiPage;
    class Resources;

    class HUGGLE_EX_UI GenericBrowser : public QFrame
    {
            Q_OBJECT
        public:
            explicit GenericBrowser(QWidget *parent = nullptr);
            virtual ~GenericBrowser();
            virtual QString CurrentPageName();
            /*!
             * \brief Retrieve a page in render mode on currently selected project
             * \param page
             */
            virtual void DisplayPreFormattedPage(WikiPage *page);
            /*!
             * \brief Open a page but append action=render to it
             * \param url
             */
            virtual void DisplayPreFormattedPage(QString url);
            virtual void DisplayPage(const QString &url)=0;
            /*!
             * \brief Display a html text in window of huggle
             * \param html
             */
            virtual void RenderHtml(const QString &html)=0;
            /*!
             * \brief Display a diff of an edit using its RevID
             * Either uses an api or in case that api fails, the page is downloaded using standard rendering
             * \param edit
             */
            virtual void DisplayDiff(WikiEdit *edit);
            virtual void DisplayNewPageEdit(WikiEdit *edit);
            virtual void Find(QString text);
            virtual QString RetrieveHtml()=0;
            static QString Encode(const QString &string);
            Collectable_SmartPtr<WikiEdit> CurrentEdit;

        private:
            virtual QString GetShortcut();
            QString CurrentPage;
    };
}

#endif // HUGGLEWEB_H
