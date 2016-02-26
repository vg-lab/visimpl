#include <QPainter>
#include <QBrush>

#include "Gradient.h"

Gradient::Gradient(QWidget *parent_) :
    QFrame(parent_),
    _direction(VERTICAL)
{
     QPixmap pixmap(20, 20);
     QPainter painter(&pixmap);
     painter.fillRect(0, 0, 10, 10, Qt::lightGray);
     painter.fillRect(10, 10, 10, 10, Qt::lightGray);
     painter.fillRect(0, 10, 10, 10, Qt::darkGray);
     painter.fillRect(10, 0, 10, 10, Qt::darkGray);
     painter.end();
     QPalette pal = palette();
     pal.setBrush(backgroundRole(), QBrush(pixmap));
     setPalette(pal);
     setAutoFillBackground(true);
}
    
void Gradient::redGradient()
{
    QGradientStops stops;
    stops << qMakePair(0.0, QColor(255, 0, 0))
          << qMakePair(1.0, QColor(0, 0, 0));
    setGradientStops(stops);
}

void Gradient::greenGradient()
{
    QGradientStops stops;
    stops << qMakePair(0.0, QColor(0, 255, 0))
          << qMakePair(1.0, QColor(0, 0, 0));
    setGradientStops(stops);
}

void Gradient::blueGradient()
{
    QGradientStops stops;
    stops << qMakePair(0.0, QColor(0, 0, 255))
          << qMakePair(1.0, QColor(0, 0, 0));
    setGradientStops(stops);
}

void Gradient::alphaGradient()
{
    QGradientStops stops;
    stops << qMakePair(1.0, QColor(0, 0, 0, 0))
          << qMakePair(0.0, QColor(0, 0, 0, 255));
    setGradientStops(stops);
}

void Gradient::paintEvent(QPaintEvent* /*e*/)
{
    QPainter painter(this);
    QLinearGradient gradient(0, 0, _direction == HORIZONTAL ? width() : 0,
                             _direction == VERTICAL ? height() : 0);
    gradient.setStops(_stops);
    QBrush brush(gradient); 
    painter.fillRect(rect(), brush);
}
