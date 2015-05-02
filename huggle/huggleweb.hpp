//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLEWEB_H
#define HUGGLEWEB_H

#include "definitions.hpp"

#include <QFrame>
#include <QWebHistory>
#include <QWebFrame>
#include "collectable_smartptr.hpp"
#include "wikiedit.hpp"

namespace Ui
{
    class HuggleWeb;
}

namespace Huggle
{
    class WikiEdit;
    class WikiPage;
    class Resources;

    //! Web browser
    class HUGGLE_EX HuggleWeb : public QFrame
    {
            Q_OBJECT
        public:
            explicit HuggleWeb(QWidget *parent = nullptr);
            ~HuggleWeb();
            QString CurrentPageName();
            /*!
             * \brief Retrieve a page in render mode on currently selected project
             * \param page
             */
            void DisplayPreFormattedPage(WikiPage *page);
            /*!
             * \brief Open a page but append action=render to it
             * \param url
             */
            void DisplayPreFormattedPage(QString url);
            void DisplayPage(const QString &url);
            /*!
             * \brief Display a html text in window of huggle
             * \param html
             */
            void RenderHtml(const QString &html);
            /*!
             * \brief Display a diff of an edit using its RevID
             * Either uses an api or in case that api fails, the page is downloaded using standard rendering
             * \param edit
             */
            void DisplayDiff(WikiEdit *edit);
            void DisplayNewPageEdit(WikiEdit *edit);
            QString RetrieveHtml();
            static QString Encode(const QString &string);
            Collectable_SmartPtr<WikiEdit> CurrentEdit;

        private slots:
            void Click(const QUrl &page);

        private:
            QString GetShortcut();
            Ui::HuggleWeb *ui;
            QString CurrentPage;
    };
}

#endif // HUGGLEWEB_H
