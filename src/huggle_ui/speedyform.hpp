//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef SPEEDYFORM_H
#define SPEEDYFORM_H

#include <huggle_core/definitions.hpp>

#include <QTimer>
#include "hw.hpp"
#include <huggle_core/apiquery.hpp>
#include <huggle_core/collectable_smartptr.hpp>
#include <huggle_core/editquery.hpp>
#include <huggle_core/message.hpp>
#include <huggle_core/wikiedit.hpp>

class QCloseEvent;

namespace Ui
{
    class SpeedyForm;
}

namespace Huggle
{
    class WikiEdit;
    class WikiUser;
    class Query;

    /*!
     * \brief The window that is used to report a page for deletion
     *
     * This is a feature that originated on English wikipedia and is designed for users who need to delete a page, but don't
     * have the permissions to do that.
     */
    class HUGGLE_EX_UI SpeedyForm : public HW
    {
            Q_OBJECT
        public:
            explicit SpeedyForm(QWidget *parent = nullptr);
            ~SpeedyForm() override;
            void Init(WikiEdit *edit_);
            QString GetSelectedDBReason();
            QString GetSelectedTagID();
            void SetMessageUserCheck(bool new_value);
            bool IsBusy() const;
            bool ReplacePage = false;
            QString ReplacingText;
            Collectable_SmartPtr<WikiEdit> edit;
            QString Text;
            QString Header;

        private slots:
            void OnTick();
            void on_btnCancel_clicked();
            void on_btnTag_clicked();
            void on_cbReason_currentIndexChanged(int index);

        private:
            static void *ContentSuccess(Query *query);
            static void *ContentFailure(Query *query);
            static void *EditSuccess(Query *query);
            static void *EditFailure(Query *query);
            static void *FounderSuccess(Query *query);
            static void *FounderFailure(Query *query);
            static SpeedyForm *ReleaseCallback(Query *query);
            static void DetachCallback(Query *query);
            void closeEvent(QCloseEvent *event) override;
            void Fail(const QString &reason, bool pageTagged = false);
            void Finish();
            void processTags();
            void tagSucceeded();
            void retrieveFounder();
            void notifyFounder(const QString &founder);
            Collectable_SmartPtr<EditQuery> Template;
            Collectable_SmartPtr<ApiQuery> qObtainText;
            Collectable_SmartPtr<ApiQuery> qFounder;
            Collectable_SmartPtr<Message> message;
            QString base;
            QString warning;
            QTimer *timer;
            Ui::SpeedyForm *ui;
            bool busy = false;
    };
}
#endif // SPEEDYFORM_H
