#include "monitoringcyclethread.h"
#include<QSqlDatabase>
#include<QSql>
#include<QMessageBox>
#include<QSqlQuery>
#include<QSqlRecord>
#include<QDateTime>
#include<QSqlError>
#include"pdatabasesoure.h"
void MonitoringCycleThread::init()
{
    qDebug()<<"MonitoringCycleThread::init0";
    if(jobMonitoring!=nullptr) delete jobMonitoring;
    jobMonitoring = new QVector<QPair<quint64,QMap<QString,QVariant>>>;
    QSqlDatabase mateDB = pDataBaseSoure->getMateConnect();
    if(!mateDB.open())  QMessageBox::information(nullptr,"信息","mate数据库连接失败");
    QSqlQuery query(mateDB);
    QString str = "SELECT job_id, job_connection_id, job_connection_name, waiting_time_sec, job_rule_id, job_rule_name,suspend_flag,COALESCE(end_time,DATETIME()) FROM pulse_job_monitoring a left join"
                  "(SELECT  rule_id,max(end_time)  as end_time FROM pulse_job_butch group by rule_id) b on a.job_rule_id = b.rule_id";
    if(!query.exec(str)){
        QMessageBox::information(nullptr,"信息","调度信息加载失败,但你任然可以观看历史数据"+query.lastError().text());
        qDebug()<<query.lastError().text();
        stopThread();
    }
    QSqlRecord record = query.record();
    while (query.next()) {
        quint32 jobId = query.value(record.indexOf("job_id")).toUInt();
        QMap<QString,QVariant> jobInfo;
        for (int i = 1; i < record.count(); ++i) {
            jobInfo.insert(record.fieldName(i),query.value(i));
        }
        jobInfo.insert(QString(jobRunDateKey),QVariant(qMax(QDateTime::currentDateTime().addSecs(-1*jobInfo.value("waiting_time_sec").toInt()),
                                                             jobInfo.value("end_time").toDateTime())));
        QPair<quint32,QMap<QString,QVariant>> row(jobId,jobInfo);
        jobMonitoring->push_back(row);
    }
    qDebug()<<"MonitoringCycleThread::init";
}

void MonitoringCycleThread::cycleJob()
{

    m_isStop = false;
    while (!m_isStop) {
        QDateTime nextSecond = QDateTime::currentDateTime().addSecs(1);
        for (int i = 0; i < jobMonitoring->count(); ++i) {
            QMutexLocker lock(&mutex);
            QPair<quint32,QMap<QString,QVariant>> job = (jobMonitoring->at(i));
            if(job.second.value("suspend_flag")=="Y") continue;
            qint64 cycle = job.second.value("waiting_time_sec").toInt();//取出job运行周期
            QDateTime curTime = QDateTime::currentDateTime();
            if(curTime<=job.second.value(jobRunDateKey).toDateTime().addSecs(cycle)) continue;//如果当前时间小于上次运行时间+job运行时间，跳过本次循环
            emit runJob(job.first,job.second.value("job_connection_id").toInt(),job.second.value("job_rule_id").toInt());
            job.second.insert(jobRunDateKey,job.second.value(jobRunDateKey).toDateTime().addSecs(cycle));
            jobMonitoring->replace(i,job);
        }
        nextSecond.setMSecsSinceEpoch(nextSecond.toMSecsSinceEpoch() / 1000 * 1000);//设置为一秒的最开始
        int sleepMsec = nextSecond.toMSecsSinceEpoch() - QDateTime::currentDateTime().toMSecsSinceEpoch()-1;

        msleep(qMax(sleepMsec,1));//休眠线程直到下一秒开始


    }
}

void MonitoringCycleThread::doUpdateJob(quint64 jobId)
{
    QMutexLocker lock(&mutex);
    QSqlDatabase mateDB = pDataBaseSoure->getMateConnect();
    if(!mateDB.open())  QMessageBox::information(nullptr,"信息","mate数据库连接失败");
    QSqlQuery query(mateDB);
    QString str = "SELECT job_id, job_connection_id, job_connection_name, waiting_time_sec, job_rule_id, job_rule_name,suspend_flag,COALESCE(end_time,DATETIME()) FROM pulse_job_monitoring a left join"
                  "(SELECT  rule_id,max(end_time)  as end_time FROM pulse_job_butch group by rule_id) b on a.job_rule_id = b.rule_id and a.job_id = :job_id";
    query.prepare(str);
    query.bindValue(":job_id",jobId);


    if(!query.exec()){
        QMessageBox::information(nullptr,"信息","调度信息加载失败,但你任然可以观看历史数据"+query.lastError().text());
        qDebug()<<query.lastError().text();
        stopThread();
    }
    if(query.next()){
        QSqlRecord record = query.record();
        quint32 jobId = query.value(record.indexOf("job_id")).toUInt();
        QMap<QString,QVariant> jobInfo;
        for (int i = 1; i < record.count(); ++i) {
            jobInfo.insert(record.fieldName(i),query.value(i));
        }
        jobInfo.insert(QString(jobRunDateKey),QVariant(qMax(QDateTime::currentDateTime().addSecs(-1*jobInfo.value("waiting_time_sec").toInt()),
                                                             jobInfo.value("end_time").toDateTime())));
        QPair<quint32,QMap<QString,QVariant>> row(jobId,jobInfo);
        for (int i = 0; i < jobMonitoring->count(); ++i) {
            if(jobId == jobMonitoring->at(i).first) jobMonitoring->remove(i);
        }
        jobMonitoring->push_back(row);
    }else{
        for (int i = 0; i < jobMonitoring->count(); ++i) {
            if(jobId == jobMonitoring->at(i).first) jobMonitoring->remove(i);
        }
    }
}

MonitoringCycleThread::MonitoringCycleThread(QObject *parent)
    : QThread{parent}
{
    init();
}

void MonitoringCycleThread::stopThread()
{
    m_isStop = true;
}

MonitoringCycleThread::~MonitoringCycleThread()
{
    delete jobMonitoring;
}

void MonitoringCycleThread::run()
{
    cycleJob();
}
