#ifndef JOBTHREADFACTORY_H
#define JOBTHREADFACTORY_H


#include <QObject>
#include<QHash>
#include<QSqlDatabase>
#include<QMutex>
class JobThreadFactory;

class JobThreadFactory : public QObject
{
    Q_OBJECT
public:
     static JobThreadFactory *jobFactory;//JobThreadFactory在全局只有一个实例可用
     void init();
private:
    QMap<qint64,QHash<QString,QVariant>> *connectInfos = nullptr;//所有连接
    QMap<qint64,QHash<QString,QVariant>> *rules = nullptr;//所有规则
    QMap<qint64,QList<QHash<QString,QVariant>>> *insertMappers = nullptr;//插入结果表的字段mapper
    QSqlDatabase mateDB;
    const QString JobFactoryDB = "FactoryDB";
    QMutex mutex;

private:
    explicit JobThreadFactory(QObject *parent = nullptr);

    void initConnects();
    void initRules();
    void initMappers();

signals:

public slots:
    void jobRunnbaleFactory(quint64 jobId,quint64 connectId = -1,quint64 ruleId = -1);
    void do_updateConnect(qint64 connectId);
    void do_updateRule(qint64 ruleId);
};

#endif // JOBTHREADFACTORY_H
