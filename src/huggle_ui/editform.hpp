//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef EDITFORM_HPP
#define EDITFORM_HPP

#include <huggle_core/definitions.hpp>
#include <huggle_core/collectable_smartptr.hpp>
#include <QDialog>
#include "hw.hpp"

namespace Ui
{
    class EditForm;
}

namespace Huggle
{
    class ApiQuery;
    class EditQuery;
    class HuggleWeb;
    class WikiPage;

    class HUGGLE_EX_UI EditForm : public HW
    {
            Q_OBJECT

        public:
            explicit EditForm(WikiPage *wp, QWidget *parent = nullptr);
            ~EditForm() override;
            void Failure(const QString& reason);
            void RenderSource(const QString& code, const QString &time);
            void DisplayPreview(QString html);
            void FinishEdit();
            void FailEdit(const QString& reason);

        private slots:
            void on_saveButton_clicked();
            void on_previewButton_clicked();

        private:
            void renderText(const QString& text);
            Collectable_SmartPtr<ApiQuery> parseQuery;
            Collectable_SmartPtr<EditQuery> editQuery;
            Collectable_SmartPtr<ApiQuery> contentQuery;
            WikiPage *page;
            QString pageTime;
            HuggleWeb *webView;
            Ui::EditForm *ui;
    };
}

#endif // EDITFORM_HPP
