#ifndef JOBDBRUNNABLE_H
#define JOBDBRUNNABLE_H

#include<QSqlDatabase>
#include "jobrunnable.h"
#include<QSqlRecord>

class JobDBRunnable : public JobRunnable
{
public:
    JobDBRunnable(qint64 jobId,const QHash<QString,QVariant> jobConnect,const QHash<QString,QVariant> jobRule,const QList<QHash<QString,QVariant>> jobMapper);
    ~JobDBRunnable();
    // QRunnable interface
public:
    virtual void run() override;

    // JobRunnable interface
private:
    virtual void connectInit() override;
    virtual void execJob() override;
    virtual void insertMapper() override;

private:
    QSqlDatabase execConnect;
    QSqlDatabase insertDB;
    QHash<QString,QVariant> batchRecord;
    QList<QSqlRecord> result;
};



#endif // JOBDBRUNNABLE_H
