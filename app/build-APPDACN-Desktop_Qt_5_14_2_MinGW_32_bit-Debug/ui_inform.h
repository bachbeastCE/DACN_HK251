/********************************************************************************
** Form generated from reading UI file 'inform.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INFORM_H
#define UI_INFORM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>

QT_BEGIN_NAMESPACE

class Ui_Inform
{
public:
    QPushButton *Mapbutton;
    QTableWidget *tableWidget;
    QPushButton *Savebutton;

    void setupUi(QDialog *Inform)
    {
        if (Inform->objectName().isEmpty())
            Inform->setObjectName(QString::fromUtf8("Inform"));
        Inform->resize(1228, 749);
        Mapbutton = new QPushButton(Inform);
        Mapbutton->setObjectName(QString::fromUtf8("Mapbutton"));
        Mapbutton->setGeometry(QRect(10, 10, 93, 28));
        tableWidget = new QTableWidget(Inform);
        if (tableWidget->columnCount() < 4)
            tableWidget->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));
        tableWidget->setGeometry(QRect(10, 51, 1201, 681));
        tableWidget->horizontalHeader()->setDefaultSectionSize(200);
        Savebutton = new QPushButton(Inform);
        Savebutton->setObjectName(QString::fromUtf8("Savebutton"));
        Savebutton->setGeometry(QRect(1120, 10, 93, 28));

        retranslateUi(Inform);

        QMetaObject::connectSlotsByName(Inform);
    } // setupUi

    void retranslateUi(QDialog *Inform)
    {
        Inform->setWindowTitle(QCoreApplication::translate("Inform", "Dialog", nullptr));
        Mapbutton->setText(QCoreApplication::translate("Inform", "Map", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("Inform", "Time", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("Inform", "Observer Coordinate", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("Inform", "Target Coordinate", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidget->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("Inform", "Distance", nullptr));
        Savebutton->setText(QCoreApplication::translate("Inform", "Save Log", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Inform: public Ui_Inform {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INFORM_H
