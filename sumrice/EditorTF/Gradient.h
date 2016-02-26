#ifndef GRADIENT_H
#define GRADIENT_H

#include <QFrame>

class Gradient : public QFrame
{
/* Declarations */
public:
    enum Direction {HORIZONTAL, VERTICAL};

/* Constructor */
public:
    Gradient(QWidget *parent = 0);
    virtual ~Gradient( ) {}
/* Member functions */
public:
    void setDirection(Direction direction)
    {
        _direction = direction;
        update();
    }

    void setGradientStops(const QGradientStops &stops)
    {
        _stops = stops;
        update();
    }

    const QGradientStops &getGradientStops() const
    {
        return _stops;
    }

    void redGradient();
    void blueGradient();
    void greenGradient();
    void alphaGradient();

    virtual void paintEvent(QPaintEvent *event);

/* Member attributes */
protected:
    Direction _direction;
    QGradientStops _stops;
};

#endif
