/*
 * CustomSlider.h
 *
 *  Created on: 11 de dic. de 2015
 *      Author: sgalindo
 */

#ifndef __QT_CUSTOMSLIDER_H__
#define __QT_CUSTOMSLIDER_H__

#include <QMouseEvent>
#include <QSlider>

class CustomSlider : public QSlider
{
public:

  CustomSlider( enum Qt::Orientation _orientation = Qt::Horizontal,
                QWidget* _parent = nullptr )
  : QSlider( _orientation, _parent )
  { }

protected:

  void mousePressEvent ( QMouseEvent * _event )
    {
      if (_event->button() == Qt::LeftButton)
      {
          if (orientation() == Qt::Vertical)
              setValue(minimum() + ((maximum()-minimum()) * (height()-_event->y())) / height() ) ;
          else
              setValue(minimum() + ((maximum()-minimum()) * _event->x()) / width() ) ;

          _event->accept();
      }
      QSlider::mousePressEvent(_event);
    }

};



#endif /* SRC_QT_CUSTOMSLIDER_H_ */
