//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "ui_custommessage.h"
#include "custommessage.hpp"
#include <huggle_core/exception.hpp>
#include <huggle_core/wikiuser.hpp>
#include <huggle_core/wikiutil.hpp>
#include <huggle_core/localization.hpp>

using namespace Huggle;

CustomMessage::CustomMessage(WikiUser *User, QWidget *parent) : HW("custommessage", this, parent), ui(new Ui::CustomMessage)
{
    this->ui->setupUi(this);
    this->user = nullptr;
    this->RestoreWindow();
    this->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    this->SetWikiUser(User);
    this->ui->pushButton_2->setText(_l("cancel"));
    this->ui->pushButton->setText(_l("custommessage-send"));
    this->ui->lSummary->setText(_l("custommessage-summary"));
    this->ui->lSubject->setText(_l("custommessage-subject"));
}

CustomMessage::~CustomMessage()
{
    delete this->user;
    delete this->ui;
}

void CustomMessage::SetWikiUser(WikiUser *User)
{
    if (User == nullptr)
    {
        throw new Huggle::NullPointerException("WikiUser *User", BOOST_CURRENT_FUNCTION);
    }
    this->user = new WikiUser(User);
    this->setWindowTitle(_l("custommessage-title", this->user->Username));
    this->ui->leSummary->setText(_l("custommessage-wikiuser-lesummary", this->user->Username));
    this->ui->plainTextEdit->setPlainText(QString(_l("custommessage-wikiuser-plaintext-1", this->user->Username) + "\n\n" + _l("custommessage-wikiuser-plaintext-2") + "\n\n~~~~"));
}

void CustomMessage::on_cancelButton_clicked()
{
    this->close();
}

void CustomMessage::on_sendButton_clicked()
{
    // plainTextEdit: wikitext message ; lineEdit: section title
    WikiUtil::MessageUser(this->user, this->ui->plainTextEdit->toPlainText(), this->ui->lineEdit->text(), this->ui->leSummary->text());
    this->close();
}

bool CustomMessage::VerifyMessage()
{
    if (this->ui->plainTextEdit->toPlainText().isEmpty() || this->ui->lineEdit->text().isEmpty() || this->ui->leSummary->text().isEmpty())
    {
        this->ui->sendButton->setEnabled(false);
        return false;
    } else
    {
        this->ui->sendButton->setEnabled(true);
        return true;
    }
}

void Huggle::CustomMessage::on_lineEdit_textChanged()
{
    this->VerifyMessage();
}

void Huggle::CustomMessage::on_plainTextEdit_textChanged()
{
    this->VerifyMessage();
}
