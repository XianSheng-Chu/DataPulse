#ifndef PSTANDARDITEMMODEL_H
#define PSTANDARDITEMMODEL_H

#include "qsqldatabase.h"
#include <QStandardItemModel>
#include <QWidget>

class PStandardItemModel : public QStandardItemModel
{
    Q_OBJECT
public:
    PStandardItemModel(QWidget *parent= nullptr);
    PStandardItemModel(int rows, int columns, QObject *parent = nullptr);
    ~PStandardItemModel();
    void init(int index);
    void setModelDim();//设置模型展示维度
    void setModelDim(QStringList dim);//设置模型展示维度
private:
    QSqlDatabase mateDB;
    QHash<qint64,QHash<QString,QVariant>>* mateData = nullptr;
    qint64 widgetIndex;
    QString idName;
    QStringList m_dim;
    void traverseTree(qint64 id,QString idType,qint64 level,bool deleteFlag = false);
public slots:
    void do_updateConnect(qint64 id);
    void do_updateRule(qint64 id);
    void do_updateJob(qint64 id);
};

#endif // PSTANDARDITEMMODEL_H
