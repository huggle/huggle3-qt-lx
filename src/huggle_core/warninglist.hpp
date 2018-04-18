//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WARNINGLIST_HPP
#define WARNINGLIST_HPP

#include "definitions.hpp"

#include <QDialog>
#include "collectable_smartptr.hpp"
#include "wikiedit.hpp"

namespace Ui
{
    class WarningList;
}

namespace Huggle
{
    class WikiEdit;
    //! Widget that allows user to pick a warning to send to user
    class HUGGLE_EX WarningList : public QDialog
    {
            Q_OBJECT
        public:
            explicit WarningList(WikiEdit *edit, QWidget *parent = nullptr);
            ~WarningList();

        private slots:
            void on_pushButton_clicked();

        private:
            Collectable_SmartPtr<WikiEdit> wikiEdit;
            Ui::WarningList *ui;
    };
}

#endif // WARNINGLIST_HPP
