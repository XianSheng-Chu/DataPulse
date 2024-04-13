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



void PChart::attachAllAxis()
{
    addAxis(axisX,Qt::AlignBottom);
    addAxis(axisY,Qt::AlignLeft);


}

void PChart::removeSeries()
{

}
