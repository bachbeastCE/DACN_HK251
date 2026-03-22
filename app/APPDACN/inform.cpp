#include "inform.h"
#include "ui_inform.h"

Inform::Inform(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Inform)
{
    ui->setupUi(this);

    ui->tableWidget->setColumnCount(6);

    QStringList headers;
    headers << "Time"
            << "Observer Coordinate"
            << "Observer Altitude"
            << "Target Coordinate"
            << "Target Altitude"
            << "Distance";

    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    serial = new QSerialPort(this);
    networkManager = new QNetworkAccessManager(this);
    loadFromFirebase();
    serial->setPortName("COM4");
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    connect(serial, &QSerialPort::readyRead,
            this, &Inform::readSerialData);
    connect(ui->tableWidget, &QTableWidget::cellDoubleClicked,
            this, &Inform::onRowDoubleClicked);

    if(serial->open(QIODevice::ReadOnly))
        qDebug() << "Serial opened";
    else
        qDebug() << "Serial open failed";
}
Inform::~Inform()
{
    delete ui;
}

void Inform::on_Mapbutton_clicked()
{
    this->hide();
    parentWidget()->show();
}
void Inform::addRow(QString time,
                    QString observer,
                    double observerAlt,
                    QString target,
                    double targetAlt,
                    double distance)
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);

    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(time));
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(observer));
    ui->tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(observerAlt)));
    ui->tableWidget->setItem(row, 3, new QTableWidgetItem(target));
    ui->tableWidget->setItem(row, 4, new QTableWidgetItem(QString::number(targetAlt)));
    ui->tableWidget->setItem(row, 5, new QTableWidgetItem(QString::number(distance)));

    ui->tableWidget->scrollToBottom();
    ui->tableWidget->viewport()->update();

    emit newPacket();
}


void Inform::readSerialData(){
    while (serial->canReadLine())
    {
        QString line = QString::fromUtf8(serial->readLine()).trimmed();

        // Observer
        if(line.startsWith("Observer Coordinate"))
        {
            QStringList vals = line.split("=").last().split(",");

            if(vals.size() == 3)
            {
                lat1 = vals[0].trimmed().toDouble();
                lon1 = vals[1].trimmed().toDouble();
                alt1 = vals[2].trimmed().toDouble();

                emit sendObserver(lat1, lon1);
            }
        }

        // Target
        else if(line.startsWith("Target"))
        {
            QStringList vals = line.split("=").last().split(",");

            if(vals.size() == 3)
            {
                lat2 = vals[0].trimmed().toDouble();
                lon2 = vals[1].trimmed().toDouble();
                alt2 = vals[2].trimmed().toDouble();

                emit sendTarget(lat2, lon2);
            }
        }


        if(lat1 != 0 && lat2 != 0)
        {
            QGeoCoordinate p1(lat1, lon1);
            QGeoCoordinate p2(lat2, lon2);

            double distance = p1.distanceTo(p2);

            emit sendDistance(distance);

            QString observer = QString("%1, %2").arg(lat1).arg(lon1);
            QString target   = QString("%1, %2").arg(lat2).arg(lon2);

            QString time = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");

            addRow(time, observer, alt1, target, alt2, distance);
            uploadFirebase(lat1,lon1,alt1,lat2,lon2,alt2,distance);

            lat1 = lon1 = alt1 = 0;
            lat2 = lon2 = alt2 = 0;
        }
    }
}



void Inform::on_Savebutton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(
                this,
                "Save File",
                "",
                "CSV files (*.csv)");

    if(fileName.isEmpty())
        return;

    QFile file(fileName);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);

    int columnCount = ui->tableWidget->columnCount();
    int rowCount = ui->tableWidget->rowCount();

    // ghi header
    for(int i=0;i<columnCount;i++)
    {
        out << ui->tableWidget->horizontalHeaderItem(i)->text();

        if(i < columnCount-1)
            out << ",";
    }
    out << "\n";

    // ghi dữ liệu
    for(int row=0; row<rowCount; row++)
    {
        for(int col=0; col<columnCount; col++)
        {
            QTableWidgetItem *item = ui->tableWidget->item(row,col);

            if(item)
                out << item->text();

            if(col < columnCount-1)
                out << ",";
        }
        out << "\n";
    }

    file.close();

    qDebug() << "File saved:" << fileName;
}

void Inform::onRowDoubleClicked(int row, int column)
{
    Q_UNUSED(column);

    QString observer = ui->tableWidget->item(row,1)->text();
    QString target   = ui->tableWidget->item(row,3)->text();

    QStringList obs = observer.split(",");
    QStringList tar = target.split(",");

    if(obs.size()==2 && tar.size()==2)
    {
        double lat1 = obs[0].trimmed().toDouble();
        double lon1 = obs[1].trimmed().toDouble();

        double lat2 = tar[0].trimmed().toDouble();
        double lon2 = tar[1].trimmed().toDouble();

        emit sendObserver(lat1, lon1);
        emit sendTarget(lat2, lon2);

        // tính lại distance
        QGeoCoordinate p1(lat1, lon1);
        QGeoCoordinate p2(lat2, lon2);

        double distance = p1.distanceTo(p2);

        emit sendDistance(distance);

        // quay lại map
        if(parentWidget())
            parentWidget()->show();

        this->hide();
    }
}

void Inform::uploadFirebase(double lat1,double lon1,double alt1,
                            double lat2,double lon2,double alt2,
                            double distance)
{
    QUrl url("https://binnocular-default-rtdb.asia-southeast1.firebasedatabase.app/tracking_history.json");

    QJsonObject json;

    json["observer_lat"] = lat1;
    json["observer_lon"] = lon1;
    json["observer_alt"] = alt1;

    json["target_lat"] = lat2;
    json["target_lon"] = lon2;
    json["target_alt"] = alt2;

    json["distance"] = distance;
    json["time"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonDocument doc(json);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QNetworkReply *reply = networkManager->post(request, doc.toJson());

    connect(reply, &QNetworkReply::finished, [reply]() {

        if(reply->error())
            qDebug() << "Firebase error:" << reply->errorString();
        else
            qDebug() << "Firebase upload OK";

        reply->deleteLater();
    });
}

void Inform::loadFromFirebase()
{
    QUrl url("https://binnocular-default-rtdb.asia-southeast1.firebasedatabase.app/tracking_history.json");

    QNetworkRequest request(url);

    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply]() {

        if(reply->error())
        {
            qDebug() << "Firebase read error:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();

        QJsonDocument doc = QJsonDocument::fromJson(data);

        if(!doc.isObject())
        {
            reply->deleteLater();
            return;
        }

        QJsonObject root = doc.object();

        for(auto key : root.keys())
        {
            QJsonObject obj = root[key].toObject();

            QString time = obj["time"].toString();

            QDateTime dt = QDateTime::fromString(time, Qt::ISODate);
            time = dt.toString("dd/MM/yyyy hh:mm:ss");

            double lat1 = obj["observer_lat"].toDouble();
            double lon1 = obj["observer_lon"].toDouble();
            double alt1 = obj["observer_alt"].toDouble();

            double lat2 = obj["target_lat"].toDouble();
            double lon2 = obj["target_lon"].toDouble();
            double alt2 = obj["target_alt"].toDouble();

            double distance = obj["distance"].toDouble();

            QString observer = QString("%1, %2").arg(lat1).arg(lon1);
            QString target   = QString("%1, %2").arg(lat2).arg(lon2);

            addRow(time, observer, alt1, target, alt2, distance);
        }

        reply->deleteLater();
    });
}
