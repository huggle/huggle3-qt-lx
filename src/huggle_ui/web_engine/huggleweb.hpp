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
#include <QWebEngineHistory>
#include <QWebEngineView>
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
            void Find(QString text);
            void DisplayPage(const QString &url);
            void RenderHtml(const QString &html);
            QString RetrieveHtml();
            void ToggleSearchWidget();

        private slots:
            void on_lineEdit_textChanged(const QString &arg1);

        private:
            Ui::HuggleWeb *ui;
            QString source;
    };
}

#endif // HUGGLEWEB_H
