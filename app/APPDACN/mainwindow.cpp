#include "mainwindow.h"
#include "ui_mainwindow.h"

static const double FIXED_LAT = 11.537543;
static const double FIXED_LON = 106.901618;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->quickWidget_MAPS->setSource(QUrl(QStringLiteral("qrc:/qmlMAPS.qml")));
    ui->quickWidget_MAPS->show();

    auto Obje = ui->quickWidget_MAPS->rootObject();
    connect(this, SIGNAL(setCenterPosition(QVariant,QVariant)), Obje, SLOT(setCenterPosition(QVariant,QVariant)));
    connect(this, SIGNAL(setLocationMarking_1(QVariant,QVariant)), Obje, SLOT(setLocationMarking_1(QVariant,QVariant)));
    connect(this, SIGNAL(setLocationMarking_2(QVariant,QVariant)), Obje, SLOT(setLocationMarking_2(QVariant,QVariant)));
    connect(this, SIGNAL(setDistance(QVariant)), Obje, SLOT(setDistance(QVariant)));
    emit setCenterPosition(FIXED_LAT, FIXED_LON);
    emit setLocationMarking_1(FIXED_LAT, FIXED_LON);

    serial = new QSerialPort(this);
    serial->setPortName("COM4");
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if (serial->open(QIODevice::ReadOnly)) {
        connect(serial, &QSerialPort::readyRead,
                this, &MainWindow::readSerialData);
        qDebug() << "COM4 opened";
    } else {
        qDebug() << "Failed to open COM4";
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::typeLocation()
{
    lat2 = ui->lineEdit->text().toDouble();
    lon2 = ui->lineEdit_2->text().toDouble();
}

void MainWindow::on_btn_clicked()
{
    if (ui->lineEdit->text().isEmpty() || ui->lineEdit_2->text().isEmpty())
        return;

    typeLocation();

    emit setLocationMarking_2(lat2, lon2);

    QGeoCoordinate p1(FIXED_LAT, FIXED_LON);
    QGeoCoordinate p2(lat2, lon2);
    double distance = p1.distanceTo(p2); // meters
    emit setDistance(distance);


    qDebug() << "Distance (meters):" << p1.distanceTo(p2);
}

void MainWindow::readSerialData()
{
    while (serial->canReadLine()) {
        QString line = QString::fromUtf8(serial->readLine()).trimmed();

        // OBSERVER
        if (line.startsWith("Observer Coordinate")) {
            QStringList vals = line.split("=").last().split(",");
            if (vals.size() == 2) {
                lat1 = vals[0].trimmed().toDouble();
                lon1 = vals[1].trimmed().toDouble();

                emit setCenterPosition(lat1, lon1);
                emit setLocationMarking_1(lat1, lon1);

                qDebug() << "OBS:" << lat1 << lon1;
            }
        }

        // TARGET
        else if (line.startsWith("Target")) {
            QStringList vals = line.split("=").last().split(",");
            if (vals.size() == 2) {
                lat2 = vals[0].trimmed().toDouble();
                lon2 = vals[1].trimmed().toDouble();

                emit setLocationMarking_2(lat2, lon2);

                qDebug() << "TAR:" << lat2 << lon2;
            }
        }

        // DISTANCE
        if (lat1 != 0 && lat2 != 0) {
            QGeoCoordinate p1(lat1, lon1);
            QGeoCoordinate p2(lat2, lon2);
            emit setDistance(p1.distanceTo(p2));
        }
    }
}
