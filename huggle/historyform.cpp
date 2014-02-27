//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "historyform.hpp"
#include "ui_historyform.h"

using namespace Huggle;

HistoryForm::HistoryForm(QWidget *parent) : QDockWidget(parent), ui(new Ui::HistoryForm)
{
    this->RetrievedEdit = NULL;
    this->RetrievingEdit = false;
    this->ui->setupUi(this);
    this->ui->pushButton->setEnabled(false);
    this->setWindowTitle(Localizations::HuggleLocalizations->Localize("History"));
    this->ui->pushButton->setText(Localizations::HuggleLocalizations->Localize("historyform-no-info"));
    this->ui->tableWidget->setColumnCount(6);
    this->SelectedRow = 0;
    this->PreviouslySelectedRow = 2;
    QStringList header;
    header << "" << Huggle::Localizations::HuggleLocalizations->Localize("user") <<
              Huggle::Localizations::HuggleLocalizations->Localize("size") <<
              Huggle::Localizations::HuggleLocalizations->Localize("summary") <<
              Huggle::Localizations::HuggleLocalizations->Localize("id") <<
              Huggle::Localizations::HuggleLocalizations->Localize("date");
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
#if QT_VERSION >= 0x050000
// Qt5 code
    this->ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    this->ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    this->ui->tableWidget->setShowGrid(false);
    this->query = NULL;
    this->t1 = NULL;
}

HistoryForm::~HistoryForm()
{
    if (this->RetrievedEdit != NULL)
    {
        this->RetrievedEdit->UnregisterConsumer(HUGGLECONSUMER_HISTORYWIDGET);
        this->RetrievedEdit = NULL;
    }
    if (this->query != NULL)
    {
        this->query->UnregisterConsumer(HUGGLECONSUMER_HISTORYWIDGET);
        this->query = NULL;
    }
    delete this->t1;
    delete this->ui;
}

void HistoryForm::Read()
{
    //this->ui->pushButton->setText(Localizations::HuggleLocalizations->Localize("historyform-retrieving-history"));
    this->ui->pushButton->hide();
    this->query = new ApiQuery();
    this->query->SetAction(ActionQuery);
    this->query->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("ids|flags|timestamp|user|userid|size|sha1|comment") + "&rvlimit=" +
            QString::number(Huggle::Configuration::HuggleConfiguration->UserConfig_HistoryMax) +
            "&titles=" + QUrl::toPercentEncoding(this->CurrentEdit->Page->PageName);
    this->query->RegisterConsumer(HUGGLECONSUMER_HISTORYWIDGET);
    this->query->Process();
    if (this->t1 != NULL)
    {
        delete this->t1;
    }
    this->t1 = new QTimer(this);
    this->Clear();
    connect(t1, SIGNAL(timeout()), this, SLOT(onTick01()));
    this->t1->start(200);
}

void HistoryForm::Update(WikiEdit *edit)
{
    if (edit == NULL)
    {
        throw new Exception("WikiEdit edit must not be NULL", "void HistoryForm::Update(WikiEdit *edit)");
    }
    this->CurrentEdit = edit;
    this->ui->pushButton->setText(Localizations::HuggleLocalizations->Localize("historyform-retrieve-history"));
    this->ui->pushButton->show();
    this->ui->pushButton->setEnabled(true);
    this->ui->tableWidget->clearContents();
    this->Clear();
    if (this->RetrievedEdit != NULL)
    {
        this->RetrievedEdit->UnregisterConsumer(HUGGLECONSUMER_HISTORYWIDGET);
        this->RetrievedEdit = NULL;
    }
    this->RetrievingEdit = false;
    if (this->t1 != NULL)
    {
        this->t1->stop();
        delete this->t1;
        this->t1 = NULL;
    }
    if (this->query != NULL)
    {
        this->query->UnregisterConsumer(HUGGLECONSUMER_HISTORYWIDGET);
        this->query = NULL;
    }
}

void HistoryForm::onTick01()
{
    if (this->RetrievingEdit && this->RetrievedEdit != NULL)
    {
        if (this->RetrievedEdit->IsPostProcessed())
        {
            Core::HuggleCore->Main->ProcessEdit(this->RetrievedEdit, false, true);
            this->RetrievingEdit = false;
            this->RetrievedEdit->UnregisterConsumer(HUGGLECONSUMER_HISTORYWIDGET);
            this->RetrievedEdit = NULL;
            this->t1->stop();
            this->MakeSelectedRowBold();
        }
        return;
    }
    if (this->query == NULL)
    {
        return;
    }
    if (!this->query->IsProcessed())
    {
        return;
    }
    if (this->query->Result->Failed)
    {
        this->ui->pushButton->setEnabled(true);
        Huggle::Syslog::HuggleLogs->ErrorLog("Unable to retrieve history");
        this->query->UnregisterConsumer(HUGGLECONSUMER_HISTORYWIDGET);
        this->query = NULL;
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

        if (Core::HuggleCore->IsRevert(summary))
        {
            icon = QIcon(":/huggle/pictures/Resources/blob-revert.png");
        } else if (WikiUser::IsIPv6(user) || WikiUser::IsIPv4(user))
        {
            icon = QIcon(":/huggle/pictures/Resources/blob-anon.png");
        } else if (Configuration::HuggleConfiguration->WhiteList.contains(user))
        {
            icon = QIcon(":/huggle/pictures/Resources/blob-ignored.png");
        }

        WikiUser *wu = WikiUser::RetrieveUser(user);
        if (wu != NULL)
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
    this->query->UnregisterConsumer(HUGGLECONSUMER_HISTORYWIDGET);
    this->ui->tableWidget->resizeRowsToContents();
    this->query = NULL;
    this->t1->stop();
    if (!IsLatest)
    {
        if (Configuration::HuggleConfiguration->UserConfig_LastEdit)
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
            QToolTip::showText(pntr, "<b><big>" +Localizations::HuggleLocalizations->Localize("historyform-not-latest-tip")
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
    this->Display(index.row(), Huggle::Localizations::HuggleLocalizations->Localize("wait"));
}

void HistoryForm::Clear()
{
    while (this->ui->tableWidget->rowCount() > 0)
    {
        this->ui->tableWidget->removeRow(0);
    }
}

void HistoryForm::Display(int row, QString html, bool turtlemode)
{
    if (this->query != NULL || this->RetrievingEdit)
    {
        // we must not retrieve edit until previous operation did finish
        return;
    }

    if (this->ui->tableWidget->rowCount() == 0 || this->CurrentEdit == NULL)
    {
        return;
    }

    this->PreviouslySelectedRow = this->SelectedRow;
    this->SelectedRow = row;
    this->RetrievingEdit = true;
    // check if we don't have this edit in a buffer
    int x = 0;
    int revid = this->ui->tableWidget->item(row, 4)->text().toInt();
    if (revid == 0)
    {
        this->RetrievingEdit = false;
        return;
    }
    WikiEdit::Lock_EditList->lock();
    while (x < WikiEdit::EditList.count())
    {
        WikiEdit *edit = WikiEdit::EditList.at(x);
        x++;
        if (edit->RevID == revid)
        {
            Core::HuggleCore->Main->ProcessEdit(edit, true, true);
            this->RetrievingEdit = false;
            WikiEdit::Lock_EditList->unlock();
            this->MakeSelectedRowBold();
            return;
        }
    }
    WikiEdit::Lock_EditList->unlock();
    // there is no such edit, let's get it
    WikiEdit *w = new WikiEdit();
    w->User = new WikiUser(this->ui->tableWidget->item(row, 1)->text());
    w->Page = new WikiPage(this->CurrentEdit->Page);
    w->RevID = revid;
    w->RegisterConsumer(HUGGLECONSUMER_HISTORYWIDGET);
    if (this->RetrievedEdit != NULL)
    {
        this->RetrievedEdit->UnregisterConsumer(HUGGLECONSUMER_HISTORYWIDGET);
    }
    Core::HuggleCore->PostProcessEdit(w);
    if (this->t1 != NULL)
    {
        delete this->t1;
    }
    this->RetrievedEdit = w;
    Core::HuggleCore->Main->LockPage();
    Core::HuggleCore->Main->Browser->RenderHtml(html);
    this->t1 = new QTimer();
    connect(this->t1, SIGNAL(timeout()), this, SLOT(onTick01()));
    if (!turtlemode)
    {
        this->t1->start(200);
        return;
    }
    this->t1->start(2000);
}

void HistoryForm::MakeSelectedRowBold()
{
    QFont font;
    font.setBold(true);
    this->ui->tableWidget->item(this->SelectedRow, 1)->setFont(font);
    this->ui->tableWidget->item(this->SelectedRow, 2)->setFont(font);
    this->ui->tableWidget->item(this->SelectedRow, 3)->setFont(font);
    this->ui->tableWidget->item(this->SelectedRow, 4)->setFont(font);
    this->ui->tableWidget->item(this->SelectedRow, 5)->setFont(font);
    font.setBold(false);
    this->ui->tableWidget->item(this->PreviouslySelectedRow, 1)->setFont(font);
    this->ui->tableWidget->item(this->PreviouslySelectedRow, 2)->setFont(font);
    this->ui->tableWidget->item(this->PreviouslySelectedRow, 3)->setFont(font);
    this->ui->tableWidget->item(this->PreviouslySelectedRow, 4)->setFont(font);
    this->ui->tableWidget->item(this->PreviouslySelectedRow, 5)->setFont(font);
}
