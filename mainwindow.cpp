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

#define ROZ 12 //wielkosc ramki
//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
//! [0]
    ui->setupUi(this);
    setGeometry(400, 250, 542, 390);

    setupRealtimeData(ui->customPlot2);
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
    setupchannelbox();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateHz()));
    timer->start(1000);
    Hz_tmp=0;
    for(int i=0;i<12;i++) pomiar[i]=0;

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
        writeData("reset");

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

  for(int i=0;i<12;i++){
      customPlot->addGraph();
      customPlot->graph(i)->setVisible(0);
      customPlot->graph(i)->removeFromLegend();
  }
      customPlot->graph(0)->setPen(QPen(Qt::blue));
      customPlot->graph(0)->setName("Cz 1");
      customPlot->graph(1)->setPen(QPen(Qt::red));
      customPlot->graph(1)->setName("Cz 2");
      customPlot->graph(2)->setPen(QPen(Qt::green));
      customPlot->graph(2)->setName("Cz 3");
      customPlot->graph(3)->setPen(QPen(Qt::yellow));
      customPlot->graph(3)->setName("Cz 4");
      customPlot->graph(4)->setPen(QPen(Qt::cyan));
      customPlot->graph(4)->setName("Cz 5");
      customPlot->graph(5)->setPen(QPen(Qt::black));
      customPlot->graph(5)->setName("Cz 6");
      customPlot->graph(6)->setPen(QPen(Qt::magenta));
      customPlot->graph(6)->setName("Cz 7");
      customPlot->graph(7)->setPen(QPen(Qt::darkRed));
      customPlot->graph(7)->setName("Cz 8");
      customPlot->graph(8)->setPen(QPen(Qt::darkBlue));
      customPlot->graph(8)->setName("Cz 9");
      customPlot->graph(9)->setPen(QPen(Qt::darkGreen));
      customPlot->graph(9)->setName("Cz 10");
      customPlot->graph(10)->setPen(QPen(Qt::gray));
      customPlot->graph(10)->setName("Cz 11");
      customPlot->graph(11)->setPen(QPen(Qt::darkCyan));
      customPlot->graph(11)->setName("Cz 12");

  customPlot->legend->setVisible(true);
  customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
  customPlot->xAxis->setAutoTickStep(false);
  customPlot->xAxis->setTickStep(2);
  customPlot->axisRect()->setupFullAxesBox();
  customPlot->yAxis->setRange(-10,100);
  customPlot->yAxis->setLabel("nacisk 0.1 [N]");

  customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
  // make left and bottom axes transfer their ranges to right and top axes:
  //connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
 // connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

}

void MainWindow::realtimeData(QCustomPlot *customPlot)
{
  // calculate two new data points:
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  double key = 0;
#else
  double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
#endif

    for(int i=0;i<12;i++){

        customPlot->graph(i)->addData(key,pomiar[i]);
        customPlot->graph(i)->removeDataBefore(key-8);
    }

    customPlot->xAxis->setRange(key+0.25, 8, Qt::AlignRight);

    customPlot->replot();
}

/*stare ::setupPlot
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
  customPlot->yAxis->setRange(-10, 100);
  customPlot->yAxis->setLabel("nacisk 0.1 [N]");
  //customPlot->xAxis->setVisible(false);
*/

void MainWindow::setupPlot(QCustomPlot *customPlot){

    for(int i=0;i<12;i++){//os x sie nie zmienia, os y zerowa

        QVector<double> tmp = QVector<double>() <<i+1;
        data_x.append(tmp);
    }

    for(int i=0;i<12;i++){

        QCPBars *bar = new QCPBars(customPlot->xAxis, customPlot->yAxis);
        bar->setAntialiased(false);
        bar->setWidth(0.94);
        plot_bars.append(bar);
    }

    customPlot->xAxis->setRange(0, 13);
    customPlot->yAxis->setRange(-10, 150);
    customPlot->yAxis->setLabel("nacisk 0.1 [N]");

    linia=new QCPItemLine(customPlot);
    linia->setPen(QPen(Qt::black));
    linia->start->setCoords(0,0);
    linia->end->setCoords(13,0);

}

/*

    QCPBars *bars = new QCPBars(customPlot->xAxis, customPlot->yAxis);
    bars->setData(datax1, datay1);

    bars->setBrush(QColor(238,0,0,150));//czerwony
    bars->setPen(QColor(238,0,0));

    bars1->setBrush(QColor(1,139,0,150));//zielony
    bars1->setPen(QColor(1,139,0));

    bars->setBrush(QColor(205,173,0,150));//zolty
    bars->setPen(QColor(205,173,0));

*/

void MainWindow::update(){

    if(!ui->tabWidget->currentIndex()){

        for(int i=0;i<12;i++){

            QVector<double> tmp = QVector<double>()<<pomiar[i];

            (plot_bars.at(i))->clearData();
            (plot_bars.at(i))->setData(data_x.at(i),tmp);

            if(pomiar[i]>ui->verticalSlider->value()){

                (plot_bars.at(i))->setBrush(QColor(1,139,0,150));//zielony
                (plot_bars.at(i))->setPen(QColor(1,139,0));
            }
            else{

                (plot_bars.at(i))->setBrush(QColor(205,173,0,150));//zolty
                (plot_bars.at(i))->setPen(QColor(205,173,0));
            }
        }

        ui->customPlot->replot();
    }
    else realtimeData(ui->customPlot2);
}
/*stare ::update
    QVector<double> x(13), y(13);

    for (int i=0; i<12; i++){

        x[i] = i+1;
        y[i] = pomiar[i];
        //qDebug()<<"x:"<<x[i]<<" y:"<<y[i];
    }

    ui->customPlot->graph(0)->clearData();
    ui->customPlot->graph(0)->setData(x, y);
    //ui->customPlot->graph(0)->rescaleAxes();
    ui->customPlot->replot();
*/

void MainWindow::Clear(){

    for(int i=0;i<12;i++){

        ui->customPlot2->graph(i)->clearData();
        (plot_bars.at(i))->clearData();
    }

    ui->customPlot->replot();
    ui->customPlot2->replot();
}


void MainWindow::Save(){

    ui->actionSave->setVisible(false);
    ui->actionstopSave->setVisible(true);
    static int a=1;
    QString nazwa=QString::number(a);
    a++;
    plik.setFileName(nazwa+".txt");
    if(!plik.open(QFile::WriteOnly|QFile::Text)) QMessageBox::information(this,"bład","nie utworzono pliku");
    elapsed_timer.restart();
}


void MainWindow::stopSave(){

    ui->actionstopSave->setVisible(false);
    ui->actionSave->setVisible(true);
    plik.close();

}


void MainWindow::readData(){

    Data=serial->readAll();qDebug()<<Data;

    Buffer+=QString::fromStdString(Data.toStdString());
    //qDebug()<<"Buffer";qDebug()<<Buffer;



    if((Buffer.startsWith("a")&&(Buffer.contains("x")))){

        int ilosc=Buffer.count('x');//ilosc ramek danych
        int miejsce=Buffer.lastIndexOf('x');   qDebug()<<"ilosc:"<<ilosc;
        
        Hz_tmp=Hz_tmp+ilosc;
        
        QString reszta;
               
        if(Buffer.size()>miejsce){//przerzucamy niepełny koniec buferu na pocztek nowego

            reszta=Buffer.mid(miejsce+1);

            Buffer=Buffer.left(miejsce); //qDebug()<<"reszta";qDebug()<<reszta;
        }

        Buffer.remove('a');

        QStringList buffered= Buffer.split("x",QString::SkipEmptyParts);

        for(int i=0;i<buffered.size();i++){

            QStringList split_buffer = (buffered.at(i)).split(",",QString::SkipEmptyParts);

            for(int i=0;i<ROZ;i++){//srednia z poprzedniego pomiaru

                pomiar[i]=(split_buffer.at(i)).toFloat();
                //pomiar[i]=(pomiar[i]+pomiar_old[i])/2;
               // pomiar_old[i]=pomiar[i];
                //qint32 tmp=qRound(pomiar[i]);
                //pomiar[i]=tmp;
            }

            if(ui->actionstopSave->isVisible()){

                static QTextStream out(&plik);
                for(int i=0;i<ROZ;i++) out<<pomiar[i]<<" ";
                out<<elapsed_timer.elapsed()/1000.0;
                out<<endl;
            }

            ui->statusBar->showMessage(QString("%1 Hz     | %2 | %3 | %4 | %5 | %6 | %7 | %8 | %9 | %10 | %11 | %12 | %13 |")
                .arg(Hz).arg(pomiar[0]).arg(pomiar[1]).arg(pomiar[2])
                .arg(pomiar[3]).arg(pomiar[4]).arg(pomiar[5])
                .arg(pomiar[6]).arg(pomiar[7]).arg(pomiar[8])
                .arg(pomiar[9]).arg(pomiar[10]).arg(pomiar[11]),0);

            update();

            Buffer=reszta;
        }
    }
    
    static int k=0; //ilosc usunietych buferow

    if(Buffer.size()>160){//zbyt duzo dodanych buferow, czyscimy

        Buffer.clear();
        k++;
        qDebug()<<k;

        if(k>10){//10 wyczyszczonych buferow, reset mikrokontrolera

            writeData("reset");
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


void MainWindow::on_verticalSlider_valueChanged(int value)
{
    linia->start->setCoords(0,(double)value);
    linia->end->setCoords(13,(double)value);
    ui->customPlot->replot();
}


void MainWindow::setupchannelbox(){

    ui->channelBox->addItem(tr("Cz 1"));
    ui->channelBox->addItem(tr("Cz 2"));
    ui->channelBox->addItem(tr("Cz 3"));
    ui->channelBox->addItem(tr("Cz 4"));
    ui->channelBox->addItem(tr("Cz 5"));
    ui->channelBox->addItem(tr("Cz 6"));
    ui->channelBox->addItem(tr("Cz 7"));
    ui->channelBox->addItem(tr("Cz 8"));
    ui->channelBox->addItem(tr("Cz 9"));
    ui->channelBox->addItem(tr("Cz 10"));
    ui->channelBox->addItem(tr("Cz 11"));
    ui->channelBox->addItem(tr("Cz 12"));
}

void MainWindow::on_pushButton_clicked(){

    ui->customPlot2->graph(ui->channelBox->currentIndex())->addToLegend();
    ui->customPlot2->graph(ui->channelBox->currentIndex())->setVisible(1);
    ui->customPlot2->replot();
}


void MainWindow::on_pushButton_2_clicked(){

    ui->customPlot2->graph(ui->channelBox->currentIndex())->setVisible(0);
    ui->customPlot2->graph(ui->channelBox->currentIndex())->removeFromLegend();
    ui->customPlot2->replot();
}
