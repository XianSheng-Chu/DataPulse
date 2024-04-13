#ifndef PJOBWINDOW_H
#define PJOBWINDOW_H

#include <QMainWindow>
#include <TaskSchedule/pdatabasesoure.h>
#include<QChart>
#include<QDateTime>
#include "pchartview.h"
#include"pchart.h"
#include<QSqlQueryModel>
#include<QTimer>
namespace Ui {
class PJobWindow;
}

class PJobWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PJobWindow(QWidget *parent = nullptr);
    explicit PJobWindow(qint64 jobId,QWidget *parent = nullptr);
    explicit PJobWindow(bool creatFlag, qint64 id, QWidget *parent = nullptr);
    ~PJobWindow();

private slots:
    void on_butBox_rejected();

    void on_butBox_accepted();

    void on_combTimeRang_currentIndexChanged(int index);

    void on_tabBatch_doubleClicked(const QModelIndex &index);

    void on_btnRunNow_clicked();

    void on_btnRunNow_3_clicked();

private:
    Ui::PJobWindow *ui;

    QSqlDatabase m_metaDB;
    QSqlDatabase m_connectDB;
    qint64 m_jobId;
    qint64 m_ruleId;
    qint64 m_connectId;
    bool m_creatFlag = false;
    PChart *m_chart = nullptr;
    QHash<QString,qint64> m_RuleIdMap;
    QHash<QString,QVariant> m_rule;
    QHash<QString,QVariant> m_connect;
    QHash<QString,QVariant> m_jobInfo;
    QHash<QString,QString> m_colMapper;
    QList<QHash<QString,QVariant>> m_butch;
    QList<QHash<QString,QVariant>> m_result;
    QDateTime m_RuleLastUpdate;
    QList<QPair<QString,QString>> m_sortCol;
    QList<QPair<QString,PChartView*>>  cahrtViewList;
    QList<QPair<QString,PChart*>>  cahrtList;
    static const QHash<QString,QString> mapperType;
    QDateTime dataRangMin;
    QSqlQueryModel *m_butchModel;
    QSqlQueryModel *m_resultModel;
    QTimer *timer= nullptr;
    QList<QPair<QDateTime,QSqlRecord>> RealtimeDatas;
    QHash<QString,QPair<QDateTime,QSqlRecord>> RealPrevious;
private:
    void init();
    void initMapper();
    void initButch();
    void initResult(qint64 butchId);
    void initChart();
    void initRule(qint64 ruleId);
    void initConnect(qint64 connectId);
    void readData(QList<QHash<QString,QVariant>> &listMap,QSqlQuery &query);
    void readData(QHash<QString,QVariant> &map,QSqlRecord record);
    void testChart();

private slots:
    void do_chartValue();
    void on_btnSlide_clicked(bool checked);

signals:
    void putJobid(qint64 ruleId);
    void runJob(quint64 jobId,quint64 connectId = -1,quint64 ruleId = -1);
    void do_close();
};

#endif // PJOBWINDOW_H
