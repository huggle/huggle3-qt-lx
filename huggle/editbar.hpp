//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef EDITBAR_HPP
#define EDITBAR_HPP

#include "definitions.hpp"
#include <QDockWidget>
#include <QList>
#include <QTimer>
#include "apiquery.hpp"
#include "collectable_smartptr.hpp"
#include "wikiedit.hpp"


namespace Ui
{
    class EditBar;
}

namespace Huggle
{
    class WikiEdit;
    class WikiPage;
    class WikiUser;
    class EditBarItem;

    class EditBar : public QDockWidget
    {
            Q_OBJECT
        public:
            explicit EditBar(QWidget *parent = 0);
            ~EditBar();
            void SetEdit(WikiEdit *we);
        private slots:
            void OnTick();
        private:
            void RemoveAll();
            void Read();
            Collectable_SmartPtr<WikiEdit> edit;
            Collectable_SmartPtr<ApiQuery> query;
            QList<EditBarItem*> Items;
            Ui::EditBar *ui;
            QTimer *timer;
    };
}

#endif // EDITBAR_HPP
