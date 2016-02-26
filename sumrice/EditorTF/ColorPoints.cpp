#include <cassert>

#include <QFrame>
#include <QPainter>
#include <QtCore/QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <qapplication.h>

#include "ColorPoints.h"

/*
  Helper functions
*/
inline static bool pointLessThan(const QPointF &p1, const QPointF &p2)
{
    return p1.x() < p2.x();
}

/* 
   Constructor 
*/
ColorPoints::ColorPoints(QFrame *frame)
    : QObject(frame)
{
    _frame = frame;
    frame->installEventFilter(this);

    _connectionType = LineConnection;

    _pointSize = QSize(11, 11);
    _currentPoint = -1;
    _enabled = true;
    
    connect(this, SIGNAL(pointsChanged(const QPolygonF &)),
            _frame, SLOT(update()));
}

bool ColorPoints::eventFilter(QObject *object, QEvent *event_)
{
    if (object == _frame && _enabled) 
    {
        switch (event_->type()) 
        {
        case QEvent::MouseButtonPress: 
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event_);
            
            QPoint position = mouseEvent->pos();
            /* Checking if a point has been selected. If the current
               mouse position hovers more than one point the selection
               will cycle. */
            int index = -1;
            int firstSelected = -1;
            int oldPoint = _currentPoint;
            for (int i = 0; i < _points.size(); ++i) 
            {
                QPainterPath path;
                path.addEllipse(pointBoundingRect(i));
                if (path.contains(position)) 
                {
                    if (firstSelected == -1)
                        firstSelected = i;
                    if (oldPoint < i && index == -1) 
                    {
                        index = i;
                        break;
                    }
                }
            }
            if (index == -1)
                index = firstSelected;

            if (mouseEvent->button() == Qt::LeftButton) 
            {
                if (index == -1) 
                {
                    QRect rect = _frame->rect();
                    /* Inserting point */
                    float relativeX = position.x() / 
                                      float(rect.width() - 1);
                    for (index = 0; 
                         index < _points.size() && 
                         _points[index].x() < relativeX;
                         ++index)
                        ;

                    QPointF relativePos
                        (relativeX, 1 - position.y() / 
                                        float(rect.height() - 1));
                    _points.insert(index, relativePos);
                    emit pointInserted(index, relativePos.x());
                    updatePointList();
                }
                _currentPoint = index;
            } 
            else if (mouseEvent->button() == Qt::RightButton && index != -1 &&
                     index != 0 && index != _points.size() - 1) 
            {
                removePoint(index);
                emit pointRemoved(index);
            }
            /* Changing selection to current point */
            
            _frame->update();

            return true;
            break;
        }

        case QEvent::MouseMove: {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event_);
            if (_currentPoint >= 0 && mouseEvent->buttons() & Qt::LeftButton)
                movePoint(_currentPoint, mouseEvent->pos());
            return true;
            break;
        }

        case QEvent::Paint: {
            /* Trick to paint background first */
            QFrame *frame = _frame;
            _frame = 0;
            QApplication::sendEvent(object, event_);
            _frame = frame;
            paintPoints();
            return true;
        }
        default:
            break;
        }
    }

    return false;
}


void ColorPoints::paintPoints()
{
    QRect rect = _frame->rect();

    QPainter painter;
    painter.begin(_frame);

    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(QColor(255, 255, 255, 127), 1));
    if (_connectionType == CurveConnection)
    {
        QPainterPath path;
        /* Adding first point */
        QPointF pf(_points[0]);
        QPoint p1((int) (pf.x() * (rect.width() - 1)),
                  (int) ((1 - pf.y()) * (rect.height() - 1)));
        path.moveTo(p1);
        /* Adding the remaining points */
        for (int i = 1; i < _points.size(); ++i)
        {
            QPointF pfaux(_points[i]);
            QPoint p2((int) (pfaux.x() * (rect.width() - 1)),
                      (int) ((1 - pfaux.y()) * (rect.height() - 1)));
            double distance = p2.x() - p1.x();
            path.cubicTo(p1.x() + distance / 2, p1.y(),
                         p1.x() + distance / 2, p2.y(),
                         p2.x(), p2.y());
            p1 = p2;
        }
        painter.drawPath(path);
    }
    else
    {
        /* Creating polyline in widget coordinates */
        QPolygon polyline;
        for (int i = 0; i < _points.size(); ++i)
        {
            QPointF pf(_points[i]);
            QPoint p((int) (pf.x() * (rect.width() - 1)),
                     (int) ((1 - pf.y()) * (rect.height() - 1)));
            polyline << p;
        }
        painter.drawPolyline(polyline);
    }

    painter.setPen(QPen(QColor(255, 255, 255, 191), 1));
    painter.setBrush(QBrush(QColor(191, 191, 191, 127)));
    for (int i=0; i < _points.size(); ++i)
        painter.drawEllipse(pointBoundingRect(i));

    /* Drawing again selected point */
    if (_currentPoint != -1)
    {
        painter.setPen(QPen(QColor(255, 255, 255, 127), 0));
        painter.setBrush(QBrush(QColor(0, 0, 0, 127)));
        painter.drawEllipse(pointBoundingRect(_currentPoint));
    }
}

const QPolygonF &ColorPoints::points() const 
{ 
    return _points; 
}

    void ColorPoints::setPoints(const QPolygonF &points_, bool emitUpdate)
{
    _points = points_;
    _currentPoint = -1;
    qSort(_points.begin(), _points.end(), pointLessThan);
    if (emitUpdate)
        emit pointsChanged(_points);

}

void ColorPoints::updatePointList()
{
    QPointF oldCurrent(-1, -1);
    if (_currentPoint != -1)
        oldCurrent = _points[_currentPoint];
    
    /* First and last points are never sorted */
    qSort(_points.begin() + 1, _points.end() - 1, pointLessThan);
    
    /* Finding the new current point */
    if (_currentPoint != -1)
    {
        for (int i = 0; i < _points.size(); ++i)
        {
            if (_points[i] == oldCurrent)
            {
                _currentPoint = i;
                break;
            }
        }
    }

    emit pointsChanged(_points);
}

void ColorPoints::insertPoint(int index, float x)
{
    QPointF previous = _points[index - 1];
    QPointF next = _points[index];
    float alpha = (x - previous.x()) / (next.x() - previous.x());
    float y = next.y() * alpha + previous.y() * (1 - alpha);
    _points.insert(index, QPointF(x, y));
    if (_currentPoint != -1) 
    {
        if (index <= _currentPoint)
            _currentPoint++;
    }
    emit pointsChanged(_points);
}

void ColorPoints::movePointAbscissa(int index, float value)
{
    _points[index].setX(value);
    updatePointList();
}

void ColorPoints::removePoint(int index)
{
    if (index == _currentPoint)
        _currentPoint = -1;
    _points.remove(index);
    if (_currentPoint != -1)
    {
        if (index < _currentPoint)
            _currentPoint--;
    }
    emit pointsChanged(_points);
}

QRect ColorPoints::pointBoundingRect(int i) const
{
    const QPointF &p = _points[i];
    QRect rect = _frame->rect();
    double x = p.x() * (rect.width() - 1) - _pointSize.width() / 2;
    double y = (1 - p.y()) * (rect.height() - 1) - _pointSize.height() / 2;
    return QRect(x, y, _pointSize.width(), _pointSize.height());
}

void ColorPoints::movePoint(int i, const QPoint &at, bool emitChange)
{
    QRect rect = _frame->rect();        
    QPointF p(at.x() / float(rect.width() - 1),
              1 - at.y() / float(rect.height() - 1));
    p.setX(std::max(0.0, std::min(1.0, p.x())));
    p.setY(std::max(0.0, std::min(1.0, p.y())));
    if (!i) 
    {
        p.setX(0);
    }
    if (i == _points.size() - 1)
    {
        p.setX(1);
    }
    _points[i] = p;
    if (emitChange)
    {
        emit pointAbscissaChanged(i, p.x());
        updatePointList();
    }
}

