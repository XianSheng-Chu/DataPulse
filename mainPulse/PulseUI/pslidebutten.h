#ifndef PSLIDEBUTTEN_H
#define PSLIDEBUTTEN_H

#include <QPushButton>
#include <QWidget>

class PSlideButten : public QPushButton
{
    Q_OBJECT
public:
    PSlideButten(QWidget *parent = nullptr);


    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
};



#endif // PSLIDEBUTTEN_H
