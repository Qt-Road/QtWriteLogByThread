#include "mainwindow.h"
#include <QApplication>
#include <QtGlobal>
#include "mylogger.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

  #if(QT_VERSION <= QT_VERSION_CHECK(5,0,0))
    qInstallMsgHandler(MyLogger::myMessageOutput);//qt4
  #else
    qInstallMessageHandler(MyLogger::myMessageOutput);
  #endif
    MyLogger::instance()->startLogging();

    MainWindow w;
    w.show();

    return a.exec();
}
