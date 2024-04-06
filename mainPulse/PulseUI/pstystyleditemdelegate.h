#ifndef PSTYSTYLEDITEMDELEGATE_H
#define PSTYSTYLEDITEMDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include<QStyleOptionViewItem>
#include<QModelIndex>
class PStyStyledItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit PStyStyledItemDelegate(QObject *parent = nullptr);
    ~PStyStyledItemDelegate();

    // QAbstractItemDelegate interface
public:
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // QAbstractItemDelegate interface
public:
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};





#endif // PSTYSTYLEDITEMDELEGATE_H
