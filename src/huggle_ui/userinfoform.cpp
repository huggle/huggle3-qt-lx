//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "userinfoform.hpp"
#include "editbar.hpp"
#include "mainwindow.hpp"
#include "ui_userinfoform.h"
#include "uihooks.hpp"
#include <QtXml>
#include <huggle_core/configuration.hpp>
#include <huggle_core/exception.hpp>
#include <huggle_core/hooks.hpp>
#include <huggle_core/localization.hpp>
#ifdef HUGGLE_WEBEN
    #include "web_engine/huggleweb.hpp"
#else
    #include "webkit/huggleweb.hpp"
#endif
#include <huggle_core/querypool.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/wikiuser.hpp>
#include <huggle_core/wikipage.hpp>


using namespace Huggle;

UserinfoForm::UserinfoForm(QWidget *parent) : QDockWidget(parent), ui(new Ui::UserinfoForm)
{
    this->timer = new QTimer(this);
    this->User = nullptr;
    this->ui->setupUi(this);
    this->ui->pushButton->setEnabled(false);
    connect(this->timer, &QTimer::timeout, this, &UserinfoForm::OnTick);
    QStringList header;
    this->setWindowTitle(_l("userinfo-generic"));
    this->ui->pushButton->setText(_l("userinfo-no-user"));
    this->ui->tableWidget->setColumnCount(3);
    header << _l("page") << _l("time") << _l("id");
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    if (Configuration::HuggleConfiguration->SystemConfig_DynamicColsInList)
    {
#if QT_VERSION >= 0x050000
// Qt5 code
        this->ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
        this->ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    } else
    {
        this->ui->tableWidget->setColumnWidth(0, 180);
    }
    this->ui->tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui->tableWidget->setShowGrid(false);
}

UserinfoForm::~UserinfoForm()
{
    delete this->User;
    delete this->timer;
    delete this->ui;
}

void UserinfoForm::ChangeUser(WikiUser *user)
{
    if (user == nullptr)
        throw new Huggle::NullPointerException("WikiUser *user", BOOST_CURRENT_FUNCTION);
    if (this->User != nullptr)
        delete this->User;
    // we create a copy of this wiki user so that we ensure it doesn't get deleted meanwhile
    this->User = new WikiUser(user);
    this->ui->pushButton->show();
    this->ui->pushButton->setEnabled(true);
    this->ui->pushButton->setText(_l("userinfo-retrieve"));
    this->edit = nullptr;
    this->qContributions = nullptr;
    while (this->ui->tableWidget->rowCount() > 0)
    {
        this->ui->tableWidget->removeRow(0);
    }
    this->Items.clear();
    QString text = "Flags: " + user->Flags() + " Score: " + QString::number(user->GetBadnessScore()) + " level: "
                    + QString::number(user->GetWarningLevel());
    if (user->EditCount > 0)
    {
        text += " Edit count: " + QString::number(user->EditCount);
    }
    this->ui->label->setText(text);
}

void UserinfoForm::Read()
{
    if (!UiHooks::ContribBoxBeforeQuery(this->User, this))
        return;
    this->qContributions = new ApiQuery(ActionQuery, this->User->GetSite());
    this->qContributions->Target = "Retrieving contributions of " + this->User->Username;
    this->qContributions->Parameters = "list=usercontribs&ucuser=" + QUrl::toPercentEncoding(this->User->Username) +
                                       "&ucprop=flags%7Ccomment%7Ctimestamp%7Ctitle%7Cids%7Csize&uclimit=20";
    QueryPool::HugglePool->AppendQuery(this->qContributions);
    ui->pushButton->hide();
    this->qContributions->Process();
    this->timer->start(HUGGLE_TIMER);
}

void UserinfoForm::on_pushButton_clicked()
{
    this->Read();
}

void UserinfoForm::OnTick()
{
    if (this->edit != nullptr)
    {
        if (this->edit->IsProcessed())
        {
            MainWindow::HuggleMain->DisplayEdit(this->edit, false, false, true, true);
            this->edit.Delete();
        }
        return;
    }
    if (this->qContributions == nullptr)
    {
        this->timer->stop();
        return;
    }
    if (this->qContributions->IsProcessed())
    {
        if (this->qContributions->IsFailed())
        {
            Syslog::HuggleLogs->ErrorLog(_l("user-history-fail", this->User->Username));
            this->timer->stop();
            this->qContributions.Delete();
            return;
        }
        QDomDocument d;
        d.setContent(this->qContributions->Result->Data);
        QDomNodeList results = d.elementsByTagName("item");
        int xx = 0;
        if (results.count() > 0)
        {
            //QColor xb;
            //bool xt = false;
            while (results.count() > xx)
            {
                QDomElement edit = results.at(xx).toElement();
                if (!edit.attributes().contains("user"))
                    continue;
                UserInfoFormHistoryItem item(this->User->GetSite());
                item.Top = edit.attributes().contains("top");
                item.Name = this->User->Username;
                if (this->User->IsAnon())
                {
                    item.Type = EditType_Anon;
                }
                if (this->User->IsReported)
                    item.Type = EditType_Reported;
                if (this->User->IsBlocked)
                    item.Type = EditType_Blocked;
                /*if (item.Top)
                {
                    // set a different color for edits that are top
                    if (xt)
                        xb = QColor(110, 202, 250);
                    else
                        xb = QColor(120, 222, 250);
                } else
                {
                    if (xt)
                        xb = QColor(206, 202, 250);
                    else
                        xb = QColor(224, 222, 250);
                }*/
                //xt = !xt;
                item.Page = "unknown page";
                if (edit.attributes().contains("title"))
                    item.Page = edit.attribute("title");
                item.Date = "unknown time";
                if (edit.attributes().contains("timestamp"))
                    item.Date = edit.attribute("timestamp");
                if (edit.attributes().contains("revid"))
                    item.RevID = edit.attribute("revid");
                int last = this->ui->tableWidget->rowCount();
                this->ui->tableWidget->insertRow(last);
                QFont font;
                if (item.Top)
                {
                    font.setBold(true);
                }
                QTableWidgetItem *q = new QTableWidgetItem(item.Page);
                //q->setBackgroundColor(xb);
                //q->setTextColor(QColor(0, 0, 0));
                q->setFont(font);
                this->ui->tableWidget->setItem(last, 0, q);
                q = new QTableWidgetItem(item.Date);
                //q->setTextColor(QColor(0, 0, 0));
                //q->setBackgroundColor(xb);
                this->ui->tableWidget->setItem(last, 1, q);
                q = new QTableWidgetItem(item.RevID);
                //q->setTextColor(QColor(0, 0, 0));
                //q->setBackgroundColor(xb);
                this->Items.append(item);
                this->ui->tableWidget->setItem(last, 2, q);
                xx++;
            }
        } else
            Syslog::HuggleLogs->ErrorLog(_l("user-history-fail", this->User->Username));
        this->ui->tableWidget->resizeRowsToContents();
        MainWindow::HuggleMain->wEditBar->RefreshUser();
        this->qContributions.Delete();
        this->timer->stop();
        UiHooks::ContribBoxAfterQuery(this->User, this);
    }
}

void UserinfoForm::JumpToSpecificContrib(long revid, QString page)
{
    // in case there are no edits we can safely quit here, there is also check because
    // we must not retrieve edit until previous operation did finish
    if (this->qContributions != nullptr || this->ui->tableWidget->rowCount() == 0)
        return;

    // check if revid is useable for us
    if (revid == 0)
        return;

    // check if we don't have this edit in a buffer
    int x = 0;
    WikiEdit::Lock_EditList->lock();
    while (x < WikiEdit::EditList.count())
    {
        WikiEdit *edit = WikiEdit::EditList.at(x++);
        if (!edit->IsProcessed())
            continue;
        if (edit->RevID == revid)
        {
            MainWindow::HuggleMain->DisplayEdit(edit, true, false, true);
            WikiEdit::Lock_EditList->unlock();
            return;
        }
    }
    WikiEdit::Lock_EditList->unlock();
    // there is no such edit, let's get it
    this->edit = new WikiEdit();
    this->edit->User = new WikiUser(this->User);
    this->edit->Page = new WikiPage(page, this->User->GetSite());
    this->edit->RevID = revid;
    QueryPool::HugglePool->PreProcessEdit(this->edit);
    QueryPool::HugglePool->PostProcessEdit(this->edit);
    MainWindow::HuggleMain->Browser->RenderHtml(_l("wait"));
    this->timer->start(HUGGLE_TIMER);
}

QList<revid_ht> UserinfoForm::GetTopRevisions()
{
    QList<revid_ht> top_revisions;
    foreach (UserInfoFormHistoryItem i, this->Items)
    {
        if (i.Top)
            top_revisions.append(i.RevID.toLongLong());
    }
    return top_revisions;
}

void UserinfoForm::on_tableWidget_clicked(const QModelIndex &index)
{
    this->JumpToSpecificContrib(this->ui->tableWidget->item(index.row(), 2)->text().toLong(),
                 this->ui->tableWidget->item(index.row(), 0)->text());
}

UserInfoFormHistoryItem::UserInfoFormHistoryItem(WikiSite *site) : MediaWikiObject(site)
{

}
