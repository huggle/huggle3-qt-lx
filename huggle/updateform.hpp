//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef UPDATEFORM_H
#define UPDATEFORM_H

#include "definitions.hpp"

#include <QUrl>
#include <QDialog>
#include <QTimer>

namespace Ui
{
    class UpdateForm;
}

namespace Huggle
{
    class WebserverQuery;

    /*!
     * \brief Update form is shown when there is an update for huggle
     * This form may work on some platforms only
     */
    class UpdateForm : public QDialog
    {
            Q_OBJECT

        public:
            explicit UpdateForm(QWidget *parent = nullptr);
            void Check();
            ~UpdateForm();
            WebserverQuery *qData;

        private slots:
            void on_pushButton_clicked();
            void on_pushButton_2_clicked();
            void OnTick();
            void on_label_linkActivated(const QString &link);

        private:
            void reject();
            Ui::UpdateForm *ui;
            QStringList Instructions;
            QTimer *timer;
            QUrl *manualDownloadpage = nullptr;
    };
}
#endif // UPDATEFORM_H
