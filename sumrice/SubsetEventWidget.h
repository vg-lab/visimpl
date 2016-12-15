/*
 * @file	SubsetEventWidget.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#ifndef __STACKVIZ_SUBSETEVENTWIDGET__
#define __STACKVIZ_SUBSETEVENTWIDGET__

#include <simil/simil.h>

#include <QFrame>

#include "types.h"

namespace visimpl
{
  class SubsetEventWidget : public QFrame
  {
    Q_OBJECT;

  public:

    SubsetEventWidget( void );

//    virtual void mousePressEvent( QMouseEvent* event_ );
//    virtual void mouseReleaseEvent( QMouseEvent* event_ );
//    virtual void mouseMoveEvent( QMouseEvent* event_ );

    void timeFrames( std::vector< TimeFrame >* timeFrames );
    std::vector< TimeFrame >* timeFrames( void );


  protected:

    virtual void paintEvent( QPaintEvent* event );

//    simil::SubsetEventManager* _subsetEventMngr;
    std::vector< TimeFrame >* _timeFrames;

    unsigned int _heightPerRow;
    unsigned int _columns;
    unsigned int _centralColumns;
    unsigned int _margin;
  };


}



#endif /* __STACKVIZ_SUBSETEVENTWIDGET__ */
