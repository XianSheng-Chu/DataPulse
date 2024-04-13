#include "pcreateruledialog.h"
#include<QSqlQuery>
#include "ui_pcreateruledialog.h"
#include<QDateTime>
#include<qsqlrecord.h>
#include<QSqlError>
PCreateRuleDialog::PCreateRuleDialog(QWidget *parent, qint64 connectId)
    : QDialog(parent)
    , ui(new Ui::PCreateRuleDialog)
{
    ui->setupUi(this);
    metaDB = pDataBaseSoure->getMateConnect("PCreateRuleDialog"+QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()));
    m_connectId = connectId;
    init();
    ruleTestThread = new RuleTestThread(m_columnMapper);
    connect(ruleTestThread,&RuleTestThread::testFlag,this,&PCreateRuleDialog::do_TestFlag);
}

PCreateRuleDialog::PCreateRuleDialog(qint64 ruleId, QWidget *parent):PCreateRuleDialog(parent)
{
    updateFlag=true;
    QSqlQuery query(metaDB);

    m_ruleId = ruleId;
    QString str = "SELECT rule_id, rule_name, rule_connect_type, rule_connect_sub_type, rule_script, rule_is_test, rule_comment, create_date, update_date, del_flag, public_flag, test_connect_id "
                " FROM pulse_rule_info where rule_id=:rule_id;";
    query.prepare(str);
    query.bindValue(":rule_id",m_ruleId);
    if(!query.exec()){
        qDebug()<<query.lastError();
        qDebug()<<query.lastQuery();
    }
    if(query.next()){
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); ++i) {
            row.insert(record.fieldName(i),record.value(i));
        }
        ui->editName->setText(row.value("rule_name").toString());
        if(row.value("public_flag").toString()=="Y"){
            ui->radioTrue->setChecked(true);
        }else{
            ui->radioTrue->setCheckable(true);
        }
        ui->editScript->clear();
        ui->editScript->appendPlainText(row.value("rule_script").toString());
        ui->editCommect->appendPlainText(row.value("rule_comment").toString());

        if(row.value("test_connect_id").toInt()!=0){
            m_connectId = row.value("test_connect_id").toInt();
            connectInit();
        }
    }

    query.prepare("select connect_type, connect_sub_type,connect_name from  pulse_connect_info where connect_id = :connect_id");
    query.bindValue(":connect_id",m_connectId);
    query.exec();
    if(!query.exec()){
        qDebug()<<query.lastError();
        qDebug()<<query.lastQuery();
    }
    query.next();
    ui->combDBType->setCurrentText(query.record().value("connect_sub_type").toString());
    QString connectName = query.record().value("connect_name").toString();
    ui->combConnectName->setCurrentText(connectName);


}



PCreateRuleDialog::~PCreateRuleDialog()
{
    delete ui;
    QSqlDatabase::removeDatabase(metaDB.connectionName());
    ruleTestThread->terminate();
    ruleTestThread->wait();
}


void PCreateRuleDialog::init()
{
    if(!metaDB.isOpen()) metaDB.open();

    QSqlQuery query(metaDB);
    QString str = "SELECT DISTINCT  connect_sub_type FROM pulse_connect_info where connect_type = 'Database' order by 1;";
    query.exec(str);
    while (query.next()) {
        DBTypes.append(query.record().value("connect_sub_type").toString());
    }
    ui->combDBType->addItems(DBTypes);
    ui->combDBType->setCurrentIndex(0);
    str = "SELECT DISTINCT  connect_sub_type,connect_name FROM pulse_connect_info where connect_type = 'Database' order by 1;";
    query.exec(str);
    while (query.next()) {
        QString DBType = query.record().value("connect_sub_type").toString();
        if(DBNames.count(DBType)==0) DBNames.insert(DBType,QStringList());
        QStringList list = DBNames.value(DBType);
        list.append(query.record().value("connect_name").toString());
        DBNames.insert(DBType,list);
        qDebug()<<query.record().value("connect_name").toString();
    }
    ui->combConnectName->addItems(DBNames.value(ui->combDBType->currentText()));
    connectInit();
}

void PCreateRuleDialog::connectInit()
{

    if(m_connectId==-1) return;
    QSqlQuery query(metaDB);
    QString str;
    str = "SELECT connect_id, connect_name, connect_type, connect_sub_type, database_name, user_name, connect_password, host_name, connect_port, connect_options, del_flag, object_version, privateKey"
          " FROM pulse_connect_info where connect_id = :connect_id;";
    query.prepare(str);
    query.bindValue(":connect_id",m_connectId);
    query.exec(str);
    if(query.next()){
        ui->combDBType->setCurrentText(query.record().value("connect_sub_type").toString());
        ui->combConnectName->clear();
        ui->combConnectName->addItems(DBNames.value(ui->combDBType->currentText()));
        ui->combConnectName->setCurrentText(query.record().value("connect_name").toString());
    }
}



void PCreateRuleDialog::do_TestFlag(bool flag, QString info)
{
    if(flag){
       ui->labelStatus->setText("sql执行成功");
       testFlag = true;
    }else{
       ui->labelStatus->setText("数据库打开失败 info:"+info);
       testFlag = false;
    }
    ui->btnRun->setEnabled(true);
}


void PCreateRuleDialog::on_combDBType_currentIndexChanged(int index)
{
    ui->combConnectName->clear();
    ui->combConnectName->addItems(DBNames.value(ui->combDBType->currentText()));
    ui->combConnectName->setCurrentIndex(0);
}


void PCreateRuleDialog::on_editName_editingFinished()
{
    QSqlQuery query(metaDB);
    QString str = "select count(1) as count from pulse_rule_info where rule_name = :rule_name";
    query.prepare(str);
    query.bindValue(":rule_name",ui->editName->text());
    query.exec();
    query.next();
    QPalette palette = ui->editName->palette();
    if(query.record().value("count").toInt()>(updateFlag?1:0)){
        palette.setColor(QPalette::Text,Qt::red);
        ui->editName->setPalette(palette);
        nameTestFlag = false;
        ui->editName->setToolTip("规则名称已经存在");
    }else{
        palette.setColor(QPalette::Text,Qt::black);
        ui->editName->setPalette(palette);
        nameTestFlag = true;
        ui->editName->setToolTip("规则名称可用");
    }

}


void PCreateRuleDialog::on_btnRun_clicked()
{
    QString connectName = ui->combConnectName->currentText();
    if(connectName.isEmpty()){
        ui->labelStatus->setText("未选择数据库连接");
        return;
    }
    testDB = pDataBaseSoure->getDatabaseConnect(connectName+"test-rule",connectName);
    script = ui->editScript->toPlainText();
    ui->labelStatus->setText("sql正在执行，请稍后...");
    ui->btnRun->setEnabled(false);
    ruleTestThread->setAll(testDB,script);
    ruleTestThread->start();
}

void PCreateRuleDialog::on_editScript_textChanged()
{
    testFlag = false;
}

void PCreateRuleDialog::on_combConnectName_currentTextChanged(const QString &arg1)
{
    testFlag = false;
}


void PCreateRuleDialog::on_btnOK_clicked()
{
    if(!testFlag){
        ui->labelStatus->setText("请运行sql以测试");
        return;
    }
    if(!nameTestFlag){
        ui->labelStatus->setText("任务名已存在");
        return;
    }

    if(ui->editName->text().isEmpty()){
        ui->labelStatus->setText("请填写任务名");
        return;
    }

    row.insert("rule_name",ui->editName->text());
    row.insert("rule_connect_type","Database");
    row.insert("rule_connect_sub_type",ui->combDBType->currentText());
    row.insert("rule_script",ui->editScript->toPlainText());
    row.insert("rule_is_test",testFlag?"Y":"N");
    row.insert("rule_comment",ui->editCommect->toPlainText());
    row.insert("del_flag","N");
    row.insert("connect_name",ui->combConnectName->currentText());

    row.insert("public_flag",ui->radioTrue->isChecked()?"Y":"N");
    QSqlQuery query(metaDB);
    QString str = "select connect_id from pulse_connect_info where connect_name = :connect_name";
    query.prepare(str);
    query.bindValue(":connect_name",ui->combConnectName->currentText());
    query.exec();
    query.next();
    qint64 connectId =  query.record().value("connect_id").toInt();
    row.insert("test_connect_id",connectId);
    if(updateFlag){
        updateRule();
    }else{
        createRule();
    }

    str = "DELETE FROM pulse_insert_mappers WHERE rule_id=:rule_id;";
    query.prepare(str);
    query.bindValue(":rule_id",m_ruleId);
    query.exec();

    str = "INSERT INTO pulse_insert_mappers"
          "(rule_id, soure_column_name, target_column_name) VALUES(:rule_id, :soure_column_name, :target_column_name);";

    for (QString var : m_columnMapper.keys()) {
        query.prepare(str);
        query.bindValue(":rule_id",m_ruleId);
        query.bindValue(":soure_column_name",var);
        query.bindValue(":target_column_name",m_columnMapper.value(var));
        if(!query.exec()){
            qDebug()<<query.lastError().text();
            qDebug()<<query.lastQuery();
        }
    }
    emit putRuleId(m_ruleId);
    accept();
    emit do_close();

}


void PCreateRuleDialog::createRule()
{
    row.insert("create_date",QDateTime::currentDateTime());
    row.insert("update_date",QDateTime::currentDateTime());

    QString str = "INSERT INTO pulse_rule_info"
                  "( rule_name, rule_connect_type, rule_connect_sub_type, rule_script, rule_is_test, rule_comment, create_date, update_date, del_flag, public_flag, test_connect_id)"
                  "VALUES( :rule_name, :rule_connect_type, :rule_connect_sub_type, :rule_script, :rule_is_test, :rule_comment, :create_date, :update_date, :del_flag, :public_flag, :test_connect_id);";
    QSqlQuery query(metaDB);
    query.prepare(str);
    foreach (auto var, row.keys()) {
        query.bindValue(":"+var,row.value(var));
    }
    query.exec();

    str = "select rule_id from pulse_rule_info where rule_name = :rule_name";
    query.prepare(str);
    query.bindValue(":rule_name",row.value("rule_name"));
    query.exec();
    query.next();
    m_ruleId = query.record().value("rule_id").toInt();
}

void PCreateRuleDialog::updateRule()
{
    row.insert("update_date",QDateTime::currentDateTime());
    row.insert("rule_id",m_ruleId);

    QString str = "UPDATE pulse_rule_info "
        " SET rule_name=:rule_name, rule_connect_type=:rule_connect_type, rule_connect_sub_type=:rule_connect_sub_type, rule_script=:rule_script, rule_is_test=:rule_is_test, rule_comment=:rule_comment, create_date=:create_date, update_date=:update_date, del_flag=:del_flag, public_flag=:public_flag, test_connect_id=:test_connect_id "
        " WHERE rule_id=:rule_id;";
    QSqlQuery query(metaDB);
    query.prepare(str);
    foreach (auto var, row.keys()) {
        query.bindValue(":"+var,row.value(var));
    }
    query.exec();
    qDebug()<<query.lastError().text();
}

void PCreateRuleDialog::on_btnClose_clicked()
{
    emit do_close();
}




RuleTestThread::RuleTestThread(QHash<QString,QString> &columnMapper,QObject *parent):QThread(parent),m_columnMapper(columnMapper)
{

}

void RuleTestThread::setAll(QSqlDatabase DB, QString script)
{
    m_testDB = DB;
    m_script = script;
}


void RuleTestThread::run()
{
    bool test = true;
    QString error;
    if(!m_testDB.open()){
        test = false;
        error = "数据库打开失败 info:"+m_testDB.lastError().text();
    }else{
        QSqlQuery query(m_testDB);
        if(!query.exec(m_script)){
            test = false;
            error ="sql执行失败 info:" + query.lastError().text();
        }else{
            test = initRuleMapper(query.record(),error);
        }
    }
    QSqlDatabase::removeDatabase(m_testDB.databaseName());
    emit testFlag(test,error);
}

bool RuleTestThread::initRuleMapper(QSqlRecord record, QString &error)
{
    bool initFlag = true;
    qint16 typeNumber = 0;
    qint16 typeText = 0;
    qint16 typeDate = 0;
    QSet<QString> numbers;
    numbers<<"double"<<"int"<<"qulonglong"<<"uint"<<"qlonglong"<<"long"<<"longlong"<<"qint";
    QSet<QString> texts;
    texts<<"QString"<<"QByteArray"<<"QChar";
    QSet<QString> dates;
    dates<<"QDateTime";
    m_columnMapper.clear();
    for (int i = 0; i < record.count(); ++i) {
        bool isType = false;
        QVariant var = record.value(i);
        QString colName = record.fieldName(i);
        //qDebug()<<record.fieldName(i)<<var.typeName()<<var.metaType();
        if(numbers.contains(var.typeName())){
            typeNumber+=1;
            m_columnMapper.insert(colName,"c"+QString::number(typeNumber));
            isType = true;
        }
        if(texts.contains(var.typeName())){
            typeText+=1;
            m_columnMapper.insert(colName,"c"+QString::number(10+typeText));
            isType = true;
        }

        if(dates.contains(var.typeName())){
            typeDate+=1;
            m_columnMapper.insert(colName,"c"+QString::number(20+typeDate));
            isType = true;
        }
        if(!isType){
            initFlag = false;
            error = "目前只支持数字，字符串以及时间类型 info:"+colName+"<<"+var.typeName();
            return initFlag;
        }
        if(typeNumber>10){
            initFlag = false;
            error = "数字类型的列最多只能有10个";
            return initFlag;
        }
        if(typeText>10){
            initFlag = false;
            error = "字符串类型的列最多只能有10个";
            return initFlag;
        }
        if(typeDate>5){
            initFlag = false;
            error = "日期类型的列最多只能有5个";
            return initFlag;
        }
    }
    return initFlag;
}










