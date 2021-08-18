#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QThread>
#include <QReadWriteLock>
#include <QFile>
#include <QSemaphore>
#include <QQueue>

class LogManager : public QThread
{
    Q_OBJECT
public:
    explicit LogManager(QObject *parent = Q_NULLPTR);
    ~LogManager() Q_DECL_OVERRIDE;

    bool isLogging() const;
    void startLogging();
    void suspendLogging();
    void setLogFileFlushSize(quint64 size);

signals:
    void sigLogsDirInitFailed(void);
    void sigLogFileExistsNon(void);
    void sigLogFileRenameFailed(const QString &filename);
    void sigLogFileOpenFailed(const QString &filename);

public:
    void addLogWriteStr(const QString &str);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    bool initLoggingFile(QFile &fileHandler);

private:
    bool mIsWriteWorking;
    bool quitFlag;

    quint64 mFileFlushSize;
    QReadWriteLock mLocker;
    QSemaphore mSemaphore;
    QQueue<QString> mLogInfosQueue;
};


#endif // LOGMANAGER_H
