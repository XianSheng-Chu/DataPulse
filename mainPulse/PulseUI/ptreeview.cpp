#include "ptreeview.h"
#include<QEvent>
#include<QMouseEvent>

PTreeView::PTreeView(QWidget *parent) :QTreeView(parent){

}

PTreeView::~PTreeView()
{

}




void PTreeView::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndex index = this->indexAt(event->pos());
    // if((index.data(Qt::UserRole+1).toInt()!=previousIndex.data(Qt::UserRole+1).toInt()) ){
    //     this->model()->setData(previousIndex,0,Qt::UserRole+3);
    //     previousIndex=index.data(Qt::UserRole+1);
    // }
    QTreeView::mouseMoveEvent(event);
}


