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
// now we need to ensure that python is included first. Don't believe? See this:
// http://stackoverflow.com/questions/20300201/why-python-h-of-python-3-2-must-be-included-as-first-together-with-qt4
#ifdef PYTHONENGINE
#include <Python.h>
#endif

#include <QFrame>
#include <QWebHistory>
#include <QWebFrame>
#include "wikipage.hpp"
#include "wikiedit.hpp"
#include "resources.hpp"

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
    class HuggleWeb : public QFrame
    {
            Q_OBJECT
        public:
            explicit HuggleWeb(QWidget *parent = 0);
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

        private slots:
            void Click(const QUrl &page);

        private:
            Ui::HuggleWeb *ui;
            QString CurrentPage;
    };
}

#endif // HUGGLEWEB_H
