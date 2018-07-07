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
#include "uigeneric.hpp"
#include <QMouseEvent>
#include <huggle_core/scripting/script.hpp>

using namespace Huggle;

int OverlayBox::GetDefaultOverlayWidth()
{
    return HUGGLE_OVERLAY_DEFAULT_WIDTH;
}

OverlayBox *OverlayBox::ShowOverlay(QWidget *parent, QString text, int x, int y, int timeout, int width, int height, bool dismissable)
{
    OverlayBox *o = new OverlayBox(text, parent);
    o->SetPosition(x, y);
    o->SetTimeout(timeout);
    o->SetDismissableOnClick(dismissable);
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
    this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_DeleteOnClose);
    connect(&this->destroyTimer, SIGNAL(timeout()), this, SLOT(timer()));
    this->destroyTimer.setInterval(5000);
    this->destroyTimer.start();
}

OverlayBox::~OverlayBox()
{
    delete this->ui;
}

void OverlayBox::Close()
{
    if (this->isPersistent)
        this->isPersistent = false;
    this->close();
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

void OverlayBox::SetText(QString text)
{
    this->ui->label->setText(text);
}

void OverlayBox::SetPersistent(bool yes)
{
    this->isPersistent = yes;
    this->isDissmissableOnLink = !yes;
}

void OverlayBox::SetDismissableOnClick(bool yes)
{
    this->isDissmissableOnClick = yes;
}

void OverlayBox::Resize(int width, int height)
{
    this->resize(width, height);
}

void Huggle::OverlayBox::on_label_linkActivated(const QString &link)
{
    QUrl url(link);
    if (url.scheme() == "hgjs")
        Script::ProcessURL(url);
    else if (url.scheme() == "huggle")
        UiGeneric::ProcessURL(url);
    if (this->isDissmissableOnLink)
        this->Close();
}

void OverlayBox::timer()
{
    this->destroyTimer.stop();
    this->close();
}

void OverlayBox::reject()
{
    if (this->isPersistent)
        return;
    QDialog::reject();
}

void OverlayBox::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (!this->isPersistent && this->isDissmissableOnClick)
        this->close();
}
