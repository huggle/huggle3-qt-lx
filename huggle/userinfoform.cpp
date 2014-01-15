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
    this->qContributions = NULL;
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
    if (this->edit != NULL)
    {
        this->edit->UnregisterConsumer(HUGGLECONSUMER_USERINFO);
    }
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
        this->edit = NULL;
    }
    if (this->qContributions != NULL)
    {
        this->qContributions->UnregisterConsumer(HUGGLECONSUMER_USERINFO);
        this->qContributions = NULL;
    }
    while (this->ui->tableWidget->rowCount() > 0)
    {
        this->ui->tableWidget->removeRow(0);
    }
    this->ui->label->setText("Flags: " + user->Flags() + " Score: " + QString::number(user->getBadnessScore()) + " level: "
                                                                                    + QString::number(user->WarningLevel));
}

void UserinfoForm::Read()
{
    this->qContributions = new ApiQuery();
    this->qContributions->Target = "Retrieving contributions of " + this->User->Username;
    this->qContributions->SetAction(ActionQuery);
    this->qContributions->Parameters = "list=recentchanges&rcuser=" + QUrl::toPercentEncoding(this->User->Username) +
            "&rcprop=user%7Ccomment%7Ctimestamp%7Ctitle%7Cids%7Csizes&rclimit=20&rctype=edit%7Cnew";
    Core::HuggleCore->AppendQuery(this->qContributions);
    this->qContributions->RegisterConsumer(HUGGLECONSUMER_USERINFO);
    ui->pushButton->hide();
    this->qContributions->Process();
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
    if (this->qContributions == NULL)
    {
        this->timer->stop();
        return;
    }
    if (this->qContributions->IsProcessed())
    {
        if (this->qContributions->Result->Failed)
        {
            this->qContributions->UnregisterConsumer(HUGGLECONSUMER_USERINFO);
            Syslog::HuggleLogs->ErrorLog("unable to retrieve history for user: " + this->User->Username);
            this->qContributions = NULL;
            this->timer->stop();
            return;
        }
        QDomDocument d;
        d.setContent(this->qContributions->Result->Data);
        QDomNodeList results = d.elementsByTagName("rc");
        int xx = 0;
        if (results.count() > 0)
        {
            QColor xb;
            bool xt = false;
            while (results.count() > xx)
            {
                QDomElement edit = results.at(xx).toElement();
                if (!edit.attributes().contains("type"))
                {
                    continue;
                }
                if (xt)
                {
                    xb = QColor(206, 202, 250);
                } else
                {
                    xb = QColor(224, 222, 250);
                }
                xt = !xt;
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
                QTableWidgetItem *q = new QTableWidgetItem(page);
                q->setBackgroundColor(xb);
                this->ui->tableWidget->setItem(last, 0, q);
                q = new QTableWidgetItem(time);
                q->setBackgroundColor(xb);
                this->ui->tableWidget->setItem(last, 1, q);
                q = new QTableWidgetItem(diff);
                q->setBackgroundColor(xb);
                this->ui->tableWidget->setItem(last, 2, q);
                xx++;
            }
        } else
        {
            Syslog::HuggleLogs->ErrorLog("unable to retrieve history for user: " + this->User->Username);
        }
        this->ui->tableWidget->resizeRowsToContents();
        this->qContributions->UnregisterConsumer(HUGGLECONSUMER_USERINFO);
        this->qContributions = NULL;
        this->timer->stop();
    }
    this->timer->stop();
}

void UserinfoForm::on_tableWidget_clicked(const QModelIndex &index)
{
    if (this->qContributions != NULL)
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
    Core::HuggleCore->Main->Browser->RenderHtml(Localizations::HuggleLocalizations->Localize("wait"));
    this->timer->start(800);
}
