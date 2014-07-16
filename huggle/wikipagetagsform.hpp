//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WIKIPAGETAGSFORM_HPP
#define WIKIPAGETAGSFORM_HPP

#include "definitions.hpp"

#include <QDialog>
#include <QCheckBox>
#include <QStringList>
#include <QString>

namespace Ui
{
    class WikiPageTagsForm;
}

namespace Huggle
{
    // fwdecl
    class Query;
    class WikiPage;
    //! This is a callback for query we use to read the page
    void WikiPageTagsForm_FinishRead(Query *result);

    //! Form used to tag page
    class WikiPageTagsForm : public QDialog
    {
            Q_OBJECT
        public:
            explicit WikiPageTagsForm(QWidget *parent = nullptr);
            ~WikiPageTagsForm();
            void ChangePage(WikiPage *wikipage);
        private slots:
            void on_pushButton_clicked();
        private:
            friend void Huggle::WikiPageTagsForm_FinishRead(Query *result);
            WikiPage *page = nullptr;
            QList<QCheckBox*> CheckBoxes;
            Ui::WikiPageTagsForm *ui;
    };
}

#endif // WIKIPAGETAGSFORM_HPP
