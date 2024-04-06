#include "mainwindow.h"
#include "PulseUI/pcreateruledialog.h"
#include "PulseUI/pjobwindow.h"
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
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initThread();
    initUI();
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
    //cycleThread->start();//控制后台调度线程是否启用
    connect(cycleThread,&MonitoringCycleThread::runJob,this,&MainWindow::do_runJob);
    connect(cycleThread,&MonitoringCycleThread::runJob,JobThreadFactory::jobFactory,&JobThreadFactory::jobRunnbaleFactory);

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
     PJobWindow * cr = new PJobWindow(false,1,this);
     ui->tabWidgetMain->insertTab(0,cr,cr->windowTitle());
     ui->tabWidgetMain->setCurrentWidget(cr);
     connect(cr,&PJobWindow::putJobid,cycleThread,&MonitoringCycleThread::doUpdateJob);
     connect(cr,&PJobWindow::putJobid,treeModelConnect,&PStandardItemModel::do_updateJob);
}



void MainWindow::do_runJob(quint64 jobId, quint64 connectId, quint64 ruleId)
{
    QString str = QString::asprintf("现在正在执行job:%llu  connectId:%llu  ruleId:%llu  ",jobId,connectId,ruleId)+QDateTime::currentDateTime().toString("HH:mm:ss");
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
    if(index.data(Qt::UserRole+2).toString()=="connect_id"){
        PCreateConnectDialog* pc = new PCreateConnectDialog(index.data(Qt::UserRole+1).toInt(),this);
        pc->setAttribute(Qt::WA_DeleteOnClose);
        pc->setWindowTitle("修改连接");
        pc->show();
        connect(pc,&PCreateConnectDialog::putConnectId,JobThreadFactory::jobFactory,&JobThreadFactory::do_updateConnect);
        connect(pc,&PCreateConnectDialog::putConnectId,treeModelConnect,&PStandardItemModel::do_updateConnect);
    }else if(index.data(Qt::UserRole+2).toString()=="job_id"){
        PJobWindow * cr = new PJobWindow(false ,index.data(Qt::UserRole+1).toInt(),this);
        ui->tabWidgetMain->insertTab(0,cr,cr->windowTitle());
        ui->tabWidgetMain->setCurrentWidget(cr);
        connect(cr,&PJobWindow::putJobid,cycleThread,&MonitoringCycleThread::doUpdateJob);
        connect(cr,&PJobWindow::putJobid,treeModelConnect,&PStandardItemModel::do_updateJob);
    }

}


void MainWindow::on_actAddConnect_triggered()
{
    PCreateConnectDialog* pc = new PCreateConnectDialog(this);
    pc->setAttribute(Qt::WA_DeleteOnClose);
    connect(pc,&PCreateConnectDialog::putConnectId,JobThreadFactory::jobFactory,&JobThreadFactory::do_updateConnect);
    connect(pc,&PCreateConnectDialog::putConnectId,treeModelConnect,&PStandardItemModel::do_updateConnect);
    pc->show();

}


void MainWindow::on_treeViewRule_doubleClicked(const QModelIndex &index)
{
    if(index.data(Qt::UserRole+2).toString()=="rule_id"){
        qint64 ruleId =  index.data(Qt::UserRole+1).toInt();
        PCreateRuleDialog* pr = new PCreateRuleDialog(ruleId,this);
        pr->setWindowTitle("修改监控规则");
        ui->tabWidgetMain->insertTab(1,pr,pr->windowTitle());
        ui->tabWidgetMain->setCurrentWidget(pr);
        connect(pr,&PCreateRuleDialog::putRuleId,treeModelRule,&PStandardItemModel::do_updateRule);
        connect(pr,&PCreateRuleDialog::putRuleId,JobThreadFactory::jobFactory,&JobThreadFactory::do_updateRule);
    }
}


void MainWindow::on_tabWidgetMain_tabCloseRequested(int index)
{
    QWidget *widget;
    widget = ui->tabWidgetMain->widget(index);
    if(widget!=nullptr){
        widget->close();
        delete widget;
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

}


void MainWindow::on_actAddJob_triggered()
{
    QModelIndex index = ui->treeViewConnect->currentIndex();
    if(index.isValid()){
        if(index.data(Qt::UserRole+2).toString()=="connect_id"){
            PJobWindow * cr = new PJobWindow(true ,index.data(Qt::UserRole+1).toInt(),this);
            ui->tabWidgetMain->insertTab(0,cr,cr->windowTitle());
            ui->tabWidgetMain->setCurrentWidget(cr);
            connect(cr,&PJobWindow::putJobid,cycleThread,&MonitoringCycleThread::doUpdateJob);
            connect(cr,&PJobWindow::putJobid,treeModelConnect,&PStandardItemModel::do_updateJob);

        }else{

        }
    }
}

