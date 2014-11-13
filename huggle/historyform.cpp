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
#include "apiqueryresult.hpp"
#include "configuration.hpp"
#include "editbar.hpp"
#include "exception.hpp"
#include "mainwindow.hpp"
#include "localization.hpp"
#include "resources.hpp"
#include "huggleweb.hpp"
#include "syslog.hpp"
#include "ui_historyform.h"
#include "querypool.hpp"
#include "wikisite.hpp"
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
    this->Clear();
    delete this->t1;
    delete this->ui;
}

void HistoryForm::Read()
{
    //this->ui->pushButton->setText(Localizations::HuggleLocalizations->nullptrze("historyform-retrieving-history"));
    this->ui->pushButton->hide();
    this->query = new ApiQuery(ActionQuery, this->CurrentEdit->GetSite());
    this->query->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("ids|flags|timestamp|user|userid|size|sha1|comment") + "&rvlimit=" +
            QString::number(hcfg->UserConfig->HistoryMax) + "&titles=" + QUrl::toPercentEncoding(this->CurrentEdit->Page->PageName);
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
        throw new Huggle::NullPointerException("WikiEdit *edit", BOOST_CURRENT_FUNCTION);
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
    if (this->CurrentEdit == nullptr)
        throw new Huggle::NullPointerException("sp CurrentEdit", BOOST_CURRENT_FUNCTION);

    if (this->RetrievingEdit && this->RetrievedEdit != nullptr)
    {
        if (this->RetrievedEdit->IsPostProcessed())
        {
            MainWindow::HuggleMain->ProcessEdit(this->RetrievedEdit, false, true);
            this->RetrievingEdit = false;
            this->RetrievedEdit = nullptr;
            this->t1->stop();
            this->MakeSelectedRowBold();
            MainWindow::HuggleMain->wEditBar->RefreshPage();
        }
        return;
    }

    if (this->query == nullptr || !this->query->IsProcessed())
        return;

    if (this->query->IsFailed())
    {
        this->ui->pushButton->setEnabled(true);
        Huggle::Syslog::HuggleLogs->ErrorLog(_l("history-failure"));
        this->query = nullptr;
        this->t1->stop();
        return;
    }
    bool IsLatest = false;
    QList<ApiQueryResultNode*>revision_data = this->query->GetApiQueryResult()->GetNodes("rev");
    int x=0;
    QColor xb;
    bool xt = false;
    while (x < revision_data.count())
    {
        if (xt)
        {
            xb = QColor(206, 202, 250);
        } else
        {
            xb = QColor(224, 222, 250);
        }
        xt = !xt;
        ApiQueryResultNode *rv = revision_data.at(x);
        WikiPageHistoryItem *item = new WikiPageHistoryItem();
        if (rv->Attributes.contains("revid"))
        {
            item->RevID = rv->GetAttribute("revid");
        } else
        {
            x++;
            continue;
        }
        item->Name = this->CurrentEdit->Page->PageName;
        item->Site = this->CurrentEdit->GetSite();
        if (rv->Attributes.contains("user"))
            item->User = rv->GetAttribute("user");
        if (rv->Attributes.contains("size"))
            item->Size = rv->GetAttribute("size");
        if (rv->Attributes.contains("timestamp"))
            item->Date = rv->GetAttribute("timestamp");
        if (rv->Attributes.contains("comment") && !rv->GetAttribute("comment").isEmpty())
            item->Summary = rv->GetAttribute("comment");
        this->ui->tableWidget->insertRow(x);
        QIcon icon(":/huggle/pictures/Resources/blob-none.png");
        if (WikiUtil::IsRevert(item->Summary))
        {
            item->Type = EditType_Revert;
            icon = QIcon(":/huggle/pictures/Resources/blob-revert.png");
        }
        else if (WikiUser::IsIPv6(item->User) || WikiUser::IsIPv4(item->User))
        {
            item->Type = EditType_Anon;
            icon = QIcon(":/huggle/pictures/Resources/blob-anon.png");
        }
        else if (this->CurrentEdit->GetSite()->GetProjectConfig()->WhiteList.contains(item->User))
        {
            item->Type = EditType_W;
            icon = QIcon(":/huggle/pictures/Resources/blob-ignored.png");
        }
        WikiUser *wu = WikiUser::RetrieveUser(item->User, item->Site);
        if (wu != nullptr)
        {
            if (wu->IsReported)
            {
                item->Type = EditType_Reported;
                icon = QIcon(":/huggle/pictures/Resources/blob-reported.png");
            } else if (wu->GetWarningLevel() > 0)
            {
                switch(wu->GetWarningLevel())
                {
                    case 1:
                        item->Type = EditType_1;
                        icon = QIcon(":/huggle/pictures/Resources/blob-warn-1.png");
                        break;
                    case 2:
                        item->Type = EditType_2;
                        icon = QIcon(":/huggle/pictures/Resources/blob-warn-2.png");
                        break;
                    case 3:
                        item->Type = EditType_3;
                        icon = QIcon(":/huggle/pictures/Resources/blob-warn-3.png");
                        break;
                    case 4:
                        item->Type = EditType_4;
                        icon = QIcon(":/huggle/pictures/Resources/blob-warn-4.png");
                        break;
                }
            }
        }
        if (this->CurrentEdit->RevID == item->RevID.toInt())
        {
            if (x == 0)
                IsLatest = true;
            item->IsCurrent = true;
            this->SelectedRow = x;
            QFont font;
            font.setBold(true);
            QTableWidgetItem *i = new QTableWidgetItem(icon, "");
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 0, i);
            i = new QTableWidgetItem(item->User);
            i->setFont(font);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 1, i);
            i = new QTableWidgetItem(item->Size);
            i->setFont(font);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 2, i);
            i = new QTableWidgetItem(item->Summary);
            i->setFont(font);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 3, i);
            i = new QTableWidgetItem(item->RevID);
            i->setFont(font);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 4, i);
            i = new QTableWidgetItem(item->Date);
            i->setFont(font);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 5, i);
        } else
        {
            QTableWidgetItem *i = new QTableWidgetItem(icon, "");
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 0, i);
            i = new QTableWidgetItem(item->User);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 1, i);
            i = new QTableWidgetItem(item->Size);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 2, i);
            i = new QTableWidgetItem(item->Summary);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 3, i);
            i = new QTableWidgetItem(item->RevID);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 4, i);
            i = new QTableWidgetItem(item->Date);
            i->setBackgroundColor(xb);
            this->ui->tableWidget->setItem(x, 5, i);
        }
        this->Items.append(item);
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
                pntr.setX(this->pos().x() - 200);
            else
                pntr.setX(this->pos().x() + 100);

            // display a tip
            QToolTip::showText(pntr, "<b><big>" + _l("historyform-not-latest-tip") + "</big></b>", this);
        }
    }
    MainWindow::HuggleMain->wEditBar->RefreshPage();
}

void HistoryForm::on_pushButton_clicked()
{
    this->Read();
}

void HistoryForm::Clear()
{
    while (this->Items.count())
    {
        delete this->Items.at(0);
        this->Items.removeAt(0);
    }
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

    int revid = this->ui->tableWidget->item(row, 4)->text().toInt();
    if (revid == 0)
        return;

    this->GetEdit(revid, "prev", row, html, turtlemode);
}

void HistoryForm::GetEdit(long revid, QString prev, int row, QString html, bool turtlemode)
{
    this->PreviouslySelectedRow = this->SelectedRow;
    this->SelectedRow = row;
    this->GetEdit(revid, prev, this->ui->tableWidget->item(row, 1)->text(), html, turtlemode);
}

void HistoryForm::GetEdit(long revid, QString prev, QString user, QString html, bool turtlemode)
{
    this->RetrievingEdit = true;
    Collectable_SmartPtr<WikiEdit> edit = WikiEdit::FromCacheByRevID(revid, prev);
    if (edit != nullptr)
    {
        MainWindow::HuggleMain->ProcessEdit(edit, false, true);
        this->RetrievingEdit = false;
        this->MakeSelectedRowBold();
        MainWindow::HuggleMain->wEditBar->RefreshPage();
        return;
    }
    // there is no such edit, let's get it
    WikiEdit *w = new WikiEdit();
    w->DiffTo = prev;
    w->User = new WikiUser(user);
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
        if (this->Items.count() > this->SelectedRow)
            this->Items[this->SelectedRow]->IsCurrent = true;
        this->ui->tableWidget->item(this->SelectedRow, 1)->setFont(font);
        this->ui->tableWidget->item(this->SelectedRow, 2)->setFont(font);
        this->ui->tableWidget->item(this->SelectedRow, 3)->setFont(font);
        this->ui->tableWidget->item(this->SelectedRow, 4)->setFont(font);
        this->ui->tableWidget->item(this->SelectedRow, 5)->setFont(font);
    }
    if (this->PreviouslySelectedRow != -1)
    {
        if (this->Items.count() > this->PreviouslySelectedRow)
            this->Items[this->PreviouslySelectedRow]->IsCurrent = false;
        font.setBold(false);
        this->ui->tableWidget->item(this->PreviouslySelectedRow, 1)->setFont(font);
        this->ui->tableWidget->item(this->PreviouslySelectedRow, 2)->setFont(font);
        this->ui->tableWidget->item(this->PreviouslySelectedRow, 3)->setFont(font);
        this->ui->tableWidget->item(this->PreviouslySelectedRow, 4)->setFont(font);
        this->ui->tableWidget->item(this->PreviouslySelectedRow, 5)->setFont(font);
    }
}

void Huggle::HistoryForm::on_tableWidget_itemSelectionChanged()
{
    // check if user selected a range
    QItemSelection selection(this->ui->tableWidget->selectionModel()->selection());
    QList<int> rows;
    foreach(const QModelIndex & index, selection.indexes())
       rows.append( index.row() );
    if (rows.count() == 1)
    {
        this->Display(rows[0], _l("wait"));
    } else if (rows.count() > 1)
    {
        int max = this->ui->tableWidget->item(rows[0], 4)->text().toInt();
        QString min = this->ui->tableWidget->item(rows[rows.count()-1], 4)->text();
        if (!max)
            return;

        // display a range of edits
        if (this->query != nullptr || this->RetrievingEdit || this->CurrentEdit == nullptr)
        {
            // we must not retrieve edit until previous operation did finish
            return;
        }
        this->GetEdit(max, min, rows[0], _l("wait"));
    }
}
