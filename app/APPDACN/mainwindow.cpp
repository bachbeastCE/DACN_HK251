#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "inform.h"

static const double FIXED_LAT = 11.537543;
static const double FIXED_LON = 106.901618;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    inform = new Inform(this);

    // connect Inform -> MainWindow
    connect(inform, &Inform::sendObserver,
            this, &MainWindow::updateObserver);

    connect(inform, &Inform::sendTarget,
            this, &MainWindow::updateTarget);

    connect(inform, &Inform::sendDistance,
            this, &MainWindow::updateDistance);


    ui->quickWidget_MAPS->setSource(QUrl(QStringLiteral("qrc:/qmlMAPS.qml")));
    ui->quickWidget_MAPS->show();

    auto Obje = ui->quickWidget_MAPS->rootObject();

    connect(this, SIGNAL(setCenterPosition(QVariant,QVariant)),
            Obje, SLOT(setCenterPosition(QVariant,QVariant)));

    connect(this, SIGNAL(setLocationMarking_1(QVariant,QVariant)),
            Obje, SLOT(setLocationMarking_1(QVariant,QVariant)));

    connect(this, SIGNAL(setLocationMarking_2(QVariant,QVariant)),
            Obje, SLOT(setLocationMarking_2(QVariant,QVariant)));

    connect(this, SIGNAL(setDistance(QVariant)),
            Obje, SLOT(setDistance(QVariant)));
    ui->labelBadge->setStyleSheet(
        "background:red;"
        "color:white;"
        "border-radius:10px;"
        "min-width:20px;"
        "min-height:20px;"
        "font-weight:bold;"
    );
    connect(inform, &Inform::newPacket,
            this, &MainWindow::onNewPacket);
    ui->labelBadge->hide();
    emit setCenterPosition(FIXED_LAT, FIXED_LON);
    emit setLocationMarking_1(FIXED_LAT, FIXED_LON);
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

    double distance = p1.distanceTo(p2);

    emit setDistance(distance);

    qDebug() << "Distance (meters):" << distance;
}

void MainWindow::on_Inform_clicked()
{
    packetCount = 0;
    ui->labelBadge->hide();

    hide();
    inform->show();
}


void MainWindow::updateObserver(double lat, double lon)
{
    lat1 = lat;
    lon1 = lon;

    emit setCenterPosition(lat, lon);
    emit setLocationMarking_1(lat, lon);
}

void MainWindow::updateTarget(double lat, double lon)
{
    lat2 = lat;
    lon2 = lon;

    emit setLocationMarking_2(lat, lon);
}

void MainWindow::updateDistance(double distance)
{
    emit setDistance(distance);
}

void MainWindow::onNewPacket()
{
    packetCount++;

    ui->labelBadge->setText(QString::number(packetCount));
    ui->labelBadge->show();
}
