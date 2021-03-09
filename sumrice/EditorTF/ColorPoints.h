/*
 * Copyright (c) 2015-2020 VG-Lab/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

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
