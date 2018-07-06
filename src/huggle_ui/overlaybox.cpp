//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "overlaybox.hpp"
#include "ui_overlaybox.h"

using namespace Huggle;

int OverlayBox::GetDefaultOverlayWidth()
{
    return HUGGLE_OVERLAY_DEFAULT_WIDTH;
}

OverlayBox *OverlayBox::ShowOverlay(QWidget *parent, QString text, int x, int y, int timeout, int width, int height)
{
    OverlayBox *o = new OverlayBox(text, parent);
    o->SetPosition(x, y);
    o->SetTimeout(timeout);
    if (height > 0 && width > 0)
        o->Resize(width, height);
    o->show();
    return o;
}

OverlayBox::OverlayBox(QString text, QWidget *parent) : QDialog(parent), ui(new Ui::OverlayBox)
{
    this->ui->setupUi(this);
    this->resize(HUGGLE_OVERLAY_DEFAULT_WIDTH, HUGGLE_OVERLAY_DEFAULT_HEIGHT);
    this->ui->label->setText(text);
    this->setWindowOpacity(0.8);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_DeleteOnClose);
    connect(&this->destroyTimer, SIGNAL(timeout()), this, SLOT(timer()));
    this->destroyTimer.setInterval(5000);
    this->destroyTimer.start();
}

OverlayBox::~OverlayBox()
{
    delete this->ui;
}

void OverlayBox::SetTransparency(qreal x)
{
    this->setWindowOpacity(x);
}

void OverlayBox::SetPosition(int x, int y)
{
    this->move(x, y);
}

void OverlayBox::SetTimeout(int timeout)
{
    if (!timeout)
    {
        this->destroyTimer.stop();
        return;
    }
    this->destroyTimer.setInterval(timeout);
    this->destroyTimer.stop();
    this->destroyTimer.start();
}

void OverlayBox::Resize(int width, int height)
{
    this->resize(width, height);
}

void Huggle::OverlayBox::on_label_linkActivated(const QString &link)
{

}

void OverlayBox::timer()
{
    this->destroyTimer.stop();
    this->close();
}

void OverlayBox::mousePressEvent(QMouseEvent *event)
{
    this->close();
}
