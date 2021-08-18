#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "mylogger.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug () << "MainWindow::MainWindow 0";
    MyLogger::instance()->suspendLogging();
    //qInstallMessageHandler(Q_NULLPTR);
    qDebug () << "MainWindow::MainWindow 1";
    MyLogger::instance()->startLogging();
    qDebug () << "MainWindow::MainWindow 2";
}

MainWindow::~MainWindow()
{
    delete ui;
}
