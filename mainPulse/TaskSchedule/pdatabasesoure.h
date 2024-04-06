#ifndef PDATABASESOURE_H
#define PDATABASESOURE_H

#include <QObject>
#include<QSql>
#include<QSqlDatabase>

class PDataBaseSoure : public QObject//管理所有的数据库连接
{
    Q_OBJECT
protected:

public:
    QSqlDatabase getMateConnect();
    QSqlDatabase getMateConnect(QString connectName);
    QSqlDatabase getDatabaseConnect(QString connectName,QString databaseType,QString hostName,quint16 port,QString databaseName,QString user,QString password,QStringList options = QStringList());
    static PDataBaseSoure *getSoure();
    qint64 getNextJob();
    QSqlDatabase getDatabaseConnect(QString connectName,qint64 connectId);
    QSqlDatabase getDatabaseConnect(QString connectName,QString name);
private:
    QString mateName;
    QSqlDatabase mateDB;
    QHash<QString,QString> *driverMap;//定义驱动映射
    void init();//初始化类
    explicit PDataBaseSoure(QObject *parent = nullptr);
    static bool isInit;
    static qint64 job_butch_pk ;
signals:
};

static PDataBaseSoure *pDataBaseSoure = PDataBaseSoure::getSoure();
#endif // PDATABASESOURE_H
