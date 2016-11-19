/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "settingsdialog.h"

#include <QMessageBox>
#include <QLabel>
#include <QtSerialPort/QSerialPort>

//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
//! [0]
    ui->setupUi(this);
    setGeometry(400, 250, 542, 390);
    //setupRealtimeData(ui->customPlot);
    setupPlot(ui->customPlot);
//! [1]
    serial = new QSerialPort(this);
//! [1]
    settings = new SettingsDialog;

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionConfigure->setEnabled(true);
    ui->actionClear->setEnabled(true);
    ui->actionSave->setEnabled(false);
    ui->actionstopSave->setVisible(false);
    status = new QLabel;
    ui->statusBar->addWidget(status);

    initActionsConnections();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateHz()));
    timer->start(1000);
    Hz_tmp=0;

    connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &MainWindow::handleError);

//! [2]
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);


//! [2]

//! [3]
}
//! [3]

MainWindow::~MainWindow()
{
    delete settings;
    delete ui;
    plik.close();
}

//! [4]
void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    if (serial->open(QIODevice::ReadWrite)) {

        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionConfigure->setEnabled(false);
        ui->actionSave->setEnabled(true);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
        writeData("rstrt");
        flaga_rstrt=1;

    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        showStatusMessage(tr("Open error"));
    }
}
//! [4]

//! [5]
void MainWindow::closeSerialPort()
{

    if (serial->isOpen())
        serial->close();

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionSave->setEnabled(false);
    ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
    stopSave();
}
//! [5]
//!
void MainWindow::setupRealtimeData(QCustomPlot *customPlot)
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  QMessageBox::critical(this, "", "You're using Qt < 4.7, the realtime data demo needs functions that are available with Qt 4.7 to work properly");
#endif


  // include this section to fully disable antialiasing for higher performance:
  /*
  customPlot->setNotAntialiasedElements(QCP::aeAll);
  QFont font;
  font.setStyleStrategy(QFont::NoAntialias);
  customPlot->xAxis->setTickLabelFont(font);
  customPlot->yAxis->setTickLabelFont(font);
  customPlot->legend->setFont(font);
  */
  customPlot->addGraph(); // blue line
  customPlot->graph(0)->setPen(QPen(Qt::blue));
  customPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
  customPlot->graph(0)->setAntialiasedFill(false);
  customPlot->addGraph(); // red line
  customPlot->graph(1)->setPen(QPen(Qt::red));
  customPlot->graph(0)->setChannelFillGraph(customPlot->graph(1));
/**/
  customPlot->addGraph(); // blue dot
  customPlot->graph(2)->setPen(QPen(Qt::blue));
  customPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
  customPlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
  customPlot->addGraph(); // red dot
  customPlot->graph(3)->setPen(QPen(Qt::red));
  customPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
  customPlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);

  customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
  customPlot->xAxis->setAutoTickStep(false);
  customPlot->xAxis->setTickStep(2);
  customPlot->axisRect()->setupFullAxesBox();

  // make left and bottom axes transfer their ranges to right and top axes:
  connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
  connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));
  customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
  // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
//  connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
//  dataTimer.start(0); // Interval 0 means to refresh as fast as possible
}

void MainWindow::realtimeData()
{
  // calculate two new data points:
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  double key = 0;
#else
  double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
#endif/*

  if (key-lastPointKey > 0.01) // at most add point every 10 ms
  {
 */
    //static double lastPointKey = 0;
    double value0 = pomiar[0]; //qSin(key*1.6+qCos(key*1.7)*2)*10 + qSin(key*1.2+0.56)*20 + 26;
    //double value1 = pomiar[1]; //qSin(key*1.3+qCos(key*1.2)*1.2)*7 + qSin(key*0.9+0.26)*24 + 26;
    // add data to lines:
    ui->customPlot->graph(0)->addData(key, value0);
   // ui->customPlot->graph(1)->addData(key, value1);
    // set data of dots:
   /* ui->customPlot->graph(2)->clearData();
    ui->customPlot->graph(2)->addData(key, value0);
    ui->customPlot->graph(3)->clearData();
    ui->customPlot->graph(3)->addData(key, value1);
*/     // remove data of lines that's outside visible range:
    ui->customPlot->graph(0)->removeDataBefore(key-8);
    ui->customPlot->graph(1)->removeDataBefore(key-8);
    // rescale value (vertical) axis to fit the current data:
    ui->customPlot->graph(0)->rescaleValueAxis();
    ui->customPlot->graph(1)->rescaleValueAxis(true);
   // lastPointKey = key;
 // }
  // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlot->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
   // ui->customPlot->graph(0)->rescaleAxes();
    ui->customPlot->replot();

  // calculate frames per second:
  static double lastFpsKey;
  static int frameCount;
  ++frameCount;
  if (key-lastFpsKey > 2) // average fps over 2 seconds
  {
    ui->statusBar->showMessage(
          QString("%1 Hz, Total Data points: %2")
          .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
          .arg(ui->customPlot->graph(0)->data()->count()+ui->customPlot->graph(1)->data()->count())
          , 0);
    lastFpsKey = key;
    frameCount = 0;
  }
}


void MainWindow::setupPlot(QCustomPlot *customPlot){

  customPlot->addGraph();

  customPlot->graph(0)->setPen(QPen(Qt::transparent));
  customPlot->graph(0)->setLineStyle(QCPGraph::lsStepRight);

  QLinearGradient gradient;
  gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
  gradient.setStart(0.0, 0.0);
  gradient.setFinalStop(0.0, 1.0);
  gradient.setColorAt(1.0,Qt::blue);
  gradient.setColorAt(0.0,Qt::yellow);
  customPlot->graph(0)->setBrush(gradient);
  customPlot->xAxis->setRange(0, 12);

  customPlot->xAxis->setScaleType(QCPAxis::stLinear);
  customPlot->yAxis->setRange(-1, 190);
  customPlot->yAxis->setLabel("");
  //customPlot->xAxis->setVisible(false);
}

void MainWindow::update(){

    QVector<double> x(13), y(13);

    for (int i=0; i<12; i++){

        x[i] = i;
        y[i+1] = pomiar[i];
        //qDebug()<<"x:"<<x[i]<<" y:"<<y[i];
    }

    ui->customPlot->graph(0)->clearData();
    ui->customPlot->graph(0)->setData(x, y);
    //ui->customPlot->graph(0)->rescaleAxes();
    ui->customPlot->replot();
}


void MainWindow::Clear(){

    ui->customPlot->graph(0)->clearData();
    //ui->customPlot->graph(1)->clearData();
    ui->customPlot->replot();
}

void MainWindow::Save(){

    ui->actionSave->setVisible(false);
    ui->actionstopSave->setVisible(true);
    static int a=1;
    QString nazwa=QString::number(a);
    a++;
    plik.setFileName(nazwa+".txt");
    if(!plik.open(QFile::WriteOnly|QFile::Text)) QMessageBox::information(this,"bład","nie utworzono pliku");
}

void MainWindow::stopSave(){

    ui->actionstopSave->setVisible(false);
    ui->actionSave->setVisible(true);
    plik.close();
}

void MainWindow::readData(){

    Data=serial->readAll();
    //qDebug()<<Data;
    Buffer+=QString::fromStdString(Data.toStdString());

    QTextStream out(&plik);
    //qDebug()<<Buffer;
        
    if((Buffer.startsWith("a")&&(Buffer.contains("x")&&(!flaga_rstrt)))){

        int ilosc=Buffer.count('x');//ilosc ramek danych
        int miejsce=Buffer.lastIndexOf('x'); 
        qDebug()<<"ilosc:"<<ilosc;

        QString tmp;
        QString reszta;

        if(Buffer.size()>miejsce){//przerzucamy niepełny koniec buferu na pocztek nowego

            reszta=Buffer.mid(miejsce+1);
            Buffer=Buffer.left(miejsce+1);
            Buffer=Buffer.replace("x",",");
            //qDebug()<<"reszta";qDebug()<<reszta;
        }

        Buffer.remove('a');
        QStringList split_buffer= Buffer.split(",",QString::SkipEmptyParts);
        //qDebug()<<split_buffer;

        Hz_tmp=Hz_tmp+ilosc;

        for(int i=0;i<ilosc_cz;i++)

            pomiar[((split_cz.at(i)).toInt()-1)]=(split_buffer.at(i)).toDouble();

        ui->statusBar->showMessage(QString("%1 Hz  | %2 | %3 | %4 | %5 | %6 | %7 | %8 | %9 | %10 | %11 | %12 | %13 |")
                .arg(Hz).arg(pomiar[0]).arg(pomiar[1]).arg(pomiar[2])
                .arg(pomiar[3]).arg(pomiar[4]).arg(pomiar[5])
                .arg(pomiar[6]).arg(pomiar[7]).arg(pomiar[8])
                .arg(pomiar[9]).arg(pomiar[10]).arg(pomiar[11]),0);
        update();
   /*
        for(int j=0;j<ilosc*2;j=j+2){// dla bufera roznej wielkosci


                tmp=split_buffer.at(j);
                //test=tmp.toDouble();
                //if(test>50&&test<200){
                pomiar[i]=tmp.toDouble();
                 qDebug()<<tmp;
                i++;
                update();
                //tmp=split_buffer.at(j+1);
                //pomiar[]=tmp.toDouble();
                //out<<pomiar[0]<<"\t"<<pomiar[1]<<"\n";
                //qDebug()<<pomiar[0]<<"\t"<<pomiar[1];
                //}
                //realtimeData();
                // przetestowac poniższe
                //  connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
                //  dataTimer.start(0); // Interval 0 means to refresh as fast as possible

        }*/
        Buffer=reszta;

        //tmp.clear();
        //QTextStream out(&plik);
        //out<<pomiar[0]<<"\t"<<pomiar[1]<<"\n";

    }

    else if((Buffer.contains("b"))&&(Buffer.contains("c"))&&flaga_rstrt){

        int miejsce=Buffer.indexOf('c');
        Buffer=Buffer.left(miejsce);
        miejsce=Buffer.indexOf('b');
        Buffer=Buffer.mid(miejsce+1);

        split_cz= Buffer.split(",",QString::SkipEmptyParts);
        ilosc_cz=split_cz.count();
        qDebug()<<split_cz;
        qDebug()<<ilosc_cz;

        flaga_rstrt=0;

        for(int i=0;i<12;i++) pomiar[i]=0;

        for(int i=0;i<ilosc_cz;i++)//-1 w miejscach gdzie powienien byc pomiar, w celu detekcji bledow

             pomiar[((split_cz.at(i)).toInt()-1)]=-1;
    }

    else if(flaga_rstrt) writeData("rstrt");//nie wczytano które cz sa podlaczone, reset

    static int k=0; //ilosc usunietych buferow

    if(Buffer.size()>160){//zbyt duzo dodanych buferow, czyscimy

        Buffer.clear();
        k++;
        qDebug()<<k;

        if(k>10){//10 wyczyszczynoch buferow, reset mikrokontrolera

            writeData("rstrt");
            k=0;
        }
    }

}

void MainWindow::updateHz(){

    Hz=Hz_tmp;
    Hz_tmp=0;
    //ui->statusBar->showMessage(QString("%1 Hz ").arg(Hz),0);
    timer->start(1000);
}

//! [6]
void MainWindow::writeData(const QByteArray &data){

    int roz=serial->write(data);
}
//! [6]

//! [7]
 /*
void MainWindow::readData()
{
    QByteArray data = serial->readAll();

}*/
//! [7]

//! [8]
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}
//! [8]

void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionConfigure, &QAction::triggered, settings, &MainWindow::show);
    connect(ui->actionClear, &QAction::triggered, this, &MainWindow::Clear);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::Save);
    connect(ui->actionstopSave, &QAction::triggered, this, &MainWindow::stopSave);
 }

void MainWindow::showStatusMessage(const QString &message)
{
    status->setText(message);
}
