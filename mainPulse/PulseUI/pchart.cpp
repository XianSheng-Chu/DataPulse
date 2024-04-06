#include "pchart.h"


PChart::PChart(QGraphicsItem *parent, Qt::WindowFlags wFlags):QChart(parent,wFlags)
{

}

PChart::~PChart()
{

}

void PChart::removeAxis()
{
    if(axisX!= nullptr ) delete axisX;
    if(axisY!= nullptr ) delete axisY;
}

void PChart::removeSeries()
{
    for (QPair<QString,QAbstractSeries*> var : seriesList) {
        if(var.second!=nullptr) delete var.second;
    }
    seriesList.clear();
}

void PChart::attachAllAxis()
{
    addAxis(axisX,Qt::AlignBottom);
    addAxis(axisY,Qt::AlignLeft);

    for (QPair<QString,QAbstractSeries*> var : seriesList) {
        if(var.second!=nullptr) {
            var.second->attachAxis(axisX);
            var.second->attachAxis(axisY);
            addSeries(var.second);
        }
    }
}
