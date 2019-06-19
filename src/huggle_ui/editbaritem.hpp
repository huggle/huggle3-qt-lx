//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef EDITBARITEM_HPP
#define EDITBARITEM_HPP

#include <huggle_core/definitions.hpp>
#include <QFrame>

namespace Ui
{
    class EditBarItem;
}

namespace Huggle
{
  /*!
     * \brief The EditBarItem class is used for being an item of an EditBar
     * It gives general information about the edit of a page
     */
    class HUGGLE_EX_UI EditBarItem : public QFrame
    {
            Q_OBJECT
        public:
            /*!
             * \brief EditBarItem Creates a new instance of the EditBarItem class
             * \param parent The parent widget in which the EditBar is contained
             */
            explicit EditBarItem(QWidget *parent = nullptr);
            ~EditBarItem() override;
            /*!
             * \brief SetLineWidth Sets the width of the textlines
             * \param width The width of the textlines
             */
            void SetLineWidth(int width);
            /*!
             * \brief SetText Sets the whole text of this entry
             * \param text The text the entrz is to be set to
             */
            void SetText(const QString& text);
            /*!
             * \brief SetFrame Sets the colour of the items borders
             * \param colour The new colour of the items borders
             */
            void SetFrame(Qt::GlobalColor colour);
            /*!
             * \brief SetPixmap Sets the background of the item to a pixmap
             * \param path The location where the pixmap is stored
             */
            void SetPixmap(const QString& path);
            bool IsUser = true; ///< True if edit was made by current user
            QString Page; ///< The edited page
            QString RevID; ///< The reference ID of this edit
            int RowId = -1; ///< The row ID of this edit
            QString Username; ///< The username of the editing user

        private:
            Ui::EditBarItem *ui;

        protected:
            void mousePressEvent(QMouseEvent *event) override;
    };
}

#endif // EDITBARITEM_HPP
