#ifndef PCHARTVIEW_H
#define PCHARTVIEW_H

#include <QChartView>
#include <QWidget>
#include<TaskSchedule/pdatabasesoure.h>
class PChartView : public QChartView
{
    Q_OBJECT
public:
    PChartView(QChart *chart, QWidget *parent = nullptr);
    PChartView(QWidget *parent = nullptr);

};

#endif // PCHARTVIEW_H
