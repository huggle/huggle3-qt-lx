//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef WELCOMEINFO_HPP
#define WELCOMEINFO_HPP

#include "definitions.hpp"

#include <QDialog>
#include <QUrl>

namespace Ui
{
    class WelcomeInfo;
}

namespace Huggle
{
    class WelcomeInfo : public QDialog
    {
            Q_OBJECT

        public:
            explicit WelcomeInfo(QWidget *parent = 0);
            ~WelcomeInfo();
            void DisableFirst();

        private slots:
            void on_pushButton_clicked();
            void on_label_2_linkActivated(const QString &link);
            void on_cb_Language_currentIndexChanged(const QString &arg1);

        private:
            void Localize();
            bool loading = true;
            Ui::WelcomeInfo *ui;
    };
}

#endif // WELCOMEINFO_HPP
