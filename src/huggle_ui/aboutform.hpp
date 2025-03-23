//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef ABOUTFORM_H
#define ABOUTFORM_H

#include <huggle_core/definitions.hpp>

#include "hw.hpp"
#include <QString>

namespace Ui
{
    class AboutForm;
}

namespace Huggle
{
    //! Form that can be displayed from help menu, should list all developers
    class HUGGLE_EX_UI AboutForm : public HW
    {
            Q_OBJECT
        public:
            explicit AboutForm(QWidget *parent = nullptr);
            ~AboutForm() override;

        private slots:
            void on_pushButton_clicked();
            void on_lblAuthorsDave_linkActivated(const QString &link);
            void on_lblAuthorsPetrb_linkActivated(const QString &link);
            void on_lblAuthorsAddshore_linkActivated(const QString &link);
            void on_lblAuthorsOriginalGurch_linkActivated(const QString &link);
            void on_lblAuthorsMGalloway_linkActivated(const QString &link); 
            void on_lblAuthorsSe4598_linkActivated(const QString &link);
            void on_lblAuthorsJosve05a_linkActivated(const QString &link);
            void on_lblAuthorsRichSmith_linkActivated(const QString &link);
            void on_lblAuthorsKrenair_linkActivated(const QString &link);
            void on_lblAuthorsPetrb_linkHovered(const QString &link);
            void on_lblAuthorsAddshore_linkHovered(const QString &link);

        private:
            Ui::AboutForm *ui;
    };
}

#endif // ABOUTFORM_H
