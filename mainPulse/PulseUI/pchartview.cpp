#include "pchartview.h"

PChartView::PChartView(QChart *chart, QWidget *parent):QChartView(chart,parent) {


}

PChartView::PChartView(QWidget *parent):QChartView(parent)
{
}
