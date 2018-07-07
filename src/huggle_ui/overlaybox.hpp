//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef OVERLAYBOX_HPP
#define OVERLAYBOX_HPP

#include <huggle_core/definitions.hpp>
#include <QTimer>
#include <QDialog>

namespace Ui
{
    class OverlayBox;
}

#define HUGGLE_OVERLAY_DEFAULT_HEIGHT 80
#define HUGGLE_OVERLAY_DEFAULT_WIDTH  400

namespace Huggle
{
    class HUGGLE_EX_UI OverlayBox : public QDialog
    {
            Q_OBJECT

        public:
            static int GetDefaultOverlayWidth();
            static OverlayBox *ShowOverlay(QWidget *parent, QString text, int x, int y, int timeout, int width = -1, int height = -1, bool dismissable = false);
            explicit OverlayBox(QString text, QWidget *parent = 0);
            ~OverlayBox();
            void Close();
            void SetTransparency(qreal x);
            void SetPosition(int x, int y);
            void SetTimeout(int timeout);
            void SetText(QString text);
            void SetPersistent(bool yes);
            void SetDismissableOnClick(bool yes);
            void Resize(int width, int height);

        private slots:
            void on_label_linkActivated(const QString &link);
            void timer();

        private:
            void reject();
            void mousePressEvent(QMouseEvent *event);
            bool isDissmissableOnClick = true;
            bool isPersistent = false;
            QTimer destroyTimer;
            Ui::OverlayBox *ui;
    };
}

#endif // OVERLAYBOX_HPP
