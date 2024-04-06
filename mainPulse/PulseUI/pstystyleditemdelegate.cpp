#include "pstystyleditemdelegate.h"
#include<QPainter>
#include<QStandardItem>
PStyStyledItemDelegate::PStyStyledItemDelegate(QObject *parent):QStyledItemDelegate(parent)
{

}

PStyStyledItemDelegate::~PStyStyledItemDelegate()
{

}


void PStyStyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // 绘制背景

    if(index.data(Qt::UserRole+2).toString() == "job_id"){

        QColor backgroundColor(Qt::yellow); // 设置背景色为黄色
        //painter->fillRect(option.rect, backgroundColor);
    }

    QStyledItemDelegate::paint(painter,option,index);
}

QSize PStyStyledItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // QSize size = QStyledItemDelegate::sizeHint(option,index);
    // if(index.data(Qt::UserRole+2).toString() == "job_id"){
    //     int addHeight = index.data(Qt::UserRole+3).toInt();
    //     size.setHeight(size.height()+addHeight);
    // }
    return QStyledItemDelegate::sizeHint(option,index);
}
