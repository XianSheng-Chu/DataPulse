#include "jobdbrunnable.h"
#include "pdatabasesoure.h"
#include<QSqlQuery>
#include<QSqlError>


JobDBRunnable::JobDBRunnable(qint64 jobId, const QHash<QString, QVariant> jobConnect, const QHash<QString, QVariant> jobRule, const QList<QHash<QString,QVariant>> jobMapper)
              :JobRunnable(jobId,jobConnect,jobRule,jobMapper)
{

}



void JobDBRunnable::run()
{
    connectInit();
    execJob();
    insertMapper();
}

void JobDBRunnable::connectInit()
{
    QString jobConnectName = QString::number(jobId)+"-cycle" + jobStartTime.toString("yyyy-MM-dd HH:mm:ss zzz");
    QString hostName = jobConnect.value("host_name").toString();
    QString databaseType = jobConnect.value("connect_sub_type").toString();
    quint16 port = jobConnect.value("connect_port").toUInt();
    QString databaseName = jobConnect.value("database_name").toString();
    QString user = jobConnect.value("user_name").toString();
    QString password = jobConnect.value("connect_password").toString();
    QStringList options = jobConnect.value("connect_options").toString().split(";");
    execConnect = pDataBaseSoure->getDatabaseConnect(jobConnectName,databaseType,hostName,port,databaseName,user,password,options);
    if(!execConnect.open()){
        QString erroe = execConnect.lastError().text();
        qDebug()<<erroe;
        isSuccess = false;
        Message = jobConnect.value("connect_name").toString()+erroe;
    }
}

JobDBRunnable::~JobDBRunnable()
{
    execConnect.close();
    QString connectionName = execConnect.connectionName();
    QSqlDatabase::removeDatabase(connectionName);
    qDebug()<<"remove " + connectionName;
    insertDB.close();
    QSqlDatabase::removeDatabase(insertDB.connectionName());
}

void JobDBRunnable::execJob()
{
    if(!isSuccess) return;
    QSqlQuery query(execConnect);
    QString script = jobRule.value("rule_script").toString();
    jobStartTime = QDateTime::currentDateTime();
    if(!query.exec(script)){
        QString erroe = query.lastError().text();
        qDebug()<<erroe+";"+script;
        isSuccess = false;
        Message = jobConnect.value("connect_name").toString()+erroe;
    }
    jobEndTime = QDateTime::currentDateTime();
    while(query.next()){
        result.append(query.record());
    }
}

void JobDBRunnable::insertMapper()
{
    if(!isSuccess) return;
    qint64 batchNum = pDataBaseSoure->getNextJob();
    batchRecord.insert("batch_number",batchNum);
    batchRecord.insert("rule_id",jobRule.value("rule_id"));
    batchRecord.insert("connect_id",jobConnect.value("connect_id"));
    batchRecord.insert("rule_name",jobRule.value("rule_name"));
    batchRecord.insert("connect_name",jobConnect.value("connect_name"));
    batchRecord.insert("start_time",jobStartTime);
    batchRecord.insert("end_time",jobEndTime);
    batchRecord.insert("row_count",result.count());
    batchRecord.insert("is_success",isSuccess?"Y":"N");
    batchRecord.insert("error_message",Message);
    batchRecord.insert("abnormal_flag","N");
    QString str = "INSERT INTO pulse_job_butch "
        "(batch_number, rule_name, connect_name, start_time, end_time, row_count, is_success, error_message, abnormal_flag, rule_id, connect_id) "
        "VALUES(:batch_number, :rule_name, :connect_name, :start_time, :end_time, :row_count, :is_success, :error_message, :abnormal_flag, :rule_id, :connect_id);";
    insertDB = pDataBaseSoure->getMateConnect(QString::number(jobId)+"-insert" + jobStartTime.toString("yyyy-MM-dd HH:mm:ss zzz"));
    if(!insertDB.isOpen()) insertDB.open();
    QSqlQuery query(insertDB);
    query.prepare(str);
    foreach (auto key, batchRecord.keys()) {
        query.bindValue(":"+key,batchRecord.value(key));
    }
    ;
    if(!query.exec()){

        str =  query.lastError().text();
        qDebug()<<query.lastError().text()<<query.lastQuery();
        return;
    }
    if((!isSuccess)||(result.count()==0)) {
        return;//如果执行失败或者没有查询出来数据，不进行值插入
    }
    QHash<QString,QString> colMapper;
    QString columns = "(batch_number, connect_id, connect_name, rule_id, rule_name,";
    QString values = "VALUES(:batch_number, :connect_id, :connect_name, :rule_id, :rule_name,";
    for (QHash<QString,QVariant> var : jobMapper) {
        columns = columns+var.value("target_column_name").toString()+",";
        values =values +":"+var.value("target_column_name").toString()+",";
        colMapper.insert(var.value("target_column_name").toString(),var.value("soure_column_name").toString());
    }
    columns+="start_time, end_time)";
    values+=":start_time,:end_time);";
    str = "INSERT INTO pulse_job_result" + columns + values;
    qDebug()<<str;

    for (QSqlRecord record : result) {
        query.prepare(str);
        query.bindValue(":batch_number",batchNum);
        query.bindValue(":connect_id",jobConnect.value("connect_id"));
        query.bindValue(":connect_name",jobConnect.value("connect_name"));
        query.bindValue(":rule_id",jobRule.value("rule_id"));
        query.bindValue(":rule_name",jobRule.value("rule_name"));
        for (QString targetColumn : colMapper.keys()) {
            if(jobRule.value("rule_id").toInt() == 2001){
                //qDebug()<<record.value(colMapper.value(targetColumn))<<targetColumn;
            }
            query.bindValue(":"+targetColumn,record.value(colMapper.value(targetColumn)));
        }
        query.bindValue(":start_time",jobStartTime);
        query.bindValue(":end_time",jobEndTime);
        if(!query.exec()){
            qDebug()<<query.lastQuery();
            qDebug()<<query.lastError().text();
        }
    }


}
