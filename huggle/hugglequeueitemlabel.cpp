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
#include "wikipage.hpp"
#include "wikisite.hpp"
#include "wikiuser.hpp"

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
    if (this->Page != nullptr)
    {
        int id = this->Page->Page->GetNS()->GetID();
        if (id != 0)
            this->ui->label_2->setStyleSheet("QLabel { background-color : #" + getColor(id) + "; }");
        // change the icon according to edit type
        this->ui->label->setPixmap(QPixmap(this->Page->GetPixmap()));
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
        this->ParentQueue->RedrawTitle();
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

QString HuggleQueueItemLabel::getColor(int id)
{
    if (this->buffer.contains(id))
        return this->buffer[id];

    // let's create some hash color from the id
    QString color = QString(QCryptographicHash::hash(QString::number(id).toUtf8(), QCryptographicHash::Md5).toHex());
    if (color.length() > 6)
        color = color.mid(0, 6);
    this->buffer.insert(id, color);
    return color;
}
