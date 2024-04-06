#include "pcreateconnectdialog.h"
#include<QSqlQuery>
#include<QSqlRecord>
#include<QSqlError>
#include<QThread>
#include "ui_pcreateconnectdialog.h"
#include<QToolTip>
#include<TaskSchedule/pdatabasesoure.h>
#include<QMessageBox>
#include<QDateTime>
PCreateConnectDialog::PCreateConnectDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PCreateConnectDialog)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    mateDB = pDataBaseSoure->getMateConnect("PCreateConnectDialog"+QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()));
    if(!mateDB.isOpen())mateDB.open();
    data.insert("connect_type","Database");

}

PCreateConnectDialog::PCreateConnectDialog(qint64 id, QWidget *parent):PCreateConnectDialog(parent)
{
    updataFlag = true;
    data.insert("connect_id",id);
    QString str = "SELECT connect_name, connect_type, connect_sub_type, database_name, user_name, connect_password, host_name, connect_port, connect_options, del_flag, object_version, privateKey, create_date, update_date, connect_comment "
                   " FROM pulse_connect_info where connect_id = :connect_id;";
    QSqlQuery query(mateDB);
    query.prepare(str);
    query.bindValue(":connect_id",data.value("connect_id").toInt());
    query.exec();
    query.next();
    QSqlRecord recode =  query.record();
    for (int i = 0; i < recode.count(); i++) {
        data.insert(recode.fieldName(i),recode.value(i));

    }
    //qDebug()<<query.lastError();
    ui->editHost->setText(data.value("host_name").toString());
    ui->editPort->setValue(data.value("connect_port").toInt());
    ui->editConnectName->setText(data.value("connect_name").toString());
    ui->editDBName->setText(data.value("database_name").toString());
    ui->editUserName->setText(data.value("user_name").toString());

    ui->editPassword->setText(data.value("connect_password").toString());

    ui->plainTextEdit->appendPlainText(data.value("connect_comment").toString());

    for (int i = 0; i < ui->listWidget->count(); ++i) {
        if(data.value("connect_sub_type").toString()==ui->listWidget->item(i)->text()) {
            ui->listWidget->setCurrentRow(i);
            break;
        }
    }
}

PCreateConnectDialog::~PCreateConnectDialog()
{
    delete ui;
    QSqlDatabase::removeDatabase(mateDB.connectionName());
    this->disconnect();
    socket->close();
    delete socket;
}


void PCreateConnectDialog::on_listWidget_clicked(const QModelIndex &index){
    data.insert("connect_sub_type",index.data().toString());
    on_btnTest_clicked();
}

void PCreateConnectDialog::do_openFlag(bool flag, QString info)
{
    QPalette palette = ui->labConnectState->palette();

    if(!flag){
        QString error = QString("数据库连接失败\n")+info;
        ui->labConnectState->setText(error);
        palette.setColor(QPalette::Text,Qt::red);
        //qDebug()<<DB.lastError().text();
        connValidity = false;
    }else{
        ui->labConnectState->setText("数据库连接成功");
        palette.setColor(QPalette::Text,Qt::black);
        connValidity = true;
    };
    ui->labConnectState->setPalette(palette);
    ui->btnTest->setEnabled(true);

}




void PCreateConnectDialog::on_editHost_editingFinished()
{
    data.insert("host_name",ui->editHost->text());
    on_editPort_editingFinished();
}


void PCreateConnectDialog::on_editPort_editingFinished()
{
    data.insert("connect_port",ui->editPort->value());
    if(data.count("host_name")==0) return;
    QHostAddress host(data.value("host_name").toString());
    socket->connectToHost(host,data.value("connect_port").toInt());
    connect(socket,&QTcpSocket::errorOccurred,this,&PCreateConnectDialog::do_errorScoket);
    connect(socket,&QTcpSocket::connected,this,&PCreateConnectDialog::do_connectedScoket);
}

void PCreateConnectDialog::do_errorScoket(QAbstractSocket::SocketError error)
{
    QString str = QString("主机连接失败");

    ui->editPort->setToolTip(str);
    ui->editHost->setToolTip(str);
    QPalette palette = ui->editPort->palette();
    palette.setColor(QPalette::Text,Qt::red);
    ui->editPort->setPalette(palette);
    ui->editHost->setPalette(palette);
}

void PCreateConnectDialog::do_connectedScoket()
{
    ui->editPort->setToolTip(QString("主机连接成功"));
    ui->editHost->setToolTip(QString("主机连接成功"));
    QPalette palette = ui->editPort->palette();
    palette.setColor(QPalette::Text,Qt::black);
    ui->editPort->setPalette(palette);
    ui->editHost->setPalette(palette);
    disconnect(socket,nullptr,nullptr,nullptr);
}




void PCreateConnectDialog::on_editConnectName_editingFinished()
{
    QString connectName = ui->editConnectName->text();
    data.insert("connect_name",connectName);
    QString str = "select count(1) as count from pulse_connect_info where connect_name = :connect_name";
    if(!mateDB.isOpen()) mateDB.open();
    QSqlQuery query(mateDB);
    query.prepare(str);
    query.bindValue(":connect_name",connectName);
    query.exec();
    query.next();
    QPalette palette = ui->editConnectName->palette();
    QSqlRecord row = query.record();
    if(updataFlag) {
        return;
    }

    if(row.value("count").toInt()!=0){
        ui->editConnectName->setToolTip(QString("连接名已存在"));
        palette.setColor(QPalette::Text,Qt::red);
        connNameValidity = false;
    }else{
        ui->editConnectName->setToolTip(QString("连接名合法"));
        palette.setColor(QPalette::Text,Qt::black);
        connNameValidity = true;

    }
    //qDebug()<<query.lastError()<<row.value("count").toInt();
    ui->editConnectName->setPalette(palette);
}

void PCreateConnectDialog::on_editDBName_editingFinished()
{
    data.insert("database_name",ui->editUserName->text());
    on_btnTest_clicked();
}


void PCreateConnectDialog::on_editUserName_editingFinished()
{
    data.insert("user_name",ui->editUserName->text());
    on_btnTest_clicked();
}


void PCreateConnectDialog::on_editPassword_editingFinished()
{
    data.insert("connect_password",ui->editPassword->text());
    on_btnTest_clicked();
}


void PCreateConnectDialog::on_btnTest_clicked()
{
    QString DBType=ui->listWidget->currentItem()->text();
    QString connectName = ui->editConnectName->text();
    QString host = ui->editHost->text();
    qint64 port = ui->editPort->value();
    QString DBName = ui->editDBName->text();
    QString userName = ui->editUserName->text();
    QString password = ui->editPassword->text();
    if((port==0)||connectName.isEmpty()||host.isEmpty()||DBName.isEmpty()||userName.isEmpty()||password.isEmpty()) return;
    QSqlDatabase DB = pDataBaseSoure->getDatabaseConnect(
        connectName+"test"+QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()),DBType,host,port,DBName,userName,password
        );
    if(connTest==nullptr){
        connTest = new DBConnectTestThread(DB);
        connect(connTest,&DBConnectTestThread::openFlag,this,&PCreateConnectDialog::do_openFlag);
    }else{
        if(!connTest->isRunning()){
            delete connTest;
            connTest = new DBConnectTestThread(DB);
            connTest->start();
            ui->labConnectState->setText("正在测试连接,请稍候...");
            ui->btnTest->setEnabled(false);
            connect(connTest,&DBConnectTestThread::openFlag,this,&PCreateConnectDialog::do_openFlag);
        }else{

        }
    }
}





void PCreateConnectDialog::on_btnOK_clicked()
{
    on_btnTest_clicked();
    on_editConnectName_editingFinished();
    if(!mateDB.isOpen()) mateDB.open();

    data.insert("connect_comment",ui->plainTextEdit->toPlainText());
    if(!connValidity){
        QMessageBox::information(this,"无效数据库连接","请检查连接信息");
        return;
    }
    if(!connNameValidity&&!updataFlag){
        QMessageBox::information(this,"连接名已存在","请修改连接名称");
        return;
    }

    if(updataFlag){
        updataConnect();
    }else{
        insertConnect();
    }


}


void PCreateConnectDialog::insertConnect()
{

    data.insert("connect_comment",ui->plainTextEdit->toPlainText());
    data.insert("connect_password",ui->editPassword->text());
    data.insert("connect_sub_type",ui->listWidget->currentItem()->text());
    data.insert("user_name",ui->editUserName->text());
    data.insert("database_name",ui->editDBName->text());
    data.insert("connect_port",ui->editPort->value());
    data.insert("host_name",ui->editHost->text());
    data.insert("create_date",QDateTime::currentDateTime());
    data.insert("update_date",QDateTime::currentDateTime());
    data.insert("del_flag","N");
    data.insert("connect_sub_type",ui->listWidget->currentItem()->text());
    QString insert ="INSERT INTO pulse_connect_info"
                     "(connect_id, connect_name, connect_type, connect_sub_type, database_name, user_name, connect_password, host_name, connect_port, connect_options, del_flag, object_version, privateKey, create_date, update_date, connect_comment)"
                     " VALUES (:connect_id, :connect_name, :connect_type, :connect_sub_type, :database_name, :user_name, :connect_password, :host_name, :connect_port, :connect_options, :del_flag, :object_version, :privateKey, :create_date, :update_date, :connect_comment);";
    QSqlQuery query(mateDB);
    query.prepare(insert);
    foreach (auto var, data.keys()) {
        query.bindValue(":"+var,data.value(var));
    }
    if(!query.exec()){
        QMessageBox::information(this,"连接信息写入失败",query.lastError().text());
        return;
    }
    QString str = "select connect_id from pulse_connect_info where connect_name = :connect_name";
    query.prepare(str);
    query.bindValue(":connect_name",data.value("connect_name"));
    query.exec();
    query.next();
    qint64 connectId =  query.record().value("connect_id").toInt();
    emit putConnectId(connectId);
    this->accept();
}

void PCreateConnectDialog::updataConnect()
{
    data.insert("connect_comment",ui->plainTextEdit->toPlainText());
    data.insert("connect_password",ui->editPassword->text());
    data.insert("connect_sub_type",ui->listWidget->currentItem()->text());
    data.insert("user_name",ui->editUserName->text());
    data.insert("connect_name",ui->editConnectName->text());
    data.insert("database_name",ui->editDBName->text());
    data.insert("connect_port",ui->editPort->value());
    data.insert("host_name",ui->editHost->text());
    data.insert("update_date",QDateTime::currentDateTime());
    data.insert("connect_sub_type",ui->listWidget->currentItem()->text());
    QSqlQuery query(mateDB);
    data.insert("update_date",QDateTime::currentDateTime());
    QString str = "UPDATE pulse_connect_info "
          " SET connect_name=:connect_name, connect_type=:connect_type, connect_sub_type=:connect_sub_type, database_name=:database_name, user_name=:user_name, connect_password=:connect_password, host_name=:host_name, connect_port=:connect_port, connect_options=:connect_options, del_flag=:del_flag, object_version=:object_version, privateKey=:privateKey, create_date=:create_date, update_date=:update_date, connect_comment=:connect_comment "
          " WHERE connect_id=:connect_id;";
    query.prepare(str);
    foreach (auto var, data.keys()) {
        query.bindValue(":"+var,data.value(var));
    }
    if(!query.exec()){
        QMessageBox::information(this,"连接信息修改失败",query.lastError().text());
        return;
    }
    emit putConnectId(data.value("connect_id").toInt());
    this->accept();
}


DBConnectTestThread::DBConnectTestThread(QSqlDatabase DB, QObject *parent):QThread(parent)
{
    this->DBConnect = DB;
}

DBConnectTestThread::~DBConnectTestThread()
{
    QSqlDatabase::removeDatabase(DBConnect.connectionName());
    qDebug()<<DBConnect.connectionName()<<"<-remove";
}

void DBConnectTestThread::run()
{

    bool isOpen = DBConnect.open();
    QString info = DBConnect.lastError().text();
    emit openFlag(isOpen,info);

    DBConnect.close();
    QSqlDatabase::removeDatabase(DBConnect.connectionName());
}


