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

#include "hw.hpp"
#include <QCheckBox>
#include <QLineEdit>
#include <QHash>
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
    class HUGGLE_EX WikiPageTagsForm : public HW
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
            QHash<QString,QLineEdit*> Arguments;
            QHash<QString,QCheckBox*> CheckBoxes;
            Ui::WikiPageTagsForm *ui;
    };
}

#endif // WIKIPAGETAGSFORM_HPP
