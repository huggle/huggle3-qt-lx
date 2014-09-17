//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "editbar.hpp"
#include "configuration.hpp"
#include "editbaritem.hpp"
#include "exception.hpp"
#include "wikipage.hpp"
#include "wikiuser.hpp"
#include "wikiedit.hpp"
#include "ui_editbar.h"

using namespace Huggle;

EditBar::EditBar(QWidget *parent) : QDockWidget(parent), ui(new Ui::EditBar)
{
    this->ui->setupUi(this);
    this->timer = nullptr;
}

EditBar::~EditBar()
{
    delete this->timer;
    delete this->ui;
}

void EditBar::SetEdit(WikiEdit *we)
{
    if (we == nullptr)
        throw new Huggle::NullPointerException("WikiEdit* we", "void EditBar::SetEdit(WikiEdit *we)");
    this->edit = we;
    this->RemoveAll();
    // if this bar is not shown we don't need to do anything here
    if (!this->isVisible())
        return;

    this->Read();
}

void EditBar::OnTick()
{

}

void EditBar::RemoveAll()
{
    while (this->Items.count())
    {
        delete this->Items.at(0);
        this->Items.removeAt(0);
    }
}

void EditBar::Read()
{
    this->query = new ApiQuery(ActionQuery, this->edit->GetSite());
    this->query->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("ids|flags|timestamp|user|userid|size|sha1|comment") + "&rvlimit=" +
            QString::number(Huggle::Configuration::HuggleConfiguration->UserConfig->HistoryMax) +
            "&titles=" + QUrl::toPercentEncoding(this->edit->Page->PageName);
    this->query->Process();
    delete this->timer;
    this->timer = new QTimer(this);
    connect(this->timer, SIGNAL(timeout()), this, SLOT(OnTick()));
    this->timer->start(HUGGLE_TIMER);
}
