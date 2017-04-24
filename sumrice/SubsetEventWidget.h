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

    void timeFrames( std::vector< TEvent >* timeFrames );
    std::vector< TEvent >* timeFrames( void );

    void index( unsigned int index_ );

  protected:

    virtual void resizeEvent( QResizeEvent* event );
    virtual void paintEvent( QPaintEvent* event );

    std::vector< TEvent >* _events;

    unsigned int _index;
    unsigned int _heightPerRow;
    unsigned int _columns;
    unsigned int _centralColumns;
    unsigned int _margin;
  };


}



#endif /* __STACKVIZ_SUBSETEVENTWIDGET__ */
