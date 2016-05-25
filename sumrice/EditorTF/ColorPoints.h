#ifndef COLORPOINTS_H
#define COLORPOINTS_H

#include <QFrame>

class ColorPoints : public QObject
{
    Q_OBJECT

    /* Declarations */
public:
    enum TConnectionType {LineConnection, CurveConnection};

    /* Constructor */
public:
    ColorPoints(QFrame *frame);

    bool eventFilter(QObject *object, QEvent *event);

    void paintPoints();

    const QPolygonF &points() const;

    void setPoints(const QPolygonF &points, bool emitUpdate = false);

    TConnectionType connectionType() const 
    {
        return _connectionType; 
    }

    void setConnectionType(TConnectionType connectionType_)
    { 
        _connectionType = connectionType_;
    }

    void setEnabled(bool enabled)
    {
        if (_enabled != enabled) 
        {
            _enabled = enabled;
            _frame->update();
        }
    }

public slots:
    void insertPoint(int index, float valueX);

    void movePointAbscissa(int index, float value);

    void removePoint(int index);

signals:
    void pointsChanged(const QPolygonF &points);
    
    void pointInserted(int index, float valueX);

    void pointAbscissaChanged(int index, float value);

    void pointRemoved(int index);

private:
    void updatePointList();

    QRect pointBoundingRect(int i) const;

    void movePoint(int i, const QPoint &at, bool emitChange = true);

    /* Member attributes */
private:
    QFrame *_frame;

    QPolygonF _points;
    QRect _bounds;

    TConnectionType _connectionType;

    QSize _pointSize;
    int _currentPoint;
    bool _enabled;
};

#endif
