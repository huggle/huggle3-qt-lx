//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "userinfoform.hpp"
#include "ui_userinfoform.h"

using namespace Huggle;

UserinfoForm::UserinfoForm(QWidget *parent) : QDockWidget(parent), ui(new Ui::UserinfoForm)
{
    this->timer = new QTimer(this);
    this->q = NULL;
    this->edit = NULL;
    this->ui->setupUi(this);
    this->ui->pushButton->setEnabled(false);
    connect(this->timer, SIGNAL(timeout()), this, SLOT(OnTick()));
    QStringList header;
    this->ui->tableWidget->setColumnCount(3);
    header << Localizations::HuggleLocalizations->Localize("page") <<
              Localizations::HuggleLocalizations->Localize("time") <<
              Localizations::HuggleLocalizations->Localize("id");
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
#if QT_VERSION >= 0x050000
// Qt5 code
    this->ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    this->ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    this->ui->tableWidget->setShowGrid(false);
}

UserinfoForm::~UserinfoForm()
{
    delete this->timer;
    delete this->ui;
}

void UserinfoForm::ChangeUser(WikiUser *user)
{
    if (user == NULL)
    {
        throw new Exception("WikiUser *user can't be NULL in this fc", "void UserinfoForm::ChangeUser(WikiUser *user)");
    }
    this->User = user;
    this->ui->pushButton->show();
    this->ui->pushButton->setEnabled(true);
    this->ui->pushButton->setText("Retrieve info");
    if (this->edit != NULL)
    {
        this->edit->UnregisterConsumer(HUGGLECONSUMER_USERINFO);
    }
    while (this->ui->tableWidget->rowCount() > 0)
    {
        this->ui->tableWidget->removeRow(0);
    }
    this->ui->label->setText("Flags: " + user->Flags() + " Score: " + QString::number(user->getBadnessScore()) + " level: " + QString::number(user->WarningLevel));
}

void UserinfoForm::Read()
{
    this->q = new ApiQuery();
    this->q->Target = "Retrieving contributions of " + this->User->Username;
    this->q->SetAction(ActionQuery);
    this->q->Parameters = "list=recentchanges&rcuser=" + QUrl::toPercentEncoding(this->User->Username) +
            "&rcprop=user%7Ccomment%7Ctimestamp%7Ctitle%7Cids%7Csizes&rclimit=20&rctype=edit%7Cnew";
    Core::HuggleCore->AppendQuery(this->q);
    this->q->RegisterConsumer(HUGGLECONSUMER_USERINFO);
    ui->pushButton->hide();
    this->q->Process();
    this->timer->start(600);
}

void UserinfoForm::on_pushButton_clicked()
{
    this->Read();
}

void UserinfoForm::OnTick()
{
    if (this->edit != NULL)
    {
        if (this->edit->IsPostProcessed())
        {
            Core::HuggleCore->Main->ProcessEdit(this->edit, false, false, true);
            this->edit->UnregisterConsumer(HUGGLECONSUMER_USERINFO);
            this->edit = NULL;
        }
        return;
    }
    if (this->q == NULL)
    {
        this->timer->stop();
        return;
    }
    if (this->q->Processed())
    {
        if (this->q->Result->Failed)
        {
            this->q->UnregisterConsumer(HUGGLECONSUMER_USERINFO);
            Syslog::HuggleLogs->Log("ERROR: unable to retrieve history for user: " + this->User->Username);
            this->q = NULL;
            this->timer->stop();
            return;
        }
        QDomDocument d;
        d.setContent(this->q->Result->Data);
        QDomNodeList results = d.elementsByTagName("rc");
        int xx = 0;
        if (results.count() > 0)
        {
            while (results.count() > xx)
            {
                QDomElement edit = results.at(xx).toElement();
                if (!edit.attributes().contains("type"))
                {
                    continue;
                }
                QString page = "unknown page";
                if (edit.attributes().contains("title"))
                {
                    page = edit.attribute("title");
                }
                QString time = "unknown time";
                if (edit.attributes().contains("timestamp"))
                {
                    time = edit.attribute("timestamp");
                }
                QString diff = "";
                if (edit.attributes().contains("revid"))
                {
                    diff = edit.attribute("revid");
                }
                int last = this->ui->tableWidget->rowCount();
                this->ui->tableWidget->insertRow(last);
                this->ui->tableWidget->setItem(last, 0, new QTableWidgetItem(page));
                this->ui->tableWidget->setItem(last, 1, new QTableWidgetItem(time));
                this->ui->tableWidget->setItem(last, 2, new QTableWidgetItem(diff));
                xx++;
            }
        } else
        {
            Syslog::HuggleLogs->Log("ERROR: unable to retrieve history for user: " + this->User->Username);
        }
        this->q->UnregisterConsumer(HUGGLECONSUMER_USERINFO);
        this->q = NULL;
        this->timer->stop();
    }
    this->timer->stop();
}

void UserinfoForm::on_tableWidget_clicked(const QModelIndex &index)
{
    if (this->q != NULL)
    {
        // we must not retrieve edit until previous operation did finish
        return;
    }

    if (this->ui->tableWidget->rowCount() == 0)
    {
        return;
    }

    // check if we don't have this edit in a buffer
    int x = 0;
    int revid = this->ui->tableWidget->item(index.row(), 2)->text().toInt();
    if (revid == 0)
    {
        return;
    }
    WikiEdit::Lock_EditList->lock();
    while (x < WikiEdit::EditList.count())
    {
        WikiEdit *edit = WikiEdit::EditList.at(x);
        x++;
        if (edit->RevID == revid)
        {
            Core::HuggleCore->Main->ProcessEdit(edit, true, false, true);
            WikiEdit::Lock_EditList->unlock();
            return;
        }
    }
    WikiEdit::Lock_EditList->unlock();
    // there is no such edit, let's get it
    this->edit = new WikiEdit();
    this->edit->User = new WikiUser(this->User->Username);
    this->edit->Page = new WikiPage(this->ui->tableWidget->item(index.row(), 0)->text());
    this->edit->RevID = revid;
    this->edit->RegisterConsumer(HUGGLECONSUMER_USERINFO);
    Core::HuggleCore->PostProcessEdit(this->edit);
    /// \todo LOCALIZE ME
    Core::HuggleCore->Main->Browser->RenderHtml("Please wait...");
    this->timer->start(800);
}
