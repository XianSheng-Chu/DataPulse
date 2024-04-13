#include "pjobwindow.h"

#include<QSqlRecord>
#include "TaskSchedule/jobthreadfactory.h"
#include "ui_pjobwindow.h"
#include<QSqlQuery>
#include<QSqlError>
#include<QSplineSeries>
#include<QValueAxis>
#include<QDateTimeAxis>
#include<QPair>
PJobWindow::PJobWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PJobWindow)
{
    ui->setupUi(this);
    m_butchModel= new QSqlQueryModel(this);
    m_resultModel = new QSqlQueryModel(this);
    connect(this,&PJobWindow::runJob,JobThreadFactory::jobFactory,&JobThreadFactory::jobRunnbaleFactory);
    dataRangMin = QDateTime::currentDateTime().addDays(-1);
    ui->chartView->setRenderHint(QPainter::RenderHint::Antialiasing);

}

PJobWindow::PJobWindow(qint64 jobId, QWidget *parent):PJobWindow(parent)
{
    m_jobId= jobId;
    m_sortCol.append(QPair<QString,QString>("start_time","asc"));
    ui->comboBox->setVisible(false);
    ui->butBox->setVisible(false);
    init();
}

PJobWindow::PJobWindow(bool creatFlag, qint64 id, QWidget *parent):PJobWindow(parent)
{
    if(creatFlag){
        m_connectId = id;
        m_creatFlag = creatFlag;
        if(!m_metaDB.isValid()) m_metaDB = pDataBaseSoure->getMateConnect("Createjob:"+QString::number(id)+QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss zzz"));
        if(!m_metaDB.isOpen())
            m_metaDB.open();
        initConnect(id);
        QSqlQuery query(m_metaDB);

        QString str ="SELECT rule_id, rule_name, rule_connect_type, rule_connect_sub_type, rule_script, rule_is_test, rule_comment, create_date, update_date, del_flag, public_flag, test_connect_id "
                     " FROM pulse_rule_info  where rule_connect_sub_type = :rule_connect_sub_type "
                     " and rule_id not in (select job_rule_id from pulse_job_monitoring where job_connection_id = :job_connection_id)"
                     " order by public_flag desc;";
        query.prepare(str);
        query.bindValue(":rule_connect_sub_type",m_connect.value("connect_sub_type"));
        query.bindValue(":job_connection_id",m_connect.value("connection_id"));
        query.exec();
        if(query.lastError().isValid()){
            qDebug()<< query.lastError();
        }
        while(query.next()){
            QSqlRecord record = query.record();
            m_RuleIdMap.insert(record.value("rule_name").toString(),record.value("rule_id").toInt());
            ui->comboBox->addItem(record.value("rule_name").toString());
        }
        ui->groupBox->setVisible(false);
        ui->tabWidget->setVisible(false);
        ui->butBox->setLocale(QLocale("Chinese"));
    }else{
        m_jobId = id;
        m_creatFlag = creatFlag;
        ui->comboBox->setVisible(false);
        init();
    }
}



PJobWindow::~PJobWindow()
{
    //deleteCahrtViewList(cahrtViewList);
    //deleteCahrtList(cahrtList);
    QSqlDatabase::removeDatabase(m_metaDB.connectionName());
    QSqlDatabase::removeDatabase(m_connectDB.connectionName());

    delete ui;
}

void PJobWindow::init()
{

    if(!m_metaDB.isValid()) m_metaDB = pDataBaseSoure->getMateConnect("job:"+QString::number(m_jobId));
    if(!m_metaDB.isOpen())
        m_metaDB.open();
    QSqlQuery query(m_metaDB);

    QString str ="SELECT job_id, job_connection_id, job_connection_name, waiting_time_sec, job_rule_id, job_rule_name, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17, c18, c19, c20, c21, c22, c23, c24, c25, c1, suspend_flag, job_comment, create_date, update_date, del_flag "
                  " FROM pulse_job_monitoring where job_id = :job_id;";
    query.prepare(str);
    query.bindValue(":job_id",m_jobId);
    query.exec();
    if(query.next()){
        readData(m_jobInfo,query.record());
    }else{
        qDebug()<< query.lastError();
    }
    ui->spinBox->setValue(m_jobInfo.value("waiting_time_sec").toInt());
    ui->editComment->clear();
    ui->editComment->appendPlainText(m_jobInfo.value("job_comment").toString());
    ui->chkSuspend->setChecked(m_jobInfo.value("suspend_flag").toString()=="Y"?true:false);
    m_ruleId = m_jobInfo.value("job_rule_id").toInt();
    m_connectId = m_jobInfo.value("job_connection_id").toInt();
    m_connectDB = pDataBaseSoure->getDatabaseConnect(m_jobInfo.value("job_connection_name").toString()+QString::number(m_connectId)+"-chart"+QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()),m_jobInfo.value("job_connection_name").toString());
    initRule(m_ruleId);
    initConnect(m_connectId);
    initMapper();
    initButch();
}

void PJobWindow::initMapper()
{
    m_colMapper.clear();
    QSqlQuery query(m_metaDB);
    QString str = "select max(update_date) last_update from pulse_rule_info where rule_id = :rule_id;";
    query.prepare(str);

    query.bindValue(":rule_id",m_ruleId);
    query.exec();
    query.next();
    m_RuleLastUpdate = query.record().value("last_update").toDateTime();
    query.clear();
    str = "SELECT rule_id, soure_column_name, target_column_name, is_check, condition_numeric, condition_text, condition_name "
            " FROM pulse_insert_mappers where rule_id = :rule_id;";
    query.prepare(str);
    query.bindValue(":rule_id",m_ruleId);
    query.exec();
    while(query.next()){
        QSqlRecord record = query.record();
        m_colMapper.insert(record.value("target_column_name").toString(),record.value("soure_column_name").toString());
    }
     qDebug()<< query.lastQuery();
    initChart();
}

void PJobWindow::initButch()
{
    m_butch.clear();
    m_butchModel->clear();
    QSqlQuery query(m_metaDB);
    QString str = "SELECT batch_number, rule_name, connect_name, start_time, end_time, row_count, is_success, error_message, abnormal_flag, rule_id, connect_id "
        " FROM pulse_job_butch where rule_id = :rule_id and start_time > :start_time;";
    query.prepare(str);
    query.bindValue(":rule_id",m_ruleId);
    query.bindValue(":start_time",qMax(dataRangMin,m_RuleLastUpdate));
    query.exec();
    readData(m_butch,query);

    ui->tabBatch->setModel(m_butchModel);
    str = "SELECT batch_number, start_time, end_time, row_count, is_success, error_message "
          " FROM pulse_job_butch where rule_id = :rule_id and start_time > :start_time order by batch_number desc;";
    query.prepare(str);
    query.bindValue(":rule_id",m_ruleId);
    query.bindValue(":start_time",qMax(dataRangMin,m_RuleLastUpdate));
    query.exec();
    m_butchModel->setQuery(query);


    m_butchModel->setHeaderData(0,Qt::Horizontal,"批次号");
    m_butchModel->setHeaderData(1,Qt::Horizontal,"运行开始时间");
    m_butchModel->setHeaderData(2,Qt::Horizontal,"运行结束时间");
    m_butchModel->setHeaderData(3,Qt::Horizontal,"结果条数");
    m_butchModel->setHeaderData(4,Qt::Horizontal,"是否成功");
    m_butchModel->setHeaderData(5,Qt::Horizontal,"错误信息");
    if(m_butchModel->index(0,1).isValid()){
        ui->dateTimeEdit->setDateTime(m_butchModel->index(0,1).data().toDateTime().addSecs(m_jobInfo.value("waiting_time_sec").toInt()));
    }
    //qDebug()<< query.lastError();
    while(m_butchModel->canFetchMore()){
        m_butchModel->fetchMore();
    }
}

void PJobWindow::initResult(qint64 butchId)
{
    m_result.clear();
    m_resultModel->clear();
    QSqlQuery query(m_metaDB);
    QString str = "SELECT batch_number, connect_name, rule_name,  ";
    for (QString var : m_colMapper.keys()) {
        str+= var + " as " + var +",";
    }
    str+="start_time, end_time FROM pulse_job_result where batch_number =:batch_number and start_time > :start_time ";
    if(!m_sortCol.isEmpty()){
        str += " order by ";
        for (int i = 0; i < m_sortCol.count(); ++i) {
            str+= " "+m_sortCol.at(i).first+" "+m_sortCol.at(i).second;
            if(i!=m_sortCol.count()-1){
                str+=",";
            }
        }
    }
    query.prepare(str);
    query.bindValue(":batch_number",butchId);
    query.bindValue(":start_time",qMax(dataRangMin,m_RuleLastUpdate));
    query.exec();
    if(query.lastError().isValid()){
        qDebug()<< query.lastError();
    }
    m_resultModel->setQuery(query);
    ui->tabResult->setModel(m_resultModel);
    ui->tabWidget->setCurrentIndex(1);
    //readData(m_result,query);
    qDebug()<<"butchId:"<<butchId;
    qDebug()<<query.lastQuery();
    QSqlRecord record = m_resultModel->record();
    for (QString var : m_colMapper.keys()) {
        m_resultModel->setHeaderData(record.indexOf(var),Qt::Horizontal,m_colMapper.value(var));
    }
    while(m_resultModel->canFetchMore()){
        m_resultModel->fetchMore();
    }
    ui->tabWidget->setCurrentIndex(1);
}



void PJobWindow::initRule(qint64 ruleId)
{
    QSqlQuery query(m_metaDB);

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
        m_rule.clear();
        readData(m_rule,query.record());
        str = QString::asprintf("规则名称：%1").arg(m_rule.value("rule_name").toString());
        ui->labRuleName->setText(str);
    }else{
        ui->statusbar->showMessage("该规则已不存在");
    }
}

void PJobWindow::initConnect(qint64 connectId)
{
    m_connectId = connectId;
    QString str = "SELECT connect_name, connect_type, connect_sub_type, database_name, user_name, connect_password, host_name, connect_port, connect_options, del_flag, object_version, privateKey, create_date, update_date, connect_comment "
                  " FROM pulse_connect_info where connect_id = :connect_id;";
    QSqlQuery query(m_metaDB);
    query.prepare(str);
    query.bindValue(":connect_id",m_connectId);
    if(!query.exec()){
        qDebug()<<query.lastError();
        qDebug()<<query.lastQuery();
    }
    if(query.next()){
        m_connect.clear();
        readData(m_connect,query.record());
        str = QString::asprintf("%1 -> %2 --%3:%4").arg(m_connect.value("connect_sub_type").toString()).arg(m_connect.value("connect_name").toString()).arg(m_connect.value("host_name").toString()).arg(m_connect.value("connect_port").toString());
        ui->labConnectInfo->setText(str);
    }
}



void PJobWindow::readData(QList<QHash<QString, QVariant> > &listMap, QSqlQuery &query)
{
    listMap.clear();
    while (query.next()) {
        QHash<QString, QVariant> hash;
        readData(hash,query.record());
        listMap.append(hash);
    }
}

void PJobWindow::readData(QHash<QString, QVariant> &map, QSqlRecord record)
{
    map.clear();
    for (int i = 0; i < record.count(); ++i) {
        map.insert(record.fieldName(i),record.value(i));
    }
}


void PJobWindow::initChart()
{
    if(m_chart != nullptr) delete m_chart;
    m_chart =new PChart();
    m_chart->setBackgroundRoundness(0);
    ui->chartView->setChart(m_chart);
    ui->combDataCol->clear();
    ui->combFlagCol->clear();
    for (QString var : m_colMapper.keys()) {
        if(mapperType.value(var)=="number"){
            ui->combDataCol->addItem(m_colMapper.value(var));
        }else if(mapperType.value(var)=="string"){
            ui->combFlagCol->addItem(m_colMapper.value(var));
        }
    }
    QValueAxis* axisY = new QValueAxis;
    axisY->setRange(0,10);
    axisY->setTickCount(0);
    QDateTimeAxis* axisX = new QDateTimeAxis;
    axisX->setFormat("MM-dd hh:mm:ss");
    axisX->setRange(QDateTime::currentDateTime().addSecs(-1000),QDateTime::currentDateTime());
    axisY->setTickCount(0);
    axisY->setLabelFormat("%.2f");
    axisX->setTitleText("时间");
    //axisX->setGridLineVisible(false);
    //axisY->setGridLineVisible(false);

    m_chart->axisX = axisX;
    m_chart->axisY = axisY;
    m_chart->addAxis(axisX,Qt::AlignBottom);
    m_chart->addAxis(axisY,Qt::AlignLeft);
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
}

void PJobWindow::testChart()
{
    m_chart->setTitle("Test Chart");
    QLineSeries* qps = new QSplineSeries;
    m_chart->addSeries(qps);
    QValueAxis* axisY = new QValueAxis;
    axisY->setRange(0,10);
    axisY->setTickCount(0);
    QDateTimeAxis* axisX = new QDateTimeAxis;
    axisX->setFormat("hh:mm:ss");
    axisX->setRange(QDateTime::currentDateTime().addSecs(-1000),QDateTime::currentDateTime());
    axisY->setTickCount(0);

    QHash<QString,QVariant> previous = m_result.at(0);
    qreal maxQps=0;
    qreal minTime;
    qreal quest;
    for (int i = 1; i < m_result.count(); ++i) {

        QHash<QString,QVariant> last = m_result.at(i);
        qreal interval = previous.value("start_time").toDateTime().msecsTo(last.value("start_time").toDateTime());
        if(interval<10000) continue;
        qDebug()<<interval;
        interval/=1000;
        qreal intervar = last.value("Questions").toInt() - previous.value("Questions").toInt();
        qreal valY = intervar/ interval;

        if(maxQps<valY){
            maxQps = valY;
            minTime = interval;
            quest = intervar;
        }
        previous = last;
        if(previous.value("start_time").toDateTime()<axisX->min()) continue;
        qDebug()<<valY;
        qps->append(last.value("start_time").toDateTime().toMSecsSinceEpoch(),valY);

    }
    qDebug()<<QString::asprintf("maxQps:%.6f,minTime:%.6f,quest:%.6f",maxQps,minTime,quest);
    m_chart->addAxis(axisX,Qt::AlignBottom);
    m_chart->addAxis(axisY,Qt::AlignLeft);

    qps->attachAxis(axisX);
    qps->attachAxis(axisY);
}











const QHash<QString,QString> PJobWindow::mapperType = {{"c1","number"}, {"c2","number"}, {"c3","number"}, {"c4","number"}, {"c5","number"}, {"c6","number"}, {"c7","number"}, {"c8","number"}, {"c9","number"}, {"c10","number"}, {"c11","string"}, {"c12","string"}, {"c13","string"}, {"c14","string"}, {"c15","string"}, {"c16","string"}, {"c17","string"}, {"c18","string"}, {"c19","string"}, {"c20","string"}, {"c21","datetime"}, {"c22","datetime"}, {"c23","datetime"}, {"c24","datetime"}, {"c25","datetime"}};









void PJobWindow::on_butBox_rejected()
{
    ui->butBox->setVisible(false);
    emit do_close();
}


void PJobWindow::on_butBox_accepted()
{
    QString str;
    QSqlQuery query(m_metaDB);
    if(m_creatFlag){
        str = "INSERT INTO pulse_job_monitoring (job_connection_id, job_connection_name, waiting_time_sec, job_rule_id, job_rule_name, suspend_flag, job_comment, create_date, update_date, del_flag)"
              " VALUES(:job_connection_id, :job_connection_name, :waiting_time_sec, :job_rule_id, :job_rule_name, :suspend_flag, :job_comment, :create_date, :update_date, :del_flag);";

        QDateTime createTime = QDateTime::currentDateTime();
        query.prepare(str);
        query.bindValue(":job_connection_id",m_connectId);
        query.bindValue(":job_connection_name",m_connect.value("connect_name"));
        query.bindValue(":waiting_time_sec",ui->spinBox->value());
        query.bindValue(":job_rule_id",m_RuleIdMap.value(ui->comboBox->currentText()));
        query.bindValue(":job_rule_name",ui->comboBox->currentText());
        query.bindValue(":suspend_flag",ui->chkSuspend->isChecked()?"Y":"N");
        query.bindValue(":job_comment",ui->editComment->toPlainText());
        query.bindValue(":create_date",createTime);
        query.bindValue(":update_date",QDateTime::currentDateTime());
        query.bindValue(":del_flag","N");

        if(query.exec()){
            str = "SELECT job_id FROM pulse_job_monitoring where job_connection_id = :job_connection_id and job_rule_id = :job_rule_id and update_date>=:update_date;";
            query.prepare(str);
            query.bindValue(":job_connection_id",m_connectId);
            query.bindValue(":job_rule_id",m_RuleIdMap.value(ui->comboBox->currentText()));
            query.bindValue(":update_date",createTime);
            query.exec();
            if(query.next()){
                m_jobId = query.record().value("job_id").toInt();
                emit putJobid(m_jobId);
                m_creatFlag = false;
                init();
            }
        }else{
            qDebug()<<query.lastError();
        }
        emit do_close();
    }else{
        str = "UPDATE pulse_job_monitoring set waiting_time_sec=:waiting_time_sec, suspend_flag=:suspend_flag, job_comment=:job_comment, update_date=:update_date, del_flag=:del_flag "
              " WHERE job_id=:job_id;";
        query.prepare(str);
        query.bindValue(":waiting_time_sec",ui->spinBox->value());
        query.bindValue(":suspend_flag",ui->chkSuspend->isChecked()?"Y":"N");
        query.bindValue(":job_comment",ui->editComment->toPlainText());
        query.bindValue(":update_date",QDateTime::currentDateTime());
        query.bindValue(":del_flag","N");
        query.bindValue(":job_id",m_jobId);
        if(query.exec()){
            emit putJobid(m_jobId);
            init();
        }else{
            qDebug()<<query.lastError();
        }
        ui->butBox->setVisible(false);
        emit do_close();
    }
}


void PJobWindow::on_combTimeRang_currentIndexChanged(int index)
{
    qint64 days = -1;
    switch (index) {
    case 0:
        days = -1;
        break;
    case 1:
        days = -2;
        break;
    case 2:
        days = -4;
        break;
    case 3:
        days = -7;
        break;
    default:
        break;
    }

    dataRangMin = QDateTime::currentDateTime().addDays(days);
    initButch();
}


void PJobWindow::on_tabBatch_doubleClicked(const QModelIndex &index)
{
    QModelIndex numIndex = m_butchModel->index(index.row(),0);
    qint64 batchNum = numIndex.data().toInt();
    initResult(batchNum);
    qDebug()<<" index.row()"<<index.row();
    qDebug()<<" index.column()"<<index.column();
    qDebug()<<" batchNum："<<batchNum;
}


void PJobWindow::on_btnRunNow_clicked()
{
    emit runJob(m_jobId,m_connectId,m_ruleId);
}


void PJobWindow::on_btnRunNow_3_clicked()
{

    if(timer == nullptr){
        timer = new QTimer(this);
        connect(timer,&QTimer::timeout,this,&PJobWindow::do_chartValue);
    }
    QString chartTitle;
    chartTitle+=(m_rule.value("rule_name").toString());
    chartTitle+="-";
    chartTitle+=ui->combDataCol->currentText();

    if(ui->chkRate->isChecked()){
        chartTitle+="/s";
    }

    m_chart->removeAllSeries();
    m_chart->seriesList.clear();
    m_chart->setTitle(chartTitle);

    QSqlQuery query(m_metaDB);
    QString str = "SELECT batch_number, connect_name, rule_name,  ";
    for (QString var : m_colMapper.keys()) {
        str+= var + " as \"" + m_colMapper.value(var) +"\",";
    }
    str+="start_time, end_time FROM pulse_job_result where  start_time > :start_time  and rule_id = :rule_id and  connect_id = :connect_id order by 1;";

    query.prepare(str);
    query.bindValue(":start_time",qMax(dataRangMin,m_RuleLastUpdate));
    query.bindValue(":rule_id",m_ruleId);
    query.bindValue(":connect_id",m_connectId);
    query.exec();
    qDebug()<<str;
    if(query.lastError().isValid()){
        qDebug()<< query.lastError();
    }
    bool linesFlag =  ui->combFlagCol->currentText().isEmpty();
    QString currKey;
    if(linesFlag) {
        currKey = ui->combDataCol->currentText();
    }

    QHash<QString,QSqlRecord> previous;
    m_chart->axisX->setMax(QDateTime::currentDateTime());

    qreal rang = ui->spinRang->value();
    m_chart->axisX->setMin(QDateTime::currentDateTime().addSecs(-1*rang));
    qreal maxY=0,minY=0;
    while(query.next()){
        QSqlRecord record = query.record();
        if(!linesFlag){
            currKey = record.value(ui->combFlagCol->currentText()).toString();
        }
        qreal intervalMsec = 1000;

        if(ui->chkRate->isChecked()){
            if(previous.count()==0){
                previous.insert(currKey,record);
                continue;
            }else{
                if(record.value(ui->combDataCol->currentText()).isNull()) continue;
                intervalMsec = previous.value(currKey).value("start_time").toDateTime().msecsTo(record.value("start_time").toDateTime());
                if(intervalMsec<rang/40) continue;
            }
        }
        intervalMsec/=1000;
        if(record.value("start_time").toDateTime().addSecs(rang*0.1)<m_chart->axisX->min()) continue;

        if(m_chart->seriesList.count(currKey)==0){
            QLineSeries* line = new QSplineSeries;
            m_chart->seriesList.insert(currKey,line);
            m_chart->addSeries(line);
            line->setName(currKey);
            qDebug()<<"linesKey:"<<currKey;
            qDebug()<<QString("m_chart->addSeries(line);");
        }
        qreal valueX = record.value("start_time").toDateTime().toMSecsSinceEpoch();
        qreal valueY = 0;
        if(ui->chkRate->isChecked()){
            valueY = (record.value(ui->combDataCol->currentText()).toDouble()-previous.value(currKey).value(ui->combDataCol->currentText()).toDouble())/intervalMsec;
            qDebug()<<"curr:"<<record.value(ui->combDataCol->currentText()).toDouble()<<" previos:"<<previous.value(currKey).value(ui->combDataCol->currentText()).toDouble();
            previous.insert(currKey,record);
        }else{
            valueY = record.value(ui->combDataCol->currentText()).toDouble();
        }
        if(maxY==0&&minY==0){
            maxY = valueY;
            minY = valueY;
        }
        if(maxY<valueY){
            maxY = valueY;
        }
        if(minY>valueY){
            minY = valueY;
        }

        m_chart->seriesList.value(currKey)->append(valueX,valueY);


    }
    for (QLineSeries *line : m_chart->seriesList) {
        line->attachAxis(m_chart->axisX);
        line->attachAxis(m_chart->axisY);
    }
    m_chart->axisY->setTitleText(ui->combDataCol->currentText());
    qreal minV =  minY-(maxY-minY)*0.1;
    if(minV<0&minY>0){
        minV = 0;
    }
    qDebug()<<"maxY:"<<maxY;
    qDebug()<<"minY:"<<minY;

    m_chart->axisY->setRange(minV,maxY+0.1*(maxY-minY));
    ui->tabWidget->setCurrentIndex(2);
    m_chart->legend()->setAlignment(Qt::AlignRight);

    if(ui->btnSlide->isChecked()){
        timer->setInterval(1000*ui->spinSampInterval->value());
        timer->start();
    }else{
        timer->stop();
    }
}

void PJobWindow::do_chartValue()
{
    if(!m_connectDB.isOpen()){
        if(!m_connectDB.open()){
            ui->statusbar->showMessage(QString(m_jobInfo.value("job_connection_name").toString()+"连接失败"+m_connectDB.lastError().text()));
            timer->stop();
            return;
        }
    }
    QSqlQuery query(m_connectDB);
    if(query.exec(m_rule.value("rule_script").toString())){
        QDateTime currDateTime = QDateTime::currentDateTime();
        ui->statusbar->showMessage(QString("上次实时状态刷新时间"+currDateTime.toString("yyyy/MM/dd hh24:mi:ss")));
        while(query.next()){
            QSqlRecord record = query.record();
            RealtimeDatas.append(QPair<QDateTime,QSqlRecord>(currDateTime,record));
            qreal valueY;
            bool isFlag = !ui->combFlagCol->currentText().isEmpty();
            QString curKey = isFlag?record.value(ui->combFlagCol->currentText()).toString():ui->combDataCol->currentText();
            QLineSeries* line = m_chart->seriesList.value(curKey);
            if(m_chart->seriesList.count(curKey) == 0) {
                line = new QSplineSeries;
                m_chart->seriesList.insert(curKey,line);
                m_chart->addSeries(line);
                line->setName(curKey);
                line->attachAxis(m_chart->axisX);
                line->attachAxis(m_chart->axisY);
            }
            if(ui->chkRate->isChecked()){
                if(RealPrevious.count(curKey)==0) {
                    RealPrevious.insert(curKey,QPair<QDateTime,QSqlRecord>(currDateTime,record));

                    continue;
                }
                valueY = ((record.value(ui->combDataCol->currentText()).toDouble()-RealPrevious.value(curKey).second.value(ui->combDataCol->currentText()).toDouble())*1000)/RealPrevious.value(curKey).first.msecsTo(currDateTime);
            }else{
                valueY = record.value(ui->combDataCol->currentText()).toDouble();
            }
            RealPrevious.insert(curKey,QPair<QDateTime,QSqlRecord>(currDateTime,record));
            if(timer->isActive()){
                m_chart->axisX->setMax(QDateTime::currentDateTime());
                qreal rang = ui->spinRang->value();
                m_chart->axisX->setMin(QDateTime::currentDateTime().addSecs(-1*rang));
                ui->statusbar->showMessage(QString("正在实时绘制"+QDateTime::currentDateTime().toString("yyyy/MM/dd hh24:mm:ss")));
                if(valueY>m_chart->axisY->max()){
                    m_chart->axisY->setMax(valueY+(m_chart->axisY->max()-m_chart->axisY->min())*0.2);
                }else if(valueY<m_chart->axisY->min()){
                    qreal minV =  valueY-(m_chart->axisY->max()-m_chart->axisY->min())*0.1;
                    m_chart->axisY->setMin(minV<=0?0:minV);
                }
                line->append(currDateTime.toMSecsSinceEpoch(),valueY);
            }
        }
    }else{
        //ui->statusbar->showMessage(QString("实时sql执行失败"+QDateTime::currentDateTime().toString("yyyy/MM/dd hh24:mm:ss")));
    }
}

void PJobWindow::on_btnSlide_clicked(bool checked)
{
    if(timer==nullptr) return;
    if(checked){
        timer->start();
    }else{
        timer->stop();
    }
}

