#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "TaskSchedule/monitoringcyclethread.h"
#include <QMainWindow>

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
};
#endif // MAINWINDOW_H
