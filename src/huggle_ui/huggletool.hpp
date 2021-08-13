//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HUGGLETOOL_H
#define HUGGLETOOL_H

#include <huggle_core/definitions.hpp>

#include <QString>
#include <QTimer>
#include <QDockWidget>
#include <QFont>
#include <huggle_core/apiquery.hpp>
#include <huggle_core/collectable_smartptr.hpp>
#include <huggle_core/wikiedit.hpp>

namespace Ui
{
    class HuggleTool;
}

constexpr int HUGGLETOOL_DONE                         = 0;
constexpr int HUGGLETOOL_DOWNLOADING_WIKI_EDIT        = 1;
constexpr int HUGGLETOOL_PROCESS_WIKI_EDIT            = 2;
constexpr int HUGGLETOOL_RETRIEVING_USER_INFO         = 3;
constexpr int HUGGLETOOL_RETRIEVING_USER_LAST_EDIT    = 4;


namespace Huggle
{
    class ApiQuery;
    class WikiEdit;
    class WikiPage;
    class WikiSite;

    //! Toolbar on top of window
    class HUGGLE_EX_UI HuggleTool : public QDockWidget
    {
            Q_OBJECT
        public:
            explicit HuggleTool(QWidget *parent = nullptr);
            ~HuggleTool();
            void SetTitle(QString title);
            void SetInfo(QString info);
            void SetUser(QString user);
            void SetPage(WikiPage* page);
            WikiSite *GetSite();
            void DownloadEdit();

        private slots:
            void on_pushButton_clicked();
            void onTick();
            void on_lineEdit_PageName_returnPressed();
            void on_lineEdit_UserName_returnPressed();

        private:
            void finishPage();
            void finishEdit();
            QString getColor(QString color);
            Ui::HuggleTool *ui;
            WikiPage *page;
            Collectable_SmartPtr<ApiQuery> query;
            //! Timer that is used to switch between events that happen when the data for page are retrieved
            QTimer *tick;
            //! Pointer used to create an instance of page before passing it to processing function
            Collectable_SmartPtr<WikiEdit> edit;
            //! Page download phase

            //! When we download a page from wiki we need to do that in several steps, this variable holds
            //! the information which step we are in
            int queryPhase;
    };
}

#endif // HUGGLETOOL_H
