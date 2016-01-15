//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "ircchattextbox.hpp"

using namespace Huggle;


IRCChatTextBox::IRCChatTextBox(QWidget *parent) : QPlainTextEdit(parent)
{
    this->setReadOnly(true);
    this->setUndoRedoEnabled(false);
}

void IRCChatTextBox::dropEvent(QDropEvent *e)
{
    (void) (e);
    // do nothing here
    // this fixes random hang-up bugs on some systems
}

void IRCChatTextBox::mousePressEvent(QMouseEvent *e)
{
    this->clickedAnchor = (e->button() & Qt::LeftButton) ? anchorAt(e->pos()) : QString();
    QPlainTextEdit::mousePressEvent(e);
}

void IRCChatTextBox::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() & Qt::LeftButton && !this->clickedAnchor.isEmpty() && anchorAt(e->pos()) == this->clickedAnchor)
    {
        emit this->Event_Link(this->clickedAnchor);
    }

    QPlainTextEdit::mouseReleaseEvent(e);
}
