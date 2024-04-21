#include "mainwindow.h"
#include "PulseUI/pcreateruledialog.h"
#include "PulseUI/pjobwindow.h"
#include "qsqlquery.h"
#include "ui_mainwindow.h"
#include"TaskSchedule/monitoringcyclethread.h"
#include"TaskSchedule/jobthreadfactory.h"
#include<QDateTime>
#include<QToolButton>
#include<QToolBox>
#include "PulseUI/pstandarditemmodel.h"
#include "PulseUI/pstystyleditemdelegate.h"
#include<QTimer>
#include<PulseUI/pcreateconnectdialog.h>
#include<QEvent>
#include<QSqlError>
#include <QSettings>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    pDataBaseSoure->init();
    JobThreadFactory::jobFactory->init();
    ui->setupUi(this);
    QSettings settings("DataPulse");
    bool dispatchSwitch = settings.value("DispatchSwitch",true).toBool();
    settings.setValue("DispatchSwitch",dispatchSwitch);
    initThread();
    initUI();
    m_mateDB = pDataBaseSoure->getMateConnect("mainWindow");
    qDebug()<<"MainWindow";

}

MainWindow::~MainWindow()
{
    if(cycleThread!=nullptr){
        cycleThread->stopThread();
        cycleThread->wait();
    }
    delete ui;
}

void MainWindow::initThread()
{
    cycleThread = new MonitoringCycleThread(this);
    QSettings settings("DataPulse");
    bool dispatchSwitch = settings.value("DispatchSwitch",true).toBool();
    if(dispatchSwitch){
        cycleThread->start();//控制后台调度线程是否启用
        qDebug()<<"cycleThread::start";
        ui->actToggle->setText("后台调度中");
    }else{
        qDebug()<<"cycleThread::stop";
        ui->actToggle->setText("后台调度停止");
    }
    connect(cycleThread,&MonitoringCycleThread::runJob,this,&MainWindow::do_runJob);
    connect(cycleThread,&MonitoringCycleThread::runJob,JobThreadFactory::jobFactory,&JobThreadFactory::jobRunnbaleFactory);
    qDebug()<<"MainWindow::initThread";
}

void MainWindow::initUI()
{
    initDock();
    QToolButton * toolBtnAdd = new QToolButton(this);
    toolBtnAdd->setText("添加");
    QMenu *meunAdd = new QMenu("添加",this);
    meunAdd->addAction(ui->actAddConnect);
    meunAdd->addAction(ui->actAddRule);
    meunAdd->addAction(ui->actAddJob);
    toolBtnAdd->setPopupMode(QToolButton::MenuButtonPopup);
    toolBtnAdd->setMenu(meunAdd);
    toolBtnAdd->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    this->ui->toolBar->insertWidget(ui->actVisible,toolBtnAdd);
    ui->actVisible->setVisible(false);
     // PJobWindow * cr = new PJobWindow(false,1,this);
     // ui->tabWidgetMain->insertTab(0,cr,cr->windowTitle());
     // ui->tabWidgetMain->setCurrentWidget(cr);
     // connect(cr,&PJobWindow::putJobid,cycleThread,&MonitoringCycleThread::doUpdateJob);
     // connect(cr,&PJobWindow::putJobid,treeModelConnect,&PStandardItemModel::do_updateJob);


    connect(this,&MainWindow::putJobid,cycleThread,&MonitoringCycleThread::doUpdateJob);
    connect(this,&MainWindow::putJobid,treeModelConnect,&PStandardItemModel::do_updateJob);
    connect(this,&MainWindow::putConnectId,JobThreadFactory::jobFactory,&JobThreadFactory::do_updateConnect);
    connect(this,&MainWindow::putConnectId,treeModelConnect,&PStandardItemModel::do_updateConnect);
    connect(this,&MainWindow::putRuleId,treeModelRule,&PStandardItemModel::do_updateRule);
    connect(this,&MainWindow::putRuleId,JobThreadFactory::jobFactory,&JobThreadFactory::do_updateRule);

    connect(this,&MainWindow::deleteJobid,cycleThread,&MonitoringCycleThread::doUpdateJob);
    connect(this,&MainWindow::deleteConnectid,JobThreadFactory::jobFactory,&JobThreadFactory::do_updateConnect);
    connect(this,&MainWindow::deleteRuleid,JobThreadFactory::jobFactory,&JobThreadFactory::do_updateRule);

    ui->treeViewConnect->installEventFilter(this);
    ui->treeViewRule->installEventFilter(this);
    qDebug()<<"initUi";
}



void MainWindow::do_runJob(quint64 jobId, quint64 connectId, quint64 ruleId)
{
    //QString str = QString::asprintf("现在正在执行job:%llu  connectId:%llu  ruleId:%llu  ",jobId,connectId,ruleId)+QDateTime::currentDateTime().toString("HH:mm:ss");
    QString str = QString("最近一次执行时间：")+QDateTime::currentDateTime().toString("HH:mm:ss");
    ui->statusBar->showMessage(str);
}

void MainWindow::initDock()
{
    if(treeModelConnect == nullptr) treeModelConnect = new PStandardItemModel(this);
    if(treeModelRule == nullptr) treeModelRule = new PStandardItemModel(this);
    ui->treeViewConnect->setItemDelegateForColumn(0,new PStyStyledItemDelegate());


    ui->treeViewConnect->header()->setVisible(false);
    ui->treeViewConnect->setModel(treeModelConnect);
    //ui->treeViewConnect->sortByColumn(0,Qt::AscendingOrder);
    ui->treeViewRule->setModel(treeModelRule);

    treeModelRule->init(1);
    treeModelConnect->init(0);
    ui->treeViewRule->header()->setVisible(false);
    //ui->treeViewRule->sortByColumn(0,Qt::AscendingOrder);
    ui->treeViewRule->setMouseTracking(true);
    ui->treeViewConnect->setMouseTracking(true);
    qDebug()<<"initDock";
}






void MainWindow::on_treeViewConnect_clicked(const QModelIndex &index)
{


}


void MainWindow::on_treeViewConnect_entered(const QModelIndex &index)
{


    treeModelConnect->setData(index,50,Qt::UserRole+3);

}


void MainWindow::on_treeViewConnect_doubleClicked(const QModelIndex &index)
{
    if(index.isValid()){
         ui->treeViewConnect->setCurrentIndex(index);
        if(index.data(Qt::UserRole+2).toString()=="connect_id"){
            on_actEditConnect_triggered();
        }else if(index.data(Qt::UserRole+2).toString()=="job_id"){
            on_actCheckJob_triggered();
        }
    }
}


void MainWindow::on_treeViewRule_doubleClicked(const QModelIndex &index)
{
    if(index.isValid()){
        ui->treeViewConnect->setCurrentIndex(index);
        if(index.data(Qt::UserRole+2).toString()=="rule_id"){
            on_actEditRule_triggered();
        }
    }
}

void MainWindow::on_actAddConnect_triggered()
{
    PCreateConnectDialog* pc = new PCreateConnectDialog(this);
    pc->setAttribute(Qt::WA_DeleteOnClose);
    pc->setWindowTitle("创建新连接");
    connect(pc,&PCreateConnectDialog::putConnectId,JobThreadFactory::jobFactory,&JobThreadFactory::do_updateConnect);
    connect(pc,&PCreateConnectDialog::putConnectId,treeModelConnect,&PStandardItemModel::do_updateConnect);
    //connect(pc,&PCreateConnectDialog::)
    pc->show();

}





void MainWindow::on_tabWidgetMain_tabCloseRequested(int index)
{
    QWidget *widget;
    widget = ui->tabWidgetMain->widget(index);
    if(widget!=nullptr){
        widget->close();
        //delete widget;
        ui->tabWidgetMain->removeTab(index);
    }
    //ui->tabWidgetMain->removeTab(index);
}


void MainWindow::on_actAddRule_triggered()
{
    PCreateRuleDialog* pr = new PCreateRuleDialog(this);
    pr->setWindowTitle("新建监控规则");
    ui->tabWidgetMain->insertTab(1,pr,pr->windowTitle());
    ui->tabWidgetMain->setCurrentWidget(pr);
    connect(pr,&PCreateRuleDialog::putRuleId,treeModelRule,&PStandardItemModel::do_updateRule);
    connect(pr,&PCreateRuleDialog::putRuleId,JobThreadFactory::jobFactory,&JobThreadFactory::do_updateRule);
    connect(pr,&PCreateRuleDialog::do_close,this,&MainWindow::on_closeTab);
}


void MainWindow::on_actAddJob_triggered()
{
    QModelIndex index = ui->treeViewConnect->currentIndex();
    if(index.isValid()){
        if(index.data(Qt::UserRole+2).toString()=="connect_id"){
            PJobWindow * cr = new PJobWindow(true ,index.data(Qt::UserRole+1).toInt(),this);
            cr->setWindowTitle("添加任务");
            ui->tabWidgetMain->insertTab(0,cr,cr->windowTitle());
            ui->tabWidgetMain->setCurrentWidget(cr);
            connect(cr,&PJobWindow::putJobid,cycleThread,&MonitoringCycleThread::doUpdateJob);
            connect(cr,&PJobWindow::putJobid,treeModelConnect,&PStandardItemModel::do_updateJob);
            connect(cr,&PJobWindow::do_close,this,&MainWindow::on_closeTab);
        }else{

        }
    }
}





bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->treeViewConnect){
        if(event->type()==QEvent::ContextMenu){
            QModelIndex index = ui->treeViewConnect->indexAt(ui->treeViewConnect->mapFromGlobal(QCursor::pos()));
            if(index.isValid()){

                ui->treeViewConnect->setCurrentIndex(index);
                if(index.data(Qt::UserRole+2).toString()=="connect_id"){
                    QMenu menu(this);
                    //menu.addAction(ui->actCheckConnect);
                    menu.addAction(ui->actEditConnect);
                    menu.addAction(ui->actDelConnect);
                    menu.addSeparator();
                    menu.addAction(ui->actAddJob);
                    menu.exec(QCursor::pos());
                }else if(index.data(Qt::UserRole+2).toString()=="job_id"){
                    QMenu menu(this);
                    menu.addAction(ui->actCheckJob);
                    menu.addAction(ui->actEditJob);
                    menu.addAction(ui->actDelJob);
                    menu.exec(QCursor::pos());

                }
                qDebug()<<index.data();
            }
        }
    }else if(watched == ui->treeViewRule){
        if(event->type()==QEvent::ContextMenu){
            QMenu menu(this);
            QModelIndex index = ui->treeViewRule->indexAt(ui->treeViewRule->mapFromGlobal(QCursor::pos()));
            if(index.isValid()){
                ui->treeViewRule->setCurrentIndex(index);
                qDebug()<<index.data();
                if(index.data(Qt::UserRole+2).toString()=="rule_id"){
                    //menu.addAction(ui->actCheckRule);
                    menu.addAction(ui->actEditRule);
                    menu.addAction(ui->actDelRule);
                    menu.addSeparator();
                }
            }
            menu.addAction(ui->actAddRule);
            menu.exec(QCursor::pos());
        }
    }
}


void MainWindow::on_actEditConnect_triggered()
{
    QModelIndex index = ui->treeViewConnect->currentIndex();
    if(index.data(Qt::UserRole+2).toString()=="connect_id"){
        PCreateConnectDialog* pc = new PCreateConnectDialog(index.data(Qt::UserRole+1).toInt(),this);
        pc->setAttribute(Qt::WA_DeleteOnClose);
        pc->setWindowTitle("修改连接");
        pc->show();
        connect(pc,&PCreateConnectDialog::putConnectId,JobThreadFactory::jobFactory,&JobThreadFactory::do_updateConnect);
        connect(pc,&PCreateConnectDialog::putConnectId,treeModelConnect,&PStandardItemModel::do_updateConnect);
    }else if(index.data(Qt::UserRole+2).toString()=="job_id"){
        QString widgetName = "编辑任务-"+index.data().toString();
        for (int i = 0; i < ui->tabWidgetMain->count(); ++i) {
            if(ui->tabWidgetMain->widget(i)->windowTitle()==widgetName){
                ui->tabWidgetMain->setCurrentIndex(i);
                return;
            }
        }
        PJobWindow * cr = new PJobWindow(false ,index.data(Qt::UserRole+1).toInt(),this);
        cr->setWindowTitle(widgetName);
        ui->tabWidgetMain->addTab(cr,cr->windowTitle());
        ui->tabWidgetMain->setCurrentWidget(cr);
        connect(cr,&PJobWindow::putJobid,cycleThread,&MonitoringCycleThread::doUpdateJob);
        connect(cr,&PJobWindow::putJobid,treeModelConnect,&PStandardItemModel::do_updateJob);
        connect(cr,&PJobWindow::do_close,this,&MainWindow::on_closeTab);
    }
}




void MainWindow::on_actEditJob_triggered()
{
    on_actEditConnect_triggered();
}

void MainWindow::on_actEditRule_triggered()
{
    QModelIndex index = ui->treeViewRule->currentIndex();
    if(index.data(Qt::UserRole+2).toString()=="rule_id"){
        QString widgetName = "编辑监控规则-"+index.data().toString();
        for (int i = 0; i < ui->tabWidgetMain->count(); ++i) {
            if(ui->tabWidgetMain->widget(i)->windowTitle()==widgetName){
                ui->tabWidgetMain->setCurrentIndex(i);
                return;
            }
        }
        qint64 ruleId =  index.data(Qt::UserRole+1).toInt();
        PCreateRuleDialog* pr = new PCreateRuleDialog(ruleId,this);
        pr->setWindowTitle(widgetName);
        ui->tabWidgetMain->addTab(pr,pr->windowTitle());
        ui->tabWidgetMain->setCurrentWidget(pr);
        connect(pr,&PCreateRuleDialog::putRuleId,treeModelRule,&PStandardItemModel::do_updateRule);
        connect(pr,&PCreateRuleDialog::putRuleId,JobThreadFactory::jobFactory,&JobThreadFactory::do_updateRule);
        connect(pr,&PCreateRuleDialog::do_close,this,&MainWindow::on_closeTab);
    }
}

void MainWindow::on_actDelConnect_triggered()
{
    QModelIndex index = ui->treeViewConnect->currentIndex();
    if(index.data(Qt::UserRole+2).toString()=="connect_id"){
        qint64 connectId = index.data(Qt::UserRole+1).toInt();
        QString str;
        QSqlDatabase MateDB = m_mateDB;
        if(!MateDB.isOpen()) MateDB.open();
        MateDB.transaction();
        QSqlQuery query(MateDB);
        str = "delete from pulse_job_monitoring where job_id in (select job_id where job_connection_id = :connect_id)";
        query.prepare(str);
        query.bindValue(":connect_id",connectId);
        if(query.exec()){
            str = "delete from pulse_connect_info where connect_id = :connect_id";
            query.prepare(str);
            query.bindValue(":connect_id",connectId);
            if(query.exec()){
                QModelIndex parentIndex = index.parent();
                QStandardItem *item = treeModelConnect->itemFromIndex(parentIndex);
                item->removeRow(index.row());
                MateDB.commit();
                emit deleteConnectid(connectId);
            }
        }else{
            MateDB.rollback();
        }
    }
}

void MainWindow::on_actDelJob_triggered()
{
    QModelIndex index = ui->treeViewConnect->currentIndex();
    if(index.data(Qt::UserRole+2).toString()=="job_id"){
        qint64 jobId = index.data(Qt::UserRole+1).toInt();
        QString str = "delete from pulse_job_monitoring where job_id = :job_id";
        QSqlDatabase MateDB = m_mateDB;
        if(!MateDB.isOpen()) MateDB.open();
        QSqlQuery query(MateDB);
        query.prepare(str);
        query.bindValue(":job_id",jobId);
        if(query.exec()){
            QModelIndex parentIndex = index.parent();
            QStandardItem *item = treeModelConnect->itemFromIndex(parentIndex);
            item->removeRow(index.row());
            ui->treeViewConnect->update();
            qDebug()<<index.row();
            emit deleteJobid(jobId);
        }else {
            qDebug()<<query.lastError().text();
        }
    }
}




void MainWindow::on_actDelRule_triggered()
{
     QModelIndex index = ui->treeViewRule->currentIndex();
    if(index.data(Qt::UserRole+2).toString()=="rule_id"){
        qint64 ruleId = index.data(Qt::UserRole+1).toInt();
        QString str;
        QSqlDatabase MateDB = m_mateDB;
        if(!MateDB.isOpen()) MateDB.open();
        MateDB.transaction();
        str = "delete from pulse_job_monitoring where job_rule_id = :rule_id";//删除任务表该规则
        QSqlQuery query(MateDB);
        query.prepare(str);
        query.bindValue(":rule_id",ruleId);
        if(!query.exec()){
            MateDB.rollback();
            return;
        }
        str = "delete from pulse_rule_info where rule_id = :rule_id";//删除该任务
        query.prepare(str);
        query.bindValue(":rule_id",ruleId);
        if(!query.exec()){
            MateDB.rollback();
            return;
        }
        str = "delete from pulse_insert_mappers where rule_id = :rule_id";//删除字段映射
        query.prepare(str);
        query.bindValue(":rule_id",ruleId);
        if(!query.exec()){
            MateDB.rollback();
            return;
        }
        MateDB.commit();
        QModelIndex parentIndex = index.parent();
        QStandardItem *item = treeModelRule->itemFromIndex(parentIndex);
        item->removeRow(index.row());
        emit deleteRuleid(ruleId);
        treeModelConnect->do_updateConnect(-1);
        qDebug()<<ruleId;
    }
}

void MainWindow::on_closeTab()
{
    on_tabWidgetMain_tabCloseRequested(ui->tabWidgetMain->currentIndex());

}





void MainWindow::on_actCheckJob_triggered()
{
     QModelIndex index = ui->treeViewConnect->currentIndex();
    if(index.data(Qt::UserRole+2).toString()=="job_id"){
        QString widgetName = "查看任务-"+index.data().toString();
        // for (int i = 0; i < ui->tabWidgetMain->count(); ++i) {
        //     if(ui->tabWidgetMain->widget(i)->windowTitle()==widgetName){
        //         ui->tabWidgetMain->setCurrentIndex(i);
        //         return;
        //     }
        // }
        PJobWindow * cr = new PJobWindow(index.data(Qt::UserRole+1).toInt(),this);
        cr->setWindowTitle(widgetName);
        ui->tabWidgetMain->addTab(cr,cr->windowTitle());
        ui->tabWidgetMain->setCurrentWidget(cr);
        connect(cr,&PJobWindow::do_close,this,&MainWindow::on_closeTab);
    }
}


void MainWindow::on_actToggle_triggered()
{
    QSettings settings("DataPulse");
    bool dispatchSwitch = settings.value("DispatchSwitch",true).toBool();
    settings.setValue("DispatchSwitch",!dispatchSwitch);
    if(!dispatchSwitch){
        cycleThread->start();//控制后台调度线程是否启用
        ui->actToggle->setText("后台调度中");
    }else{
        cycleThread->stopThread();
        ui->actToggle->setText("后台调度停止");
        ui->statusBar->clearMessage();
    }
}

