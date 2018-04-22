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

#include <huggle_core/definitions.hpp>

#include <QFrame>
#include <QWebHistory>
#include <QWebFrame>
#include <huggle_ui/genericbrowser.hpp>

namespace Ui
{
    class HuggleWeb;
}

namespace Huggle
{
    //! Web browser
    class HUGGLE_EX_UI HuggleWeb : public GenericBrowser
    {
            Q_OBJECT
        public:
            explicit HuggleWeb(QWidget *parent = nullptr);
            ~HuggleWeb();
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
            QString RetrieveHtml();

        private slots:
            void Click(const QUrl &page);

        private:
            Ui::HuggleWeb *ui;
    };
}

#endif // HUGGLEWEB_H
