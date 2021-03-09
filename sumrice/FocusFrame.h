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

#ifndef FOCUSFRAME_H_
#define FOCUSFRAME_H_

#include "Histogram.h"

class FocusFrame : public QFrame
{

  Q_OBJECT;

public:

  FocusFrame( QWidget* parent = nullptr );

  void viewRegion( const visimpl::HistogramWidget& histogram,
                   float marker,// float offset,
                   float regionWidth = 0.1f);

  void clear( void );

  virtual void paintEvent(QPaintEvent* event);

  QColor colorLocal( void );
  void colorLocal( const QColor& );

  QColor colorGlobal( void );
  void colorGlobal( const QColor& );

  void fillPlots( bool fillPlots_ );

protected:

  QPolygonF _curveLocal;
  QPolygonF _curveGlobal;
  float _visibleStart;
  float _width;
  float _offset;

  unsigned int _currentPosition;
  unsigned int _firstPointLocal;
  unsigned int _lastPointLocal;

  unsigned int _firstPointGlobal;
  unsigned int _lastPointGlobal;

  QColor _colorLocal;
  QColor _colorGlobal;

  bool _fillPlots;

};



#endif /* FOCUSFRAME_H_ */
