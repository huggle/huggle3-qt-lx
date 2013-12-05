//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "hugglequeueitemlabel.hpp"
#include "ui_hugglequeueitemlabel.h"

using namespace Huggle;

int HuggleQueueItemLabel::Count = 0;

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
        switch (this->Page->Page->GetNS())
        {
            case MediaWikiNS_Main:
                break;
            case MediaWikiNS_Talk:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #82E682; }");
                break;
            case MediaWikiNS_Project:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #6699FF; }");
                break;
            case MediaWikiNS_ProjectTalk:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #6600FF; }");
                break;
            case MediaWikiNS_User:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FF99FF; }");
                break;
            case MediaWikiNS_UserTalk:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #CC66FF; }");
                break;
            case MediaWikiNS_Help:
            case MediaWikiNS_Mediawiki:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FFFFCC; }");
                break;
            case MediaWikiNS_HelpTalk:
            case MediaWikiNS_MediawikiTalk:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FFCC66; }");
                break;
            case MediaWikiNS_Category:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FF6699; }");
                break;
            case MediaWikiNS_CategoryTalk:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FF0066; }");
                break;
            case MediaWikiNS_File:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FF9900; }");
                break;
            case MediaWikiNS_FileTalk:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FF6600; }");
                break;
            case MediaWikiNS_Portal:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FFFF66; }");
                break;
            case MediaWikiNS_PortalTalk:
                this->ui->label_2->setStyleSheet("QLabel { background-color : #FF9900; }");
                break;
            case MediaWikiNS_Special:
                this->ui->label_2->setStyleSheet("QLabel { background-color : red; }");
                break;
        }

        // change the icon according to edit type

        if (this->Page->OwnEdit)
        {
            this->ui->label->setPixmap(QPixmap(":/huggle/pictures/Resources/blob-me.png"));
            return;
        }

        if (this->Page->User->IsReported)
        {
            this->ui->label->setPixmap(QPixmap(":/huggle/pictures/Resources/blob-reported.png"));
            return;
        }

        if (this->Page->IsRevert)
        {
            this->ui->label->setPixmap( QPixmap(":/huggle/pictures/Resources/blob-revert.png") );
            return;
        }

        if (this->Page->Bot)
        {
            this->ui->label->setPixmap( QPixmap(":/huggle/pictures/Resources/blob-bot.png") );
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
    HuggleQueueItemLabel::Count--;
    if (this->ParentQueue->Items.contains(this))
    {
        this->ParentQueue->Items.removeAll(this);
    }
    Core::HuggleCore->ProcessEdit(this->Page);
    this->close();
    this->Page->RegisterConsumer(HUGGLECONSUMER_MAINFORM);
    this->Page->UnregisterConsumer(HUGGLECONSUMER_QUEUE);
    this->ParentQueue->Delete(this, qi);
}

void HuggleQueueItemLabel::Remove()
{
    HuggleQueueItemLabel::Count--;
    if (this->ParentQueue->Items.contains(this))
    {
        this->ParentQueue->Items.removeAll(this);
    }
    this->Page->UnregisterConsumer(HUGGLECONSUMER_QUEUE);
    this->Page = NULL;
    this->close();
    this->ParentQueue->Delete(this);
}

void HuggleQueueItemLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        this->Process();
        delete this;
    }
}
