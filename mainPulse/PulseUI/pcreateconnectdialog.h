#ifndef PCREATECONNECTDIALOG_H
#define PCREATECONNECTDIALOG_H

#include <QDialog>
#include<QTcpSocket>
#include<QSqlDatabase>
#include<QThread>
namespace Ui {
class PCreateConnectDialog;

}
class DBConnectTestThread;
class PCreateConnectDialog : public QDialog
{
    Q_OBJECT
    friend class DBConnectTestThread;
public:
    explicit PCreateConnectDialog(QWidget *parent = nullptr);
    explicit PCreateConnectDialog(qint64 id,QWidget *parent = nullptr);

    ~PCreateConnectDialog();
private:
    void insertConnect();
    void updataConnect();
private slots:
    void on_editHost_editingFinished();

    void on_editPort_editingFinished();

    void do_errorScoket(QAbstractSocket::SocketError error);
    void do_connectedScoket();
    void on_editConnectName_editingFinished();


    void on_editUserName_editingFinished();

    void on_editDBName_editingFinished();

    void on_btnTest_clicked();

    void on_editPassword_editingFinished();

    void on_btnOK_clicked();

    void on_listWidget_clicked(const QModelIndex &index);

private:
    Ui::PCreateConnectDialog *ui;
    QHash<QString,QVariant> data;
    QTcpSocket* socket;
    QSqlDatabase mateDB;
    bool connValidity = false;
    bool connNameValidity = false;
    bool updataFlag = false;
    DBConnectTestThread* connTest= nullptr;
private slots:
    void do_openFlag(bool flag,QString info);
signals:
    void putConnectId(qint64 id);
};

class DBConnectTestThread : public QThread
{
    Q_OBJECT
public:
    DBConnectTestThread(QSqlDatabase DB,QObject *parent = nullptr);
    ~DBConnectTestThread();
    // QThread interface
protected:
    virtual void run() override;

private:
    QSqlDatabase DBConnect;
signals:
    void openFlag(bool flag,QString info);
};



#endif // PCREATECONNECTDIALOG_H
