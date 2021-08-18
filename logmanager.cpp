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
#include "logmanager.h"

#include <QCoreApplication>
#include <QMutex>
#include <QDir>
#include <QDateTime>
#include <QDebug>

#define APP_LOGS_DIRNAME "Logs"
#define APP_LOGS_FILENUM (10)

LogManager::LogManager(QObject *parent) :
    QThread(parent)
    ,mIsWriteWorking(false)
    ,quitFlag(false)
    ,mFileFlushSize(1024)
{
    mLogInfosQueue.clear();
}

LogManager::~LogManager()
{
    mIsWriteWorking = false;
    quitFlag = true;
    mSemaphore.release(2);
    quit();
    wait();
}

bool LogManager::isLogging() const
{
    return mIsWriteWorking;
}

void LogManager::startLogging()
{
    if( !isRunning() )
    {
        start();
    }

    quitFlag = false;
    mIsWriteWorking = true;
}

void LogManager::suspendLogging()
{
    mIsWriteWorking = false;
}

void LogManager::setLogFileFlushSize(quint64 size)
{
    mFileFlushSize = size;
}

void LogManager::addLogWriteStr(const QString &str)
{
    if(mIsWriteWorking)
    {
        if(mLocker.tryLockForWrite())
        {
            mLogInfosQueue.enqueue(str);
            mLocker.unlock();
            mSemaphore.release(1);
        }
    }
}

void LogManager::run()
{
    bool logFileInitFlag = false;
    QFile logFileHander;
    quint64 writedDataLens = 0;

    while( !quitFlag )
    {
        mSemaphore.acquire(1);

        if( !logFileInitFlag )
        {
            logFileInitFlag = initLoggingFile(logFileHander);
        }

        if( !mLogInfosQueue.isEmpty() )
        {
            mLocker.lockForRead();
            const QString logStr = mLogInfosQueue.dequeue();
            mLocker.unlock();
            if(logFileHander.isOpen())
            {
                qint64 ret = logFileHander.write(logStr.toLocal8Bit().constData(), logStr.length());
                if(ret > 0)
                {
                    writedDataLens += static_cast<quint64>(ret);
                }

                if(writedDataLens >= mFileFlushSize )
                {
                    if( logFileHander.flush() )
                    {
                        writedDataLens = 0;
                    }
                }
                //可以增加文件大小到多少的时候换另一个文件继续写
            }
            else
            {
                logFileInitFlag = false;
            }
        }
    }

    if( logFileHander.isOpen() )
    {
        logFileHander.flush();
        logFileHander.close();
    }    

    writedDataLens = 0;
    logFileInitFlag = false;
    exec();
}

bool LogManager::initLoggingFile(QFile &fileHandler)
{
    bool initFlag = false;
    const QString appDir = QCoreApplication::applicationDirPath();

    QDir logsDir(appDir);
    if( !logsDir.exists(QString(APP_LOGS_DIRNAME)) )
    {
        if( !logsDir.mkdir(APP_LOGS_DIRNAME) )
        {
            emit sigLogsDirInitFailed();
            return initFlag;
        }
    }

    const QString backupDirStr = QDir::currentPath();

    if(true == logsDir.cd(QString(APP_LOGS_DIRNAME)))
    {
        QDir::setCurrent(logsDir.absolutePath());

        logsDir.setFilter(QDir::Files);
        logsDir.setSorting(QDir::Time);

        QStringList namefilters;
        namefilters << "*.log";
        logsDir.setNameFilters(namefilters);

        const QStringList logsFileNameList = logsDir.entryList();

        if(logsFileNameList.size() >= APP_LOGS_FILENUM)
        {
            //这个文件修改日期排序是向着历史前进的,意思是年份最老的在最后面
            fileHandler.setFileName(logsFileNameList.constLast());

            if( fileHandler.exists() )
            {
                const QString newFileName = QString("%1.log").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
                if(fileHandler.rename(newFileName))
                {
                    if(fileHandler.open(QIODevice::WriteOnly | QIODevice::Truncate))
                    {
                        initFlag = true;
                    }
                    else
                    {
                        emit sigLogFileOpenFailed(newFileName);
                        initFlag = false;
                    }
                }
                else
                {
                    emit sigLogFileRenameFailed(newFileName);
                    initFlag = false;
                }
            }
            else
            {
                emit sigLogFileExistsNon();
                initFlag = false;
            }
        }
        else
        {
            const QString logFileName = QString("%1.log").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
            fileHandler.setFileName(logFileName);
            if( fileHandler.open(QIODevice::WriteOnly | QIODevice::Truncate) )
            {
                initFlag = true;
            }
            else
            {
                emit sigLogFileOpenFailed(logFileName);
                initFlag = false;
            }
        }
    }
    else
    {
        emit sigLogsDirInitFailed();
    }

    QDir::setCurrent(backupDirStr);

    return initFlag;
}
