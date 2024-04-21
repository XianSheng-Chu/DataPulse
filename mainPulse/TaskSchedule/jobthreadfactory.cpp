#include "jobthreadfactory.h"
#include "pdatabasesoure.h"
#include "jobdbrunnable.h"

#include<QSqlQuery>
#include<QSqlRecord>
#include<QMessageBox>
#include<QSqlError>
#include <QRunnable>
#include<QThreadPool>
#include<QMutexLocker>
JobThreadFactory::JobThreadFactory(QObject *parent)
    : QObject{parent}
{
    qDebug()<<"JobThreadFactory";
}

 void JobThreadFactory::init()
{
    QMutexLocker lock(&mutex);
    mateDB = pDataBaseSoure->getMateConnect(JobFactoryDB);
    if(!mateDB.isOpen()) mateDB.open();
    initConnects();
    initRules();
    initMappers();
    qDebug()<<"test JobThreadFactory init";
    mateDB.close();
    qDebug()<<"JobThreadFactory::init";
}

void JobThreadFactory::initConnects()
{
    if(connectInfos!=nullptr) delete connectInfos;
    connectInfos = new QMap<qint64,QHash<QString,QVariant>>;
    if(!mateDB.isOpen()) mateDB.open();
    QSqlQuery query(mateDB);
    QString str = "SELECT connect_id, connect_name, connect_type, connect_sub_type, database_name, user_name, connect_password, host_name, connect_port, connect_options, del_flag, object_version,privateKey "
                  "FROM pulse_connect_info;";
    query.exec(str);
    if(query.lastError().isValid()){
        return;
    }
    QSqlRecord record = query.record();
    while (query.next()) {
        qint64 connectId = query.value(record.indexOf("connect_id")).toUInt();

        QHash<QString,QVariant> connectInfo;
        for (int i = 0; i < record.count(); ++i) {
            connectInfo.insert(record.fieldName(i),query.value(i));
        }
        connectInfos->insert(connectId,connectInfo);
    }
}

void JobThreadFactory::initRules()
{
    if(rules!=nullptr) delete rules;
    rules = new QMap<qint64,QHash<QString,QVariant>>;
    if(!mateDB.isOpen()) mateDB.open();
    QSqlQuery query(mateDB);
    QString str = "SELECT rule_id, rule_name, rule_connect_type, rule_connect_sub_type, rule_script, rule_is_test "
                  " FROM pulse_rule_info;";
    query.exec(str);
    if(query.lastError().isValid()){
        QString erroeMessage = mateDB.lastError().text();
        return;
    }
    QSqlRecord record = query.record();
    while (query.next()) {
        qint64 ruleId = query.value(record.indexOf("rule_id")).toUInt();
        QHash<QString,QVariant> ruleInfo;
        for (int i = 0; i < record.count(); ++i) {
            ruleInfo.insert(record.fieldName(i),query.value(i));
        }
        rules->insert(ruleId,ruleInfo);
    }
}

void JobThreadFactory::initMappers()
{
    if(insertMappers!=nullptr) delete insertMappers;
    insertMappers = new QMap<qint64,QList<QHash<QString,QVariant>>>;
    if(!mateDB.isOpen()) mateDB.open();
    QSqlQuery query(mateDB);
    QString str = "SELECT rule_id, soure_column_name, target_column_name, is_check, condition_numeric, condition_text, condition_name "
                   " FROM pulse_insert_mappers;";
    query.exec(str);
    if(query.lastError().isValid()){
        QString erroeMessage = mateDB.lastError().text();
        return;
    }
    QSqlRecord record = query.record();
    while (query.next()) {
        qint64 ruleId = query.value(record.indexOf("rule_id")).toUInt();
        QList<QHash<QString,QVariant>> mappers;
        if(insertMappers->count(ruleId)!=0) mappers = insertMappers->value(ruleId);
        QHash<QString,QVariant> row;
        for (int i = 0; i < record.count(); ++i) {
            row.insert(record.fieldName(i),query.value(i));
        }
        mappers.append(row);
        insertMappers->insert(ruleId,mappers);
    }
}

void JobThreadFactory::jobRunnbaleFactory(quint64 jobId, quint64 connectId, quint64 ruleId)
{
    qDebug()<<"JobThreadFactory::jobRunnbaleFactory0";
    QMutexLocker lock(&mutex);

    if("Database"==connectInfos->value(connectId).value("connect_type").toString()){
        QRunnable *dbRunnable = new JobDBRunnable(jobId,connectInfos->value(connectId),rules->value(ruleId),insertMappers->value(ruleId));
        dbRunnable->setAutoDelete(true);
        QThreadPool::globalInstance()->start(dbRunnable);
    }else if("OS" == connectInfos->value(connectId).value("connect_type").toString()){
        //ssh任务保留位置
    }
}

void JobThreadFactory::do_updateConnect(qint64 connectId)
{
    QMutexLocker lock(&mutex);
    if(!mateDB.isOpen()) mateDB.open();
    QSqlQuery query(mateDB);
    QString str = "SELECT connect_id, connect_name, connect_type, connect_sub_type, database_name, user_name, connect_password, host_name, connect_port, connect_options, del_flag, object_version,privateKey "
                  "FROM pulse_connect_info where connect_id = :connect_id;";

    query.prepare(str);
    query.bindValue(":connect_id",connectId);
    query.exec();
    if(query.lastError().isValid()){
        return;
    }
    QSqlRecord record = query.record();
    if (query.next()) {
        qint64 connectId = query.value(record.indexOf("connect_id")).toUInt();
        QHash<QString,QVariant> connectInfo;
        for (int i = 0; i < record.count(); ++i) {
            connectInfo.insert(record.fieldName(i),query.value(i));
        }
        connectInfos->insert(connectId,connectInfo);
    }else{
        connectInfos->remove(connectId);//没有查到就从内存移除这一条连接信息
    }
}

void JobThreadFactory::do_updateRule(qint64 ruleId)
{
    QMutexLocker lock(&mutex);
    if(!mateDB.isOpen()) mateDB.open();
    QSqlQuery query(mateDB);
    QString str = "SELECT rule_id, rule_name, rule_connect_type, rule_connect_sub_type, rule_script, rule_is_test "
                  "  FROM pulse_rule_info where rule_id = :rule_id;";

    query.prepare(str);
    query.bindValue(":rule_id",ruleId);
    query.exec();
    if(query.lastError().isValid()){
        return;
    }
    QSqlRecord record = query.record();
    if (query.next()) {
        qint64 ruleId = query.value(record.indexOf("rule_id")).toUInt();
        QHash<QString,QVariant> connectInfo;
        for (int i = 0; i < record.count(); ++i) {
            connectInfo.insert(record.fieldName(i),query.value(i));
        }
        rules->insert(ruleId,connectInfo);


        str = "SELECT rule_id, soure_column_name, target_column_name, is_check, condition_numeric, condition_text, condition_name "
              " FROM pulse_insert_mappers where rule_id = :rule_id;";
        query.prepare(str);
        query.bindValue(":rule_id",ruleId);
        query.exec();
        if(query.lastError().isValid()){
            QString erroeMessage = query.lastError().text();
            qDebug()<<erroeMessage;
            qDebug()<<query.lastQuery();

            return;
        }
        QSqlRecord record = query.record();
        QList<QHash<QString,QVariant>> mappers;
        insertMappers->insert(ruleId,mappers);
        while(query.next()) {
            QHash<QString,QVariant> row;
            if(insertMappers->count(ruleId)!=0) mappers = insertMappers->value(ruleId);
            for (int i = 0; i < record.count(); ++i) {
                row.insert(record.fieldName(i),query.value(i));
            }
            mappers.append(row);
            insertMappers->insert(ruleId,mappers);
        }

    }else{
        rules->remove(ruleId);//没有查到就从内存移除这一条连接信息
        insertMappers->remove(ruleId);
    }
}

JobThreadFactory* JobThreadFactory::jobFactory = new JobThreadFactory;

