#ifndef PTREEVIEW_H
#define PTREEVIEW_H

#include <QTreeView>
#include <QWidget>
class PTreeView : public QTreeView
{
    Q_OBJECT
public:
    PTreeView(QWidget *parent);
    ~PTreeView();
    QVariant previousIndex;
    // QObject interface
public:

    // QWidget interface
protected:
    virtual void mouseMoveEvent(QMouseEvent *event) override;

};




#endif // PTREEVIEW_H
