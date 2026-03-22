#ifndef INFORM_H
#define INFORM_H

#include <QDialog>
#include <QSerialPort>
#include <QDateTime>
#include <QTableWidgetItem>
#include <mainwindow.h>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
namespace Ui {
class Inform;
}

class Inform : public QDialog
{
    Q_OBJECT

public:
    explicit Inform(QWidget *parent = nullptr);
    ~Inform();

    void addRow(QString time,
                        QString observer,
                        double observerAlt,
                        QString target,
                        double targetAlt,
                        double distance);

signals:
    void sendObserver(double lat, double lon);
    void sendTarget(double lat, double lon);
    void sendDistance(double distance);
    void newPacket();

private slots:
    void readSerialData();
    void on_Mapbutton_clicked();
    void on_Savebutton_clicked();
    void onRowDoubleClicked(int row, int column);

private:
    Ui::Inform *ui;

    QSerialPort *serial;
    QNetworkAccessManager *networkManager;
    void uploadFirebase(double lat1,double lon1,double alt1,
                        double lat2,double lon2,double alt2,
                        double distance);
    void loadFromFirebase();
    double lat1 = 0;
    double lon1 = 0;
    double lat2 = 0;
    double lon2 = 0;
    double alt1 = 0;
    double alt2 = 0;
};

#endif
