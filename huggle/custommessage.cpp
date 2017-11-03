//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "custommessage.hpp"
#include "exception.hpp"
#include "wikiuser.hpp"
#include "wikiutil.hpp"
#include "localization.hpp"
#include "ui_custommessage.h"

using namespace Huggle;

CustomMessage::CustomMessage(WikiUser *User, QWidget *parent) : HW("custommessage", this, parent), ui(new Ui::CustomMessage)
{
    this->ui->setupUi(this);
    this->user = nullptr;
    this->RestoreWindow();
    this->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    this->SetWikiUser(User);
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
    this->setWindowTitle("Send a custom message to " + this->user->Username);
    this->ui->label->setText("Send to user <b>" + this->user->Username + "</b>");
    this->ui->plainTextEdit->setPlainText(QString("Hello <<<USERNAME>>>, xxx. Heartily --~~~~").replace("<<<USERNAME>>>", this->user->Username));
}

void CustomMessage::on_pushButton_2_clicked()
{
    this->hide();
}

void CustomMessage::on_pushButton_clicked()
{
    // plainTextEdit: wikitext message ; lineEdit: section title
    WikiUtil::MessageUser(this->user, this->ui->plainTextEdit->toPlainText(), this->ui->lineEdit->text(), "A message");
    this->close();
}

bool CustomMessage::VerifyMessage()
{
    if (this->ui->plainTextEdit->toPlainText().isEmpty() || this->ui->lineEdit->text().isEmpty())
    {
        this->ui->pushButton->setEnabled(false);
        return false;
    }else{
        this->ui->pushButton->setEnabled(true);
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
