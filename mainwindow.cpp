#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    cw = new ImageShowWidget(this);
    jlink = new JLink();

    ui->gridLayout->addWidget(cw,0,1,2,-1);
    ui->gridLayout->addWidget(jlink->info,2,0,-1,2);

    timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setInterval(2000);
    connect(timer,SIGNAL(timeout()),this,SLOT(updateVbuff()));
    connect(ui->lineEdit,SIGNAL(textChanged(QString)),this,SLOT(updateBaseAddrColumnRow()));
    connect(ui->lineEdit_column,SIGNAL(textChanged(QString)),this,SLOT(updateBaseAddrColumnRow()));
    connect(ui->lineEdit_row,SIGNAL(textChanged(QString)),this,SLOT(updateBaseAddrColumnRow()));
    this->loadIni();
    timer->start();
	
}

MainWindow::~MainWindow()
{
    this->saveIni();
    delete ui;
}
#include <iostream>
void MainWindow::paintEvent(QPaintEvent *){
//    QImage img = QImage(column,row,QImage::Format_ARGB32);
//    for(int i = 0;i<row;i++){
//        uchar *p = img.scanLine(i);
//        for(int j = 0;j<column;j++){
//            char r= this->vbuff[i*column + j];
//            p[j*4 + 0] = r;
//            p[j*4 + 1] = r;
//            p[j*4 + 2] = r;
//        }
//    }
//    QPainter painter(this);
//    painter.drawImage(0,20,img);
//    painter.end();
}
void MainWindow::updateVbuff(){
   jlink->readMem(this->baseAddr,column*row,vbuff.data());
   QVector<uint8_t> daEx;
   daEx.resize(ui->spinBox_EX_Context->value());
   bool r;
   uint32_t baseAddrEx = this->ui->lineEdit_EX_Addr->text().toInt(&r,16) + ui->spinBox_Ex_offset->value();
   if(!r){
       QMessageBox::warning(this,"Warning",tr("Illegal Extra address!!!"));
   }
   jlink->readMem(baseAddrEx, daEx.size(),daEx.data());
   if(ui->checkBox_autoClear->isChecked()){
       ui->textEdit_Ex->clear();
   }
    for (int i = 0;i<daEx.size();i++)
    {
        ui->textEdit_Ex->append(tr("0x%1:%2").arg(QString::number(baseAddrEx + i, 16)).arg(daEx[i]));
    }
    int threshold = ui->spinBox_threshold->value();
    if(ui->checkBox_binaryzation->isChecked()){
        for(int i = 0;i< this->vbuff.size();i++){
            this->vbuff[i] = this->vbuff[i] > threshold ? 255:0;
        }
    }
   this->cw->updateContentParameter(this->vbuff,this->column,this->row);
   this->update();
}
void MainWindow::loadIni()
{
    QSettings settings("settings.ini", QSettings::IniFormat);

    ui->lineEdit->setText(settings.value("global/baseAddr", "0x20000000").toString());
    ui->lineEdit_row->setText(settings.value("global/row", "240").toString());
    ui->lineEdit_column->setText(settings.value("global/column", "320").toString());
    ui->spinBox_EX_Context->setValue(settings.value("global/ExDaCount", "4").toString().toInt());
    ui->lineEdit_EX_Addr->setText(settings.value("global/ExDaAddr", "0x20000000").toString());
    ui->spinBox_catch_time_interval->setValue(settings.value("global/catchTimeInterval",1000).toInt());
    ui->spinBox_threshold->setValue(settings.value("global/threshold",160).toInt());
    ui->spinBox_Ex_offset->setValue(settings.value("global/offsetAddr",0).toInt());
    this->updateBaseAddrColumnRow();
}
void MainWindow::saveIni()
{
    QSettings settings("settings.ini", QSettings::IniFormat);
    settings.beginGroup("global");
    settings.setValue("row", ui->lineEdit_row->text());
    settings.setValue("column", ui->lineEdit_column->text());
    settings.setValue("baseAddr", ui->lineEdit->text());
    settings.setValue("ExDaAddr", ui->lineEdit_EX_Addr->text());
    settings.setValue("ExDaCount", ui->spinBox_EX_Context->value());
    settings.setValue("catchTimeInterval", ui->spinBox_catch_time_interval->value());

    settings.setValue("offsetAddr", ui->spinBox_Ex_offset->value());
    settings.setValue("threshold", ui->spinBox_threshold->value());
    settings.endGroup();
}

void MainWindow::updateBaseAddrColumnRow(){
    int column = ui->lineEdit_column->text().toInt();
    int row  = ui->lineEdit_row->text().toInt();
    bool r;
    uint32_t addr = ui->lineEdit->text().toInt(&r,16);
    if(!r){
        QMessageBox::warning(this,"Check Mem failed","Please Check addr");
    }else{
        this->vbuff.resize(row*column);
        this->row = row;
        this->column = column;
        this->baseAddr = addr;
        jlink->appendInfo(tr("set column:%1, row%2,baseAddr %3").arg(column).arg(row).arg(addr));

    }
    int value = ui->spinBox_catch_time_interval->value();
    if(value< 20){
        QMessageBox::warning(this,"Warning","Too small catch-time-interval.");
    }
    timer->setInterval(value);
}
