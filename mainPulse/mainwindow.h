#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "TaskSchedule/monitoringcyclethread.h"
#include <QMainWindow>
#include<QSqlDatabase>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE
class PStandardItemModel;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initThread();
    void initUI();
    void initDock();
private:
    Ui::MainWindow *ui;
    QSqlDatabase m_mateDB;
private:
    MonitoringCycleThread *cycleThread = nullptr;//后台循环事件的处理
    PStandardItemModel * treeModelConnect = nullptr;
    PStandardItemModel * treeModelRule = nullptr;
private slots:
    void do_runJob(quint64 jobId,quint64 connectId = -1,quint64 ruleId = -1);
    void on_treeViewConnect_clicked(const QModelIndex &index);
    void on_treeViewConnect_entered(const QModelIndex &index);
    void on_treeViewConnect_doubleClicked(const QModelIndex &index);
    void on_actAddConnect_triggered();
    void on_treeViewRule_doubleClicked(const QModelIndex &index);
    void on_tabWidgetMain_tabCloseRequested(int index);
    void on_actAddRule_triggered();
    void on_actAddJob_triggered();
    void on_actDelConnect_triggered();
    void on_actEditConnect_triggered();


    void on_actEditRule_triggered();

    void on_actDelJob_triggered();

    void on_actDelRule_triggered();

    void on_closeTab();
    void on_actEditJob_triggered();

    void on_actCheckJob_triggered();

signals:
    void putJobid(qint64 id);
    void putRuleId(qint64 id);
    void putConnectId(qint64 id);
    void deleteJobid(qint64 id);
    void deleteConnectid(qint64 id);
    void deleteRuleid(qint64 id);

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};


#endif // MAINWINDOW_H
