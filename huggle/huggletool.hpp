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

#include <QString>
#include <QTimer>
#include <QDockWidget>
#include <QFont>
#include "apiquery.hpp"
#include "wikipage.hpp"
#include "wikiedit.hpp"
#include "exception.hpp"
#include "configuration.hpp"

namespace Ui
{
    class HuggleTool;
}

namespace Huggle
{
    class WikiEdit;
    class WikiPage;

    //! Toolbar on top of window
    class HuggleTool : public QDockWidget
    {
        Q_OBJECT

    public:
        explicit HuggleTool(QWidget *parent = 0);
        ~HuggleTool();
        void SetTitle(QString title);
        void SetInfo(QString info);
        void SetUser(QString user);
        void SetPage(WikiPage* page);

    private slots:
        void on_pushButton_clicked();
        void onTick();

    private:
        Ui::HuggleTool *ui;
        ApiQuery *query;
        //! Timer that is used to switch between events that happen when the data for page are retrieved
        QTimer *tick;
        /// \todo DOCUMENT ME
        WikiEdit *edit;
        //! Page download phase

        //! When we download a page from wiki we need to do that in several steps, this variable holds
        //! the information which step we are in
        int QueryPhase;
        void FinishPage();
        void FinishEdit();
        void DeleteQuery();
    };
}

#endif // HUGGLETOOL_H
