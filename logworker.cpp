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
#include "logworker.h"

#include <QFile>
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QTextStream>

#define APP_LOGS_DIRNAME "Logs"
#define APP_LOGS_FILENUM (10)

LogWorker::LogWorker(QObject *parent) :
    QObject(parent)
    ,mFlagFileIsInit(false)
    ,mFileFlushSize(0)
    ,mWritedDataLens(0)
{

}

LogWorker::~LogWorker()
{
    if(mLogFile.isOpen())
    {
        mLogFile.flush();
        mLogFile.close();
    }
    mFlagFileIsInit = false;
    mFileFlushSize = 0;
    mWritedDataLens = 0;
}

void LogWorker::slotWriteLogStr(const QString &logStr)
{
    if( !mFlagFileIsInit )
    {
        mFlagFileIsInit = initLoggingFile(mLogFile);
        //return ;
    }

    if( !logStr.isEmpty() )
    {
        if(mLogFile.isOpen())
        {
          #if(1)
            qint64 ret = mLogFile.write(logStr.toLocal8Bit().constData(), logStr.length());
            if(ret > 0)
            {

                if(0 == mFileFlushSize)
                {
                    mLogFile.flush();
                }
                else
                {
                    mWritedDataLens += static_cast<quint64>(ret);
                    if(mWritedDataLens >= mFileFlushSize )
                    {
                        if( mLogFile.flush() )
                        {
                            mWritedDataLens = 0;
                        }
                    }

                }

            }
          #else
            //还是有点大材小用的感觉
            QTextStream out(&mLogFile);
            out.setCodec("UTF-8");
            out << logStr;
            out.flush();
          #endif
        }
        else
        {
            mFlagFileIsInit = false;
        }
    }
}

void LogWorker::slotSetLogFlushSize(quint64 size)
{
    mFileFlushSize = size;
    mLogFile.flush();
    mWritedDataLens = 0;
}

bool LogWorker::initLoggingFile(QFile &fileHandler)
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
