#ifndef JOBRUNNABLE_H
#define JOBRUNNABLE_H
#include <QObject>
#include <QRunnable>
#include<QHash>
#include<QDateTime>
class JobRunnable : public QRunnable
{

public:
    //JobRunnable();
    JobRunnable(qint64 jobId,const QHash<QString,QVariant> jobConnect,const QHash<QString,QVariant> jobRule,const QList<QHash<QString,QVariant>> jobMapper);
    // QRunnable interface
public:
    virtual void run() override = 0;
    virtual void connectInit() = 0;//定义Job的数据连接
    virtual void execJob() = 0;
    virtual void insertMapper() = 0;
protected:
    QHash<QString,QVariant> jobConnect;
    QHash<QString,QVariant> jobRule;
    QList<QHash<QString,QVariant>> jobMapper;
    qint64 jobId;

protected:
    QDateTime jobStartTime;
    QDateTime jobEndTime;
    bool isSuccess = true;
    QString Message;
};



#endif // JOBRUNNABLE_H
