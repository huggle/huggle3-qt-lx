//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef REQUESTPROTECT_H
#define REQUESTPROTECT_H

#include <huggle_core/definitions.hpp>

#include <QTimer>
#include "hw.hpp"
#include <huggle_core/wikipage.hpp>
#include <huggle_core/apiquery.hpp>
#include <huggle_core/editquery.hpp>
#include <huggle_core/collectable_smartptr.hpp>

namespace Ui
{
    class RequestProtect;
}

namespace Huggle
{
    class EditQuery;
    class ApiQuery;

    //! This can be used to request protection of a page
    class HUGGLE_EX_UI RequestProtect : public HW
    {
            Q_OBJECT
        public:
            explicit RequestProtect(WikiPage *wikiPage, QWidget *parent = nullptr);
            ~RequestProtect() override;
        private slots:
            void Tick();
            void on_pushButton_RequestProtection_clicked();
            void on_pushButton_Cancel_clicked();

        private:
            QString ProtectionType();
            void Fail(const QString &message);
            QString Timestamp;
            WikiPage *page;
            QTimer *tm;
            Ui::RequestProtect *ui;
            Collectable_SmartPtr<ApiQuery> qRFPPage;
            Collectable_SmartPtr<EditQuery> qEditRFP;
    };
}


#endif // REQUESTPROTECT_H
