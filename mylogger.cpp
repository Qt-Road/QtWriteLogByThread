/*******************************************************************
** Copyright(c) 2021 hjc. All rights reserved.
**
** brief:
**
** Hisory:
** <version> |<author>  |<time>      |<Description>
** -----------------------------------------------------------------
** 1.0       |hjc       |            |first version
**
**
*******************************************************************/
#include "mylogger.h"
#include <QString>
#include <QDateTime>
#include <cstdio>

#include "logmanager.h"
#include "logworker.h"

Q_GLOBAL_STATIC(MyLogger, mylogger)

#ifdef THREAD_USE_WORKER
    MyLogger * MyLogger::_pThis = Q_NULLPTR;
    static bool mIsLogging = false;
#else    
    static LogManager *mLogManager = Q_NULLPTR;
#endif

MyLogger::MyLogger(QObject *parent) :
    QObject(parent)
{
  #ifdef THREAD_USE_WORKER
    mThreadInit = false;
    _pThis = this;
  #endif
}

MyLogger::~MyLogger()
{
  #ifdef THREAD_USE_WORKER
    workerThread.quit();
    workerThread.wait();
  #else
    if(Q_NULLPTR != mLogManager)
    {
        mLogManager->suspendLogging();
        delete mLogManager;
        mLogManager = Q_NULLPTR;
    }
  #endif
}

MyLogger *MyLogger::instance()
{
    return mylogger();
}

bool MyLogger::isLogging() const
{
  #ifdef THREAD_USE_WORKER
      return mIsLogging;
  #else
    if(Q_NULLPTR != mLogManager)
    {
        return mLogManager->isLogging();
    }
    return false;
  #endif
}

void MyLogger::startLogging()
{
  #ifdef THREAD_USE_WORKER
    if( !mThreadInit )
    {
        LogWorker *worker = new LogWorker();
        worker->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, SIGNAL(sigWriteLogStr(QString)), worker, SLOT(slotWriteLogStr(QString)));
        workerThread.start();
        mThreadInit = true;
    }

    mIsLogging = true;
  #else
    if(Q_NULLPTR == mLogManager)
    {
        mLogManager = new LogManager(Q_NULLPTR);
    }

    if(Q_NULLPTR != mLogManager)
    {
        mLogManager->startLogging();
    }
  #endif
}

void MyLogger::suspendLogging()
{
  #ifdef THREAD_USE_WORKER
    mIsLogging = false;
  #else
    if(Q_NULLPTR != mLogManager)
    {
        mLogManager->suspendLogging();
    }
  #endif
}

void MyLogger::setLogFlushSize(quint64 size)
{
#ifdef THREAD_USE_WORKER
    emit sigSetLogFlushSize(size);
#else
    if(Q_NULLPTR != mLogManager)
    {
        mLogManager->setLogFileFlushSize(size);
    }
#endif
}

void MyLogger::myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  #ifdef THREAD_USE_WORKER
    if( !mIsLogging )
    {
        return;
    }
  #endif
    QByteArray localMsg = msg.toLocal8Bit();
    QString strMessage;

    //FILE *logFileStream;
    //logFileStream = fopen("./logs.log", "a");
    //fclose(logFileStream);
    if(type >= QtMsgType::QtFatalMsg)
    {
        fprintf(stderr, "%s", strMessage.toLocal8Bit().constData());
    }

    switch(type){
    case QtDebugMsg:
        //fprintf(logFileStream, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        strMessage = QString("Debug: %1 (%2:%3, %4)").arg(localMsg.constData())\
                .arg(context.file).arg(context.line).arg(context.function);
        break;
    case QtInfoMsg:
        strMessage = QString("Info: %1 (%2:%3, %4)").arg(localMsg.constData())\
                .arg(context.file).arg(context.line).arg(context.function);
        break;
    case QtWarningMsg:
        strMessage = QString("Warning: %1 (%2:%3, %4)").arg(localMsg.constData())\
                .arg(context.file).arg(context.line).arg(context.function);
        break;
    case QtCriticalMsg:
        strMessage = QString("Critical: %1 (%2:%3, %4)").arg(localMsg.constData())\
                .arg(context.file).arg(context.line).arg(context.function);
        break;
    case QtFatalMsg:
        strMessage = QString("Fatal: %1 (%2:%3, %4)").arg(localMsg.constData())\
                .arg(context.file).arg(context.line).arg(context.function);
        abort();
    }

    const QString strDateTime = QDateTime::currentDateTime().toString("--MMdd hh:mm:ss");
    strMessage.append(strDateTime + "\n");
  #ifdef THREAD_USE_WORKER
    emit _pThis->sigWriteLogStr(strMessage);
  #else
    if(Q_NULLPTR != mLogManager)
    {
        mLogManager->addLogWriteStr(strMessage);
    }
  #endif
}
