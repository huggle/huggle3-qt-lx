//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef IRCCHATTEXTBOX_HPP
#define IRCCHATTEXTBOX_HPP

#include "definitions.hpp"

#include <QPlainTextEdit>

namespace Huggle
{
    //!
    //! \brief The IRCChatTextBox class provides a simple text box with clickable links (QPlainTextEdit doesn't support them)
    //!
    class HUGGLE_EX IRCChatTextBox : public QPlainTextEdit
    {
        Q_OBJECT

        public:
            IRCChatTextBox(QWidget *parent = nullptr);

        signals:
            void Event_Link(QString text);

        protected:
            void dropEvent(QDropEvent *e);
            void mousePressEvent(QMouseEvent *e);
            void mouseReleaseEvent(QMouseEvent *e);
            QString clickedAnchor;
    };
}

#endif // IRCCHATTEXTBOX_HPP
