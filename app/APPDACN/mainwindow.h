#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVariant>
#include <QtCore>
#include <QtGui>
#include <QtQuick>
#include <QtPositioning>
#include <QLineEdit>
//#include <inform.h>
class Inform;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void onNewPacket();
    ~MainWindow();

private slots:
    void on_btn_clicked();
    void on_Inform_clicked();

    // nhận data từ Inform
    void updateObserver(double lat, double lon);
    void updateTarget(double lat, double lon);
    void updateDistance(double distance);

private:
    void typeLocation();

    double lat2 = 0.0;
    double lon2 = 0.0;
    double lat1 = 0.0;
    double lon1 = 0.0;
    int packetCount = 0;
    Ui::MainWindow *ui;
    Inform* inform;

signals:
    void setCenterPosition(QVariant, QVariant);
    void setLocationMarking_1(QVariant, QVariant);
    void setLocationMarking_2(QVariant, QVariant);
    void setDistance(QVariant meters);
};

#endif
