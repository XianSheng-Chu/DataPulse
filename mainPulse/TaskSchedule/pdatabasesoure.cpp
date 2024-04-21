#include "pdatabasesoure.h"

#include<QMessageBox>
#include<QSqlError>
#include<QSqlQuery>
#include<QSqlRecord>
#include<QApplication>
bool PDataBaseSoure::isInit = false;
qint64 PDataBaseSoure::job_butch_pk = -1;
PDataBaseSoure::PDataBaseSoure(QObject *parent)
    : QObject{parent}
{
    isInit = true;
    //init();
}

QSqlDatabase PDataBaseSoure::getMateConnect()
{
    QSqlDatabase MateDB = QSqlDatabase::database(mateName,false);

    return MateDB;
}

QSqlDatabase PDataBaseSoure::getMateConnect(QString connectName)
{
    return QSqlDatabase::cloneDatabase(mateName,connectName);
}

QSqlDatabase PDataBaseSoure::getDatabaseConnect(QString connectName,QString databaseType, QString hostName, quint16 port,
                                                QString databaseName, QString user,QString password,
                                                QStringList options)
{
    qDebug()<<driverMap->value(databaseType);
    QSqlDatabase connect = QSqlDatabase::addDatabase(driverMap->value(databaseType),connectName);
    connect.setHostName(hostName);
    connect.setPort(port);
    connect.setDatabaseName(databaseName);
    connect.setUserName(user);
    connect.setPassword(password);
    for (int i = 0; i < options.count(); ++i) {
        connect.setConnectOptions(options.at(i));
    }
    return connect;
}

PDataBaseSoure *PDataBaseSoure::getSoure()
{
    if(!isInit)  pDataBaseSoure =  new PDataBaseSoure();
    return pDataBaseSoure;

}

qint64 PDataBaseSoure::getNextJob()
{
    int nextVal = ++job_butch_pk;
    return nextVal;
}

QSqlDatabase PDataBaseSoure::getDatabaseConnect(QString connectName, qint64 connectId)
{

    QSqlQuery query(mateDB);
    QString str = "SELECT connect_id, connect_name, connect_type, connect_sub_type, database_name, user_name, connect_password, host_name, connect_port, connect_options, del_flag, object_version, privateKey"
                  " FROM pulse_connect_info where connect_id = :connect_id;";
    query.prepare(str);
    query.bindValue(":connect_id",connectId);
    query.exec(str);
    bool next = query.next();
    QSqlRecord record = query.record();
    qDebug()<<query.lastError().text();
    if(next){
        QSqlRecord record = query.record();
        QString ConnectName = connectName;
        QString hostName = record.value("host_name").toString();
        QString databaseType = record.value("connect_sub_type").toString();
        quint16 port = record.value("connect_port").toUInt();
        QString databaseName = record.value("database_name").toString();
        QString user = record.value("user_name").toString();
        QString password = record.value("connect_password").toString();
        QStringList options = record.value("connect_options").toString().split(";");
        return getDatabaseConnect(ConnectName,databaseType,hostName,port,databaseName,user,password,options);
    }else{
        qDebug()<<query.lastQuery();
        qDebug()<<query.lastError();
        qDebug()<<connectId;
    }
}

QSqlDatabase PDataBaseSoure::getDatabaseConnect(QString connectName, QString name)
{
    QSqlQuery query(mateDB);
    QString str = "SELECT connect_id, connect_name, connect_type, connect_sub_type, database_name, user_name, connect_password, host_name, connect_port, connect_options, del_flag, object_version, privateKey"
                  " FROM pulse_connect_info where connect_name = :connect_name;";
    query.prepare(str);
    query.bindValue(":connect_name",name);
    query.exec();
    bool next = query.next();
    QSqlRecord record = query.record();
    qDebug()<<query.lastError().text();
    if(next){
        QSqlRecord record = query.record();
        QString ConnectName = connectName;
        QString hostName = record.value("host_name").toString();
        QString databaseType = record.value("connect_sub_type").toString();
        quint16 port = record.value("connect_port").toUInt();
        QString databaseName = record.value("database_name").toString();
        QString user = record.value("user_name").toString();
        QString password = record.value("connect_password").toString();
        QStringList options = record.value("connect_options").toString().split(";");
        return getDatabaseConnect(ConnectName,databaseType,hostName,port,databaseName,user,password,options);
    }
}

void PDataBaseSoure::init()
{

    mateName = "Mate";
    QString DBFile = QCoreApplication::applicationDirPath()+"/dataBase/pulse.db";
    qDebug()<<DBFile;
    QSqlDatabase DB = QSqlDatabase::addDatabase("QSQLITE",mateName);
    DB.setDatabaseName(DBFile);
    if(!DB.open()){
        QString str = DB.lastError().text();
        QMessageBox::information(nullptr,"信息","数据库连接初始化失败"+DB.lastError().text());
    }


    driverMap = new QHash<QString,QString>;
    driverMap->insert("MySql","QMYSQL");
    driverMap->insert("PostgreSql","QPSQL");
    mateDB = DB;
    QSqlQuery query(DB);
    query.exec("select max(batch_number) as batch_number from  pulse_job_butch");

    if(query.next()){
        job_butch_pk = query.record().value("batch_number").toInt();
    }else{
        qDebug()<<query.lastError().text();
        job_butch_pk = 1;
    }


}


