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
#include "syslog.hpp"
#include "networkirc.hpp"
#include "wikiedit.hpp"
#include "core.hpp"
#include "configuration.hpp"

namespace Ui
{
    class VandalNw;
}

namespace Huggle
{
    namespace IRC
    {
        class NetworkIrc;
    }

    namespace HAN
    {
        class GenericItem
        {
            public:
                GenericItem();
                GenericItem(int _revID, QString _user);
                GenericItem(const GenericItem &i);
                GenericItem(GenericItem *i);
                //! User who scored the edit
                QString User;
                //! ID of edit
                int RevID;
        };

        //! This class is used to store information regarding page rescoring

        //! Rescoring is an event when other user define own score for some edit,
        //! this score is then added to our own score, problem is that these scores
        //! can't be applied to edits that weren't parsed by huggle yet. So in case
        //! that other user has faster internet connection we cache the information
        //! using this class and use it later when edit is parsed
        class RescoreItem : public GenericItem
        {
            public:
                RescoreItem(int _revID, int _score, QString _user);
                RescoreItem(const RescoreItem &item);
                RescoreItem(RescoreItem *item);
                int Score;
        };

    }

    //! Vandalism network

    //! Huggle 3 comes with a system that allows all clients to operate together in order
    //! to be effective in reverting of vandalism
    class VandalNw : public QDockWidget
    {
            Q_OBJECT

            /// \todo We should make a user list of other huggle users here
            /// \todo Share a version of your huggle with others in sane way
            /// \todo Hook to VERSION
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
            QString GetChannel();
            bool IsParsed(WikiEdit *edit);
            void Rescore(WikiEdit *edit);
            void Message();
            //! Prefix to special commands that are being sent to network to other users
            QString pref;
            //! Timer that is used to connect to network
            QTimer *tm;
            QList<HAN::RescoreItem> UnparsedScores;
            QList<HAN::GenericItem> UnparsedGood;
            QList<HAN::GenericItem> UnparsedRoll;
            QList<HAN::GenericItem> UnparsedSusp;
        private:
            void ProcessGood(WikiEdit *edit, QString user);
            void ProcessRollback(WikiEdit *edit, QString user);
            void ProcessSusp(WikiEdit *edit, QString user);
            Ui::VandalNw *ui;
            //! Pointer to irc server
            Huggle::IRC::NetworkIrc *Irc;
            //! Using this we track if channel was joined or not, because we need to send
            //! the request some time after connection or irc server would skip it
            bool JoinedMain;
            QString Text;
        private slots:
            void onTick();
            void on_pushButton_clicked();
            void on_lineEdit_returnPressed();
    };
}

#endif // VANDALNW_H
