/********************************************************************************
** Form generated from reading UI file 'reportuserc27129.ui'
**
** Created: Thu Sep 26 18:08:27 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef REPORTUSERC27129_H
#define REPORTUSERC27129_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtWebKit/QWebView>

QT_BEGIN_NAMESPACE

class Ui_ReportUser
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QLabel *label_3;
    QTableWidget *tableWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QLineEdit *lineEdit;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushButton;
    QPushButton *pushButton_3;
    QPushButton *pushButton_2;
    QWebView *webView;

    void setupUi(QDialog *ReportUser)
    {
        if (ReportUser->objectName().isEmpty())
            ReportUser->setObjectName(QString::fromUtf8("ReportUser"));
        ReportUser->resize(1113, 644);
        verticalLayout = new QVBoxLayout(ReportUser);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(ReportUser);
        label->setObjectName(QString::fromUtf8("label"));
        label->setMinimumSize(QSize(0, 20));

        verticalLayout->addWidget(label);

        label_3 = new QLabel(ReportUser);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        verticalLayout->addWidget(label_3);

        tableWidget = new QTableWidget(ReportUser);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));
        tableWidget->setMinimumSize(QSize(0, 200));

        verticalLayout->addWidget(tableWidget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(ReportUser);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        lineEdit = new QLineEdit(ReportUser);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

        horizontalLayout->addWidget(lineEdit);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        pushButton = new QPushButton(ReportUser);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout_2->addWidget(pushButton);

        pushButton_3 = new QPushButton(ReportUser);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));

        horizontalLayout_2->addWidget(pushButton_3);

        pushButton_2 = new QPushButton(ReportUser);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));

        horizontalLayout_2->addWidget(pushButton_2);


        verticalLayout->addLayout(horizontalLayout_2);

        webView = new QWebView(ReportUser);
        webView->setObjectName(QString::fromUtf8("webView"));
        webView->setProperty("url", QVariant(QUrl(QString::fromUtf8("about:blank"))));

        verticalLayout->addWidget(webView);


        retranslateUi(ReportUser);

        QMetaObject::connectSlotsByName(ReportUser);
    } // setupUi

    void retranslateUi(QDialog *ReportUser)
    {
        ReportUser->setWindowTitle(QApplication::translate("ReportUser", "Report", 0, QApplication::UnicodeUTF8));
        label->setText(QString());
        label_3->setText(QApplication::translate("ReportUser", "You want to report this user for following edits:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("ReportUser", "Information", 0, QApplication::UnicodeUTF8));
        lineEdit->setText(QApplication::translate("ReportUser", "Persistant vandalism and/or other unconstructive edits, found using [[WP:HG|Huggle 3]].", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("ReportUser", "Report", 0, QApplication::UnicodeUTF8));
        pushButton_3->setText(QApplication::translate("ReportUser", "Check if user is reported", 0, QApplication::UnicodeUTF8));
        pushButton_2->setText(QApplication::translate("ReportUser", "Talk page", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ReportUser: public Ui_ReportUser {};
} // namespace Ui

QT_END_NAMESPACE

#endif // REPORTUSERC27129_H
