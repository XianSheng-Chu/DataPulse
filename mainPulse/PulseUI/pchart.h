#ifndef PCHART_H
#define PCHART_H



#include <QValueAxis>
#include <QChart>
#include <QObject>
#include<QSplineSeries>
#include<QDateTimeAxis>
class PChart : public QChart
{
    Q_OBJECT
    friend class PJobWindow;
public:
    explicit PChart(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = Qt::WindowFlags());
    ~PChart();
private:
    QDateTimeAxis *axisX=nullptr;
    QValueAxis *axisY=nullptr;
    bool differentialFlag = false;//代表值坐标是否需要微分
    QString chartType;
    QHash<QString,QLineSeries*> seriesList;

private:
    void removeAxis();
    void attachAllAxis();
    void removeSeries();
};

#endif // PCHART_H
