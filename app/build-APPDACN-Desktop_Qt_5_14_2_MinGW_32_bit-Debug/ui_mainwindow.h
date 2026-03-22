/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtQuickWidgets/QQuickWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QQuickWidget *quickWidget_MAPS;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout_2;
    QLabel *labelLat;
    QLineEdit *lineEdit_2;
    QWidget *layoutWidget1;
    QHBoxLayout *horizontalLayout;
    QLabel *labelLon;
    QLineEdit *lineEdit;
    QPushButton *btn;
    QPushButton *Inform;
    QLabel *labelBadge;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1267, 736);
        MainWindow->setUnifiedTitleAndToolBarOnMac(false);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        quickWidget_MAPS = new QQuickWidget(centralwidget);
        quickWidget_MAPS->setObjectName(QString::fromUtf8("quickWidget_MAPS"));
        quickWidget_MAPS->setGeometry(QRect(10, 40, 1251, 641));
        QFont font;
        font.setFamily(QString::fromUtf8("MS Shell Dlg 2"));
        quickWidget_MAPS->setFont(font);
        quickWidget_MAPS->setResizeMode(QQuickWidget::SizeRootObjectToView);
        layoutWidget = new QWidget(centralwidget);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(250, 10, 223, 24));
        horizontalLayout_2 = new QHBoxLayout(layoutWidget);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        labelLat = new QLabel(layoutWidget);
        labelLat->setObjectName(QString::fromUtf8("labelLat"));

        horizontalLayout_2->addWidget(labelLat);

        lineEdit_2 = new QLineEdit(layoutWidget);
        lineEdit_2->setObjectName(QString::fromUtf8("lineEdit_2"));

        horizontalLayout_2->addWidget(lineEdit_2);

        layoutWidget1 = new QWidget(centralwidget);
        layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(10, 10, 233, 24));
        horizontalLayout = new QHBoxLayout(layoutWidget1);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        labelLon = new QLabel(layoutWidget1);
        labelLon->setObjectName(QString::fromUtf8("labelLon"));

        horizontalLayout->addWidget(labelLon);

        lineEdit = new QLineEdit(layoutWidget1);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

        horizontalLayout->addWidget(lineEdit);

        btn = new QPushButton(centralwidget);
        btn->setObjectName(QString::fromUtf8("btn"));
        btn->setGeometry(QRect(510, 10, 93, 28));
        Inform = new QPushButton(centralwidget);
        Inform->setObjectName(QString::fromUtf8("Inform"));
        Inform->setGeometry(QRect(1090, 10, 93, 28));
        labelBadge = new QLabel(centralwidget);
        labelBadge->setObjectName(QString::fromUtf8("labelBadge"));
        labelBadge->setGeometry(QRect(1180, 0, 16, 20));
        labelBadge->setAlignment(Qt::AlignCenter);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1267, 26));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        labelLat->setText(QCoreApplication::translate("MainWindow", "Input Latitude", nullptr));
        labelLon->setText(QCoreApplication::translate("MainWindow", "Input Longitude", nullptr));
        lineEdit->setText(QString());
        btn->setText(QCoreApplication::translate("MainWindow", "Find", nullptr));
        Inform->setText(QCoreApplication::translate("MainWindow", "InformPage", nullptr));
        labelBadge->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
