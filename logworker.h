#ifndef LOGWORKER_H
#define LOGWORKER_H

#include <QObject>
#include <QFile>

class LogWorker : public QObject
{
    Q_OBJECT
public:
    LogWorker(QObject *parent = Q_NULLPTR);
    ~LogWorker();

signals:
    void sigLogsDirInitFailed(void);
    void sigLogFileExistsNon(void);
    void sigLogFileRenameFailed(const QString &filename);
    void sigLogFileOpenFailed(const QString &filename);

public slots:
    void slotWriteLogStr(const QString &logStr);
    void slotSetLogFlushSize(quint64 size);

private:
    bool initLoggingFile(QFile &fileHandler);

    QFile mLogFile;
    bool mFlagFileIsInit;
    quint64 mFileFlushSize;
    quint64 mWritedDataLens;
};

#endif // LOGWORKER_H
