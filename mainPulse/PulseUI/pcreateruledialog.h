#ifndef PCREATERULEDIALOG_H
#define PCREATERULEDIALOG_H

#include <QDialog>
#include<TaskSchedule/pdatabasesoure.h>
#include<QThread>
namespace Ui {
class PCreateRuleDialog;
}
class RuleTestThread;
class PCreateRuleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PCreateRuleDialog(QWidget *parent = nullptr,qint64 connectId= -1);
    ~PCreateRuleDialog();
    explicit PCreateRuleDialog(qint64 ruleId,QWidget *parent = nullptr);

private slots:
    void on_combDBType_currentIndexChanged(int index);

    void on_btnRun_clicked();

private:
    QSqlDatabase metaDB;
    QSqlDatabase testDB;
    Ui::PCreateRuleDialog *ui;
    QHash<QString,QVariant> row;
    QStringList DBTypes;
    QHash<QString,QStringList> DBNames;
    RuleTestThread* ruleTestThread;
    bool updateFlag = false;
    bool testFlag = false;
    bool nameTestFlag = true;
    QString script;
    QHash<QString,QString> m_columnMapper;
    qint64 m_ruleId;
    qint64 m_connectId;

private:
    void init();
    void connectInit();
    void createRule();
    void updateRule();
 private slots:
    void do_TestFlag(bool flag,QString info);
    void on_editScript_textChanged();
    void on_btnOK_clicked();
    void on_combConnectName_currentTextChanged(const QString &arg1);
    void on_editName_editingFinished();
    void on_btnClose_clicked();

signals:
    void putRuleId(qint64 id);
};

class RuleTestThread : public QThread{
    Q_OBJECT
public:
    RuleTestThread(QHash<QString, QString> &m_columnMapper, QObject *parent = nullptr);

private:
    QSqlDatabase m_testDB;
    QString m_script;
    QHash<QString,QString> &m_columnMapper;
public:
    void setAll(QSqlDatabase DB,QString script);

private:
    bool initRuleMapper(QSqlRecord record,QString &error);

    // QThread interface
protected:
    virtual void run() override;

signals:
    void testFlag(bool flag,QString info);
};



#endif // PCREATERULEDIALOG_H
