//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <huggle_core/definitions.hpp>

#include <huggle_core/iextension.hpp>
#include <huggle_core/hugglequeuefilter.hpp>
#include "hw.hpp"
#include <QList>
#include <QHash>

class QCheckBox;

namespace Ui
{
    class Preferences;
}

namespace Huggle
{
    class HuggleQueueFilter;
    class WikiSite;
    //! Preferences window
    class HUGGLE_EX_UI Preferences : public HW
    {
        Q_OBJECT
        public:
            explicit Preferences(QWidget *parent = nullptr);
            ~Preferences();
            void EnableQueues(bool enabled);

        private slots:
            void on_listWidget_itemSelectionChanged();
            void on_checkBox_clicked();
            void on_checkBox_26_clicked();
            void on_checkBox_27_clicked();
            void RecordKeys(int row, int column);
            void on_pushButton_7_clicked();
            void on_cbSites_currentIndexChanged(int index);
            void on_cbDefault_currentIndexChanged(int index);
            void on_tableWidget_customContextMenuRequested(const QPoint &pos);
            void on_pushButton_rs_clicked();
            void on_cbqBots_currentIndexChanged(int index);
            void on_cbqIP_currentIndexChanged(int index);
            void on_cbqOwn_currentIndexChanged(int index);
            void on_cbqRevert_currentIndexChanged(int index);
            void on_cbqNew_currentIndexChanged(int index);
            void on_cbqMinor_currentIndexChanged(int index);
            void on_cbqWl_currentIndexChanged(int index);
            void on_cbqFrd_currentIndexChanged(int index);
            void on_cbqUserspace_currentIndexChanged(int index);
            void on_cbqTp_currentIndexChanged(int index);
            void on_cbqWatched_currentIndexChanged(int index);
            void on_leIgnoredTags_textEdited(const QString &arg1);
            void on_leIgnoredCategories_textEdited(const QString &arg1);
            void on_leRequiredTags_textEdited(const QString &arg1);
            void on_leRequiredCategories_textEdited(const QString &arg1);
            void on_pushButton_OK_clicked();
            void on_pushButton_CloseWin_clicked();
            void on_pushButton_QueueSave_clicked();
            void on_pushButton_QueueDelete_clicked();
            void on_pushButton_QueueInsert_clicked();
            void on_pushButton_QueueReset_clicked();

        private:
            void ResetItems();
            void Reload();
            //! Used to reload shortcuts only
            void Reload2();
            QHash<QCheckBox*, int> NamespaceBoxes;
            WikiSite *Site;
            bool IgnoreConflicts = false;
            bool isNowReloadingFilters = false;
            bool RewritingForm = false;
            bool ModifiedForm = false;
            int queueID = 0;
            bool queueModified = false;
            Ui::Preferences *ui;
    };
}

#endif // PREFERENCES_H
