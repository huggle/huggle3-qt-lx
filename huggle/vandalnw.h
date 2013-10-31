//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef VANDALNW_H
#define VANDALNW_H

#include <QDockWidget>
#include <QTimer>
#include "networkirc.h"
#include "wikiedit.h"
#include "configuration.h"
#include "core.h"

namespace Ui
{
    class VandalNw;
}

namespace Huggle
{
    //! Vandalism network

    //! Huggle 3 comes with a system that allows all clients to operate together in order
    //! to be effective in reverting of vandalism
    class VandalNw : public QDockWidget
    {
        Q_OBJECT

    public:
        explicit VandalNw(QWidget *parent = 0);
        ~VandalNw();
        /*!
         * \brief Insert text to window
         * \param text is a string that will be inserted to window, must not be terminated with newline
         */
        void Insert(QString text);
        void Connect();
        void Disconnect();
        //! This will deliver an edit to others as a good edit
        void Good(WikiEdit *Edit);
        //! Notify others about a rollback of edit
        void Rollback(WikiEdit *Edit);
        void SuspiciousWikiEdit(WikiEdit *Edit);
        void WarningSent(WikiUser *user, int Level);
        QString pref;
        QTimer *tm;
    private:
        Ui::VandalNw *ui;
        IRC::NetworkIrc *Irc;
        bool JoinedMain;
    private slots:
        void onTick();
    };
}

#endif // VANDALNW_H
