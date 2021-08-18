#ifndef MYLOGGER_H
#define MYLOGGER_H

#include <QObject>
#include <QThread>
#include <QtGlobal>

//#undef THREAD_USE_WORKER
#define THREAD_USE_WORKER

class MyLogger : public QObject
{
    Q_OBJECT
public:
    explicit MyLogger(QObject *parent = Q_NULLPTR);
    ~MyLogger();

    static MyLogger *instance();
    bool isLogging() const;
    void startLogging();
    void suspendLogging();
    void setLogFlushSize(quint64 size);

signals:
  #ifdef THREAD_USE_WORKER
    void sigWriteLogStr(const QString &str);
    void sigSetLogFlushSize(quint64 size);
  #endif

public:
    static void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
  #ifdef THREAD_USE_WORKER
    static MyLogger *_pThis;
    QThread workerThread;
  #endif

private:
  #ifdef THREAD_USE_WORKER
    bool mThreadInit;
  #endif
};

#endif // MYLOGGER_H
