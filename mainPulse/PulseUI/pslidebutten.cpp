#include "pslidebutten.h"
#include<QPainter>
#include<QPaintEvent>
PSlideButten::PSlideButten(QWidget *parent):QPushButton(parent) {

}

void PSlideButten::paintEvent(QPaintEvent *event)
{
    QPainter *painter = new QPainter(this);
    painter->setRenderHint(QPainter::Antialiasing, true);
    qint64 W=this->width();
    qint64 H=this->height();
    QPen pen;
    pen.setWidth(3);
    pen.setColor(Qt::blue);

    QRect rect(0,0,H-2*pen.width(),H-2*pen.width());
    painter->setBrush(QBrush(Qt::blue));
    painter->drawEllipse(rect);

    event->accept();
}
