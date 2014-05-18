//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglequeueitemlabel.hpp"
#include "exception.hpp"
#include "mainwindow.hpp"
#include "ui_hugglequeueitemlabel.h"

using namespace Huggle;

HuggleQueueItemLabel::HuggleQueueItemLabel(QWidget *parent) : QFrame(parent), ui(new Ui::HuggleQueueItemLabel)
{
    this->ParentQueue = (HuggleQueue*)parent;
    this->ui->setupUi(this);
}

HuggleQueueItemLabel::~HuggleQueueItemLabel()
{
    delete this->ui;
}

void HuggleQueueItemLabel::SetName(QString name)
{
    this->ui->label_2->setText(name);
    if (this->Page != NULL)
    {
        switch (this->Page->Page->GetNS()->GetID())
        {
            case MEDIAWIKI_NSID_MAIN:
                break;
            case MEDIAWIKI_NSID_TALK:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #82E682; }");
                break;
            case MEDIAWIKI_NSID_PROJECT:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #6699FF; }");
                break;
            case MEDIAWIKI_NSID_PROJECTTALK:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #6600FF; }");
                break;
            case MEDIAWIKI_NSID_USER:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FF99FF; }");
                break;
            case MEDIAWIKI_NSID_USERTALK:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #CC66FF; }");
                break;
            case MEDIAWIKI_NSID_HELP:
            case MEDIAWIKI_NSID_MEDIAWIKI:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FFFFCC; }");
                break;
            case MEDIAWIKI_NSID_HELPTALK:
            case MEDIAWIKI_NSID_MEDIAWIKITALK:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FFCC66; }");
                break;
            case MEDIAWIKI_NSID_CATEGORY:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FF6699; }");
                break;
            case MEDIAWIKI_NSID_CATEGORYTALK:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FF0066; }");
                break;
            case MEDIAWIKI_NSID_FILE:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FF9900; }");
                break;
            case MEDIAWIKI_NSID_FILETALK:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FF6600; }");
                break;
            case MEDIAWIKI_NSID_PORTAL:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FFFF66; }");
                break;
            case MEDIAWIKI_NSID_PORTALTALK:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FF9900; }");
                break;
        }

        // change the icon according to edit type (descending priority)

        if (this->Page->OwnEdit)
        {
            this->ui->label->setPixmap(QPixmap(":/huggle/pictures/Resources/blob-me.png"));
            return;
        }

        if (this->Page->User->IsBanned)
        {
            this->ui->label->setPixmap(QPixmap(":/huggle/pictures/Resources/blob-blocked.png"));
            return;
        }

        if (this->Page->User->IsReported)
        {
            this->ui->label->setPixmap(QPixmap(":/huggle/pictures/Resources/blob-reported.png"));
            return;
        }

        if (this->Page->IsRevert)
        {
            this->ui->label->setPixmap(QPixmap(":/huggle/pictures/Resources/blob-revert.png"));
            return;
        }

        if (this->Page->Bot)
        {
            this->ui->label->setPixmap(QPixmap(":/huggle/pictures/Resources/blob-bot.png"));
            return;
        }

        switch (this->Page->CurrentUserWarningLevel)
        {
            case WarningLevelNone:
                this->ui->label->setPixmap(QPixmap(":/huggle/pictures/Resources/blob-none.png"));
                break;
            case WarningLevel1:
                this->ui->label->setPixmap(QPixmap(":/huggle/pictures/Resources/blob-warn-1.png"));
                return;
            case WarningLevel2:
                this->ui->label->setPixmap(QPixmap(":/huggle/pictures/Resources/blob-warn-2.png"));
                return;
            case WarningLevel3:
                this->ui->label->setPixmap(QPixmap(":/huggle/pictures/Resources/blob-warn-3.png"));
                return;
            case WarningLevel4:
                this->ui->label->setPixmap(QPixmap(":/huggle/pictures/Resources/blob-warn-4.png"));
                return;
        }

        if (this->Page->Score > 1000)
        {
            this->ui->label->setPixmap(QPixmap(":/huggle/pictures/Resources/blob-warning.png"));
            return;
        }

        if (this->Page->NewPage)
        {
            this->ui->label->setPixmap(QPixmap(":/huggle/pictures/Resources/blob-new.png"));
        }

        if (this->Page->User->IsIP())
        {
            this->ui->label->setPixmap(QPixmap(":/huggle/pictures/Resources/blob-anon.png"));
            return;
        }
    }
}

QString HuggleQueueItemLabel::GetName()
{
    return this->ui->label_2->text();
}

void HuggleQueueItemLabel::Process(QLayoutItem *qi)
{
    MainWindow::HuggleMain->ProcessEdit(this->Page);
    this->Remove(qi);
}

void HuggleQueueItemLabel::Remove(QLayoutItem *qi)
{
    if (this->ParentQueue->Items.contains(this))
    {
        this->ParentQueue->Items.removeAll(this);
    }
    this->ParentQueue->Delete(this, qi);
}

void HuggleQueueItemLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        this->Process();
    }
}
