#ifndef MONITORINGCYCLETHREAD_H
#define MONITORINGCYCLETHREAD_H

#include <QObject>
#include <QThread>
#include<QMap>
#include<QMutex>
class MonitoringCycleThread : public QThread
{
    Q_OBJECT
private:
   bool m_isStop = false;
    const QString jobRunDateKey = "last_run_date";//用以在map中指向下一次运行时间的key值
    QVector<QPair<quint64,QMap<QString,QVariant>>> *jobMonitoring = nullptr;//事件循环结构，用于储存需要调度的所有任务
public:
    explicit MonitoringCycleThread(QObject *parent = nullptr);
    void stopThread();
    ~MonitoringCycleThread();
    // QThread interface
protected:
    virtual void run() override;

private:
    void init();//初始化事件循环
    void cycleJob();//执行job循环
    QMutex mutex;
signals:
    void runJob(quint64 jobId,quint64 connectId = -1,quint64 ruleId = -1);

public slots:
    void doUpdateJob(quint64 jobId);
};



#endif // MONITORINGCYCLETHREAD_H
