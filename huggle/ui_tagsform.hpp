/********************************************************************************
** Form generated from reading UI file 'tagsform.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TAGSFORM_H
#define UI_TAGSFORM_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>

QT_BEGIN_NAMESPACE

class Ui_TagsForm
{
public:

    void setupUi(QDialog *TagsForm)
    {
        if (TagsForm->objectName().isEmpty())
            TagsForm->setObjectName(QString::fromUtf8("TagsForm"));
        TagsForm->resize(477, 183);

        retranslateUi(TagsForm);

        QMetaObject::connectSlotsByName(TagsForm);
    } // setupUi

    void retranslateUi(QDialog *TagsForm)
    {
        TagsForm->setWindowTitle(QApplication::translate("TagsForm", "Dialog", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TagsForm: public Ui_TagsForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TAGSFORM_H
