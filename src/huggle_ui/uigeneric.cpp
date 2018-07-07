//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "uigeneric.hpp"
#include "reportuser.hpp"
#include "mainwindow.hpp"
#include <QMessageBox>
#include <huggle_core/configuration.hpp>
#include <huggle_core/syslog.hpp>
#include <huggle_core/wikisite.hpp>

using namespace Huggle;

int UiGeneric::MessageBox(QString title, QString text, MessageBoxStyle st, bool enforce_stop, QWidget *parent)
{
    QMessageBox *mb = new QMessageBox(parent);
    mb->setWindowTitle(title);
    mb->setText(text);
    int return_value = -1;
    switch (st)
    {
        case MessageBoxStyleQuestion:
            mb->setIcon(QMessageBox::Question);
            mb->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            mb->setDefaultButton(QMessageBox::Yes);
            return_value = mb->exec();
            break;
        case MessageBoxStyleQuestionAbort:
            mb->setIcon(QMessageBox::Question);
            mb->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            mb->setDefaultButton(QMessageBox::Cancel);
            return_value = mb->exec();
            break;
        case MessageBoxStyleNormal:
            mb->setIcon(QMessageBox::Information);
            goto exec;
        case MessageBoxStyleError:
            mb->setIcon(QMessageBox::Critical);
            goto exec;
        case MessageBoxStyleWarning:
            mb->setIcon(QMessageBox::Warning);
            goto exec;
    }
    delete mb;
    return return_value;
    exec:
        if (enforce_stop)
        {
            return_value = mb->exec();
            delete mb;
        }
        else
        {
            mb->setAttribute(Qt::WA_DeleteOnClose, true);
            mb->show();
        }
        return return_value;
}

int UiGeneric::pMessageBox(QWidget *parent, QString title, QString text, MessageBoxStyle st, bool enforce_stop)
{
    return UiGeneric::MessageBox(title, text, st, enforce_stop, parent);
}

void UiGeneric::DisplayContributionBrowser(WikiUser *User, QWidget *parent)
{
    // We are using ReportUser as a contribution browser because we already have all the code for contribs
    // in there, the second parameter in constructors switches between standard report form and just
    // the contribution browser.
    ReportUser *report = new ReportUser(parent, true);
    report->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    report->SetUser(User);
    report->show();
}

void UiGeneric::ProcessURL(QUrl link)
{
    if (link.scheme() == "huggle")
    {
        if (link.host() == "diff")
        {
            QList<QString> elements = link.path().split("/");
            if (elements.count() < 4)
                return;
            QString wiki = elements[1];
            QString type = elements[2];
            if (type == "revid")
            {
                // we need to display a revid on given wiki, let's first get the wiki
                WikiSite *site = nullptr;
                foreach(WikiSite *sp, hcfg->Projects)
                {
                    if (wiki == sp->Name)
                    {
                        site = sp;
                        break;
                    }
                }
                if (site == nullptr)
                {
                    HUGGLE_DEBUG1("There is no such a wiki: " + wiki);
                    return;
                }
                revid_ht id = elements[3].toLongLong();
                MainWindow::HuggleMain->DisplayRevid(id, site);
            }
        }
    }
}
