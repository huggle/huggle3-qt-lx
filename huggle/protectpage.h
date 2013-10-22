//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef PROTECTPAGE_H
#define PROTECTPAGE_H

#include <QDialog>
#include <QString>
#include <QtXml>
#include <QTimer>
#include "apiquery.h"
#include "core.h"
#include "configuration.h"
#include "wikipage.h"

namespace Ui {
class ProtectPage;
}

namespace Huggle
{
    class WikiPage;
    class ProtectPage : public QDialog
    {
        Q_OBJECT

    public:
        explicit ProtectPage(QWidget *parent = 0);
        ~ProtectPage();
        void getTokenToProtect();
        //! Pointer to get first token
        ApiQuery *ptkq;
        //! Pointer for second token
        ApiQuery *ptkk;
        //! Pointer for	protection
        ApiQuery *ptpt;
        void checkTokenToProtect();
        void Failed(QString reason);
        void setPageToProtect(WikiPage *Page);
        void Protect();
        QString protecttoken;
    private slots:
        void on_pushButton_clicked();
        void on_pushButton_2_clicked();
        void onTick();
    private:    
        Ui::ProtectPage *ui;
        WikiPage *ptpge;
        QTimer *tt;
        int PtQueryPhase;
    };
}

#endif // PROTECTPAGE_H

