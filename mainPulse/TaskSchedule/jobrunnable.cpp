#include "jobrunnable.h"
#include "qvariant.h"

//JobRunnable::JobRunnable() {}

JobRunnable::JobRunnable(qint64 jobId, const QHash<QString, QVariant> jobConnect, const QHash<QString, QVariant> jobRule, QList<QHash<QString,QVariant>> jobMapper)
{
    this->jobId = jobId;
    this->jobConnect = jobConnect;
    this->jobRule = jobRule;
    this->jobMapper = jobMapper;
    this->jobStartTime = QDateTime::currentDateTime();
}
