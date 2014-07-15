//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "historyform.hpp"
#include <QTimer>
#include <QToolTip>
#include <QtXml>
#include "configuration.hpp"
#include "exception.hpp"
#include "mainwindow.hpp"
#include "localization.hpp"
#include "resources.hpp"
#include "huggleweb.hpp"
#include "syslog.hpp"
#include "ui_historyform.h"
#include "querypool.hpp"
#include "wikiuser.hpp"
#include "wikiutil.hpp"

using namespace Huggle;

HistoryForm::HistoryForm(QWidget *parent) : QDockWidget(parent), ui(new Ui::HistoryForm)
{
    this->RetrievingEdit = false;
    this->ui->setupUi(this);
    this->ui->pushButton->setEnabled(false);
    this->setWindowTitle(_l("historyform-title"));
    this->ui->pushButton->setText(_l("historyform-no-info"));
    this->ui->tableWidget->setColumnCount(6);
    this->SelectedRow = -1;
    this->PreviouslySelectedRow = 2;
    QStringList header;
    header << "" << _l("user")
                 << _l("size")
                 << _l("summary")
                 << _l("id")
                 << _l("date");
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui->tableWidget->setShowGrid(false);
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
        this->ui->tableWidget->setColumnWidth(0, 20);
        this->ui->tableWidget->setColumnWidth(1, 100);
        this->ui->tableWidget->setColumnWidth(2, 60);
        this->ui->tableWidget->setColumnWidth(3, 200);
        this->ui->tableWidget->setColumnWidth(4, 60);
    }
    this->ui->tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

HistoryForm::~HistoryForm()
{
    delete this->t1;
    delete this->ui;
}

void HistoryForm::Read()
{
    //this->ui->pushButton->setText(Localizations::HuggleLocalizations->nullptrze("historyform-retrieving-history"));
    this->ui->pushButton->hide();
    this->query = new ApiQuery(ActionQuery, this->CurrentEdit->GetSite());
    this->query->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("ids|flags|timestamp|user|userid|size|sha1|comment") + "&rvlimit=" +
            QString::number(Huggle::Configuration::HuggleConfiguration->UserConfig->HistoryMax) +
            "&titles=" + QUrl::toPercentEncoding(this->CurrentEdit->Page->PageName);
    this->query->Process();
    delete this->t1;
    this->t1 = new QTimer(this);
    this->Clear();
    connect(t1, SIGNAL(timeout()), this, SLOT(onTick01()));
    this->t1->start(HUGGLE_TIMER);
}

void HistoryForm::Update(WikiEdit *edit)
{
    if (edit == nullptr)
        throw new Huggle::Exception("WikiEdit edit must not be nullptr", "void HistoryForm::Update(WikiEdit *edit)");
    this->CurrentEdit = edit;
    this->ui->pushButton->setText(_l("historyform-retrieve-history"));
    this->ui->pushButton->show();
    this->ui->pushButton->setEnabled(true);
    this->Clear();
    this->RetrievedEdit.Delete();
    this->RetrievingEdit = false;
    if (this->t1 != nullptr)
    {
        this->t1->stop();
        delete this->t1;
        this->t1 = nullptr;
    }
    if (this->query != nullptr)
    {
        this->query = nullptr;
    }
}

void HistoryForm::onTick01()
{
    if (this->RetrievingEdit && this->RetrievedEdit != nullptr)
    {
        if (this->RetrievedEdit->IsPostProcessed())
        {
            MainWindow::HuggleMain->ProcessEdit(this->RetrievedEdit, false, true);
            this->RetrievingEdit = false;
            this->RetrievedEdit = nullptr;
            this->t1->stop();
            this->MakeSelectedRowBold();
        }
        return;
    }

    if (this->query == nullptr || !this->query->IsProcessed())
        return;

    if (this->query->Result->IsFailed())
    {
        this->ui->pushButton->setEnabled(true);
        Huggle::Syslog::HuggleLogs->ErrorLog("Unable to retrieve history");
        this->query = nullptr;
        this->t1->stop();
        return;
    }
    bool IsLatest = false;
    QDomDocument d;
    d.setContent(this->query->Result->Data);
    QDomNodeList l = d.elementsByTagName("rev");
    int x=0;
    QColor xb;
    bool xt = false;
    while (x < l.count())
    {
        if (xt)
        {
            xb = QColor(206, 202, 250);
        } else
        {
            xb = QColor(224, 222, 250);
        }
        xt = !xt;
        QDomElement e = l.at(x).toElement();
        QString RevID;
        if (e.attributes().contains("revid"))
        {
            RevID = e.attribute("revid");
        } else
        {
            x++;
            continue;
        }
        QString user = "<Unknown>";
        if (e.attributes().contains("user"))
        {
            user = e.attribute("user");
        }
        QString size = "<Unknown>";
        if (e.attributes().contains("size"))
        {
            size = e.attribute("size");
        }
        QString date = "<Unknown>";
        if (e.attributes().contains("timestamp"))
        {
            date = e.attribute("timestamp");
        }
        QString summary = "No summary";
        if (e.attributes().contains("comment"))
        {
            if (e.attribute("comment") != "")
            {
                summary = e.attribute("comment");
            }
        }
        this->ui->tableWidget->insertRow(x);
        QIcon icon(":/huggle/pictures/Resources/blob-none.png");
        if (WikiUtil::IsRevert(summary))
            icon = QIcon(":/huggle/pictures/Resources/blob-revert.png");
        else if (WikiUser::IsIPv6(user) || WikiUser::IsIPv4(user))
            icon = QIcon(":/huggle/pictures/Resources/blob-anon.png");
        else if (Configuration::HuggleConfiguration->WhiteList.contains(user))
            icon = QIcon(":/huggle/pictures/Resources/blob-ignored.png");
        WikiUser *wu = WikiUser::RetrieveUser(user);
        if (wu != nullptr)
        {
            if (wu->IsReported)
            {
                icon = QIcon(":/huggle/pictures/Resources/blob-reported.png");
            } else if (wu->WarningLevel > 0)
            {
                switch(wu->WarningLevel)
                {
                    case 1:
                        icon = QIcon(":/huggle/pictures/Resources/blob-warn-1.png");
                        break;
                    case 2:
                        icon = QIcon(":/huggle/pictures/Resources/blob-warn-2.png");
                        break;
                    case 3:
                        icon = QIcon(":/huggle/pictures/Resources/blob-warn-3.png");
                        break;
                    case 4:
                        icon = QIcon(":/huggle/pictures/Resources/blob-warn-4.png");
                        break;
                }
            }
        }
        if (this->CurrentEdit->RevID == RevID.toInt())
        {
            if (x == 0)
            {
                IsLatest = true;
            }
            this->SelectedRow = x;
            QFont font;
            font.setBold(true);
            QTableWidgetItem *i = new QTableWidgetItem(icon, "");
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 0, i);
            i = new QTableWidgetItem(user);
            i->setFont(font);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 1, i);
            i = new QTableWidgetItem(size);
            i->setFont(font);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 2, i);
            i = new QTableWidgetItem(summary);
            i->setFont(font);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 3, i);
            i = new QTableWidgetItem(RevID);
            i->setFont(font);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 4, i);
            i = new QTableWidgetItem(date);
            i->setFont(font);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 5, i);
        } else
        {
            QTableWidgetItem *i = new QTableWidgetItem(icon, "");
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 0, i);
            i = new QTableWidgetItem(user);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 1, i);
            i = new QTableWidgetItem(size);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 2, i);
            i = new QTableWidgetItem(summary);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 3, i);
            i = new QTableWidgetItem(RevID);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 4, i);
            i = new QTableWidgetItem(date);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 5, i);
        }
        x++;
    }
    this->ui->tableWidget->resizeRowsToContents();
    this->query = nullptr;
    this->t1->stop();
    if (!this->CurrentEdit->NewPage && !Configuration::HuggleConfiguration->ForcedNoEditJump && !IsLatest)
    {
        if (Configuration::HuggleConfiguration->UserConfig->LastEdit)
        {
            this->Display(0, Resources::Html_StopFire, true);
        } else
        {
            QPoint pntr(0, this->pos().y());
            if (this->pos().x() > 400)
            {
                pntr.setX(this->pos().x() - 200);
            } else
            {
                pntr.setX(this->pos().x() + 100);
            }
            QToolTip::showText(pntr, "<b><big>" + _l("historyform-not-latest-tip")
                               + "</big></b>", this);
        }
    }
}

void HistoryForm::on_pushButton_clicked()
{
    this->Read();
}

void HistoryForm::on_tableWidget_clicked(const QModelIndex &index)
{
    this->Display(index.row(), _l("wait"));
}

void HistoryForm::Clear()
{
    while (this->ui->tableWidget->rowCount() > 0)
    {
        this->ui->tableWidget->removeRow(0);
    }
    this->SelectedRow = -1;
}

void HistoryForm::Display(int row, QString html, bool turtlemode)
{
    if (row == this->SelectedRow)
    {
        // there is nothing to do because we want to display exactly that row which was already selected
        return;
    }
    if (this->query != nullptr || this->RetrievingEdit || this->ui->tableWidget->rowCount() == 0 || this->CurrentEdit == nullptr)
    {
        // we must not retrieve edit until previous operation did finish
        return;
    }

    this->PreviouslySelectedRow = this->SelectedRow;
    this->SelectedRow = row;
    this->RetrievingEdit = true;
    // check if we don't have this edit in a buffer
    int revid = this->ui->tableWidget->item(row, 4)->text().toInt();
    if (revid == 0)
    {
        this->RetrievingEdit = false;
        return;
    }

    Collectable_SmartPtr<WikiEdit> edit = WikiEdit::FromCacheByRevID(revid);
    if (edit != nullptr)
    {
        MainWindow::HuggleMain->ProcessEdit(edit, false, true);
        this->RetrievingEdit = false;
        this->MakeSelectedRowBold();
        return;
    }

    // there is no such edit, let's get it
    WikiEdit *w = new WikiEdit();
    w->User = new WikiUser(this->ui->tableWidget->item(row, 1)->text());
    w->User->Site = this->CurrentEdit->GetSite();
    w->Page = new WikiPage(this->CurrentEdit->Page);
    w->RevID = revid;
    QueryPool::HugglePool->PreProcessEdit(w);
    QueryPool::HugglePool->PostProcessEdit(w);
    if (this->t1 != nullptr)
    {
        delete this->t1;
    }
    this->RetrievedEdit = w;
    MainWindow::HuggleMain->LockPage();
    MainWindow::HuggleMain->Browser->RenderHtml(html);
    this->t1 = new QTimer();
    connect(this->t1, SIGNAL(timeout()), this, SLOT(onTick01()));
    if (!turtlemode)
    {
        this->t1->start(HUGGLE_TIMER);
        return;
    }
    this->t1->start(2000);
}

void HistoryForm::MakeSelectedRowBold()
{
    QFont font;
    if (this->SelectedRow != -1)
    {
        font.setBold(true);
        this->ui->tableWidget->item(this->SelectedRow, 1)->setFont(font);
        this->ui->tableWidget->item(this->SelectedRow, 2)->setFont(font);
        this->ui->tableWidget->item(this->SelectedRow, 3)->setFont(font);
        this->ui->tableWidget->item(this->SelectedRow, 4)->setFont(font);
        this->ui->tableWidget->item(this->SelectedRow, 5)->setFont(font);
    }
    if (this->PreviouslySelectedRow != -1)
    {
        font.setBold(false);
        this->ui->tableWidget->item(this->PreviouslySelectedRow, 1)->setFont(font);
        this->ui->tableWidget->item(this->PreviouslySelectedRow, 2)->setFont(font);
        this->ui->tableWidget->item(this->PreviouslySelectedRow, 3)->setFont(font);
        this->ui->tableWidget->item(this->PreviouslySelectedRow, 4)->setFont(font);
        this->ui->tableWidget->item(this->PreviouslySelectedRow, 5)->setFont(font);
    }
}
