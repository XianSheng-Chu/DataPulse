#include "pstandarditemmodel.h"
#include "TaskSchedule/pdatabasesoure.h"
#include<QSqlQuery>
#include<QSqlRecord>
#include<QSqlError>
PStandardItemModel::PStandardItemModel(QWidget *parent):QStandardItemModel(parent) {

    mateData = new QHash<qint64,QHash<QString,QVariant>>;
}

PStandardItemModel::PStandardItemModel(int rows, int columns, QObject *parent):QStandardItemModel(rows,columns,parent)
{
    mateData = new QHash<qint64,QHash<QString,QVariant>>;

}

PStandardItemModel::~PStandardItemModel()
{
    if(mateDB.isOpen()) mateDB.close();
    QSqlDatabase::removeDatabase(mateDB.connectionName());
}


void PStandardItemModel::init(int index)
{

    this->clear();
    if(mateData!=nullptr) delete mateData;
    mateData = new QHash<qint64,QHash<QString,QVariant>>;
    widgetIndex= index;
    mateDB = pDataBaseSoure->getMateConnect("Mate-Tree"+idName);
    idName = widgetIndex==0?"connect_id":"rule_id";
    if(!mateDB.isOpen()) mateDB.open();
    QSqlQuery query(mateDB);
    QString str;
    if(index == 0){
        str = "SELECT connect_id, connect_name, connect_type, connect_sub_type, database_name, user_name, connect_password, host_name, connect_port, connect_options, del_flag, object_version, privateKey"
              "  FROM pulse_connect_info;";


    }else if(index == 1){
        str = "SELECT rule_id, rule_name, rule_connect_type, rule_connect_sub_type, rule_script, rule_is_test "
              "  FROM pulse_rule_info;";

    }

    if(!query.exec(str)) return;
    while (query.next()) {
        QSqlRecord record = query.record();
        qint64 id = record.value(idName).toInt();
        QHash<QString,QVariant> row ;
        for (int i = 0; i < record.count(); ++i) {
            row.insert(record.fieldName(i),record.value(i));
        }
        mateData->insert(id,row);
    }

    setModelDim();

}

void PStandardItemModel::setModelDim()
{
    QStringList dim;
    if(widgetIndex ==0){
        dim<<"connect_sub_type"<<"host_name";
    }else{
        dim<<"rule_connect_type"<<"rule_connect_sub_type";
    }

    setModelDim(dim);
}

void PStandardItemModel::setModelDim(QStringList dim)
{
    m_dim.clear();
    m_dim.append(dim);
    this->clear();
    if(widgetIndex ==0){
        dim<<"connect_name";
    }else{
        dim<<"rule_name";
    }
    //this->setColumnCount(dim.count());
    //this->setHorizontalHeaderLabels(dim);


    QStandardItem *rootItem = invisibleRootItem();
    QSqlQuery query(mateDB);
    QString queryJob = "SELECT job_id, job_connection_id, job_connection_name, waiting_time_sec, job_rule_id, job_rule_name,suspend_flag,create_date,update_date,job_comment,del_flag "
                       " FROM pulse_job_monitoring  where job_connection_id = :job_connection_id;";




    foreach (auto row, *mateData) {
        QStandardItem *item = rootItem;
        for (int i = 0; i < dim.count(); ++i) {
            QStandardItem *nextItem = new QStandardItem;
            nextItem->setData(row.value(dim.at(i)).toString(),Qt::DisplayRole);
            bool insertFlag = true;
            for (int i = 0; i < item->rowCount(); ++i) {
                if(item->child(i)->data(Qt::DisplayRole).toString()==nextItem->data(Qt::DisplayRole).toString()){
                    nextItem = item->child(i);
                    insertFlag = false;
                    break;
                }
            }
            if(insertFlag) {
                item->appendRow(nextItem);
                qDebug()<<nextItem->data(Qt::DisplayRole).toString();
            }
            item = nextItem;

        }
        item->setData(row.value(idName).toInt(),Qt::UserRole+1);//存入主键
        item->setData(idName,Qt::UserRole+2);//存入item类型
        if(widgetIndex==1) continue;//如果是规则导航，那么不再将job写入模型

        query.prepare(queryJob);
        query.bindValue(":job_connection_id",row.value(idName));
        query.exec();
        while (query.next()) {
            QSqlRecord record = query.record();
            QString jobName = record.value("job_rule_name").toString();
            QStandardItem* jobItem = new QStandardItem();
            jobItem->setText(jobName);
            jobItem->setData(record.value("job_id"),Qt::UserRole+1);//存入JobId
            jobItem->setData("job_id",Qt::UserRole+2);//存入item类型
            jobItem->setData(0,Qt::UserRole+3);//存入item高度
            item->appendRow(jobItem);
            jobItem->setTextAlignment(Qt::AlignTop|Qt::AlignLeft);
        }

    }
}

void PStandardItemModel::traverseTree(qint64 id, QString idType,qint64 level, bool deleteFlag)
{
    if(deleteFlag == true){
    }
}

void PStandardItemModel::do_updateConnect(qint64 id)
{
    init(widgetIndex);
}

void PStandardItemModel::do_updateRule(qint64 id)
{
    init(widgetIndex);
}

void PStandardItemModel::do_updateJob(qint64 id)
{
    init(widgetIndex);
}
