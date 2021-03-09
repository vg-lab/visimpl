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

#ifndef __STACKVIZ_SUBSETEVENTWIDGET__
#define __STACKVIZ_SUBSETEVENTWIDGET__

#include <simil/simil.h>
#include <sumrice/api.h>

#include <QFrame>

#include "types.h"

namespace visimpl
{
  class SUMRICE_API EventWidget : public QFrame
  {
    Q_OBJECT;

  public:

    EventWidget( void );

    void timeFrames( std::vector< TEvent >* timeFrames );
    std::vector< TEvent >* timeFrames( void );

    void index( unsigned int index_ );

    void name( const std::string& name_ );
    const std::string& name( void ) const;

    void updateCommonRepSizeVert( unsigned int newHeight );

  protected:

    virtual void resizeEvent( QResizeEvent* event );
    virtual void paintEvent( QPaintEvent* event );

    std::vector< TEvent >* _events;

    unsigned int _index;
    unsigned int _heightPerRow;
    unsigned int _columns;
    unsigned int _centralColumns;
    unsigned int _margin;

    std::string _name;
  };


}



#endif /* __STACKVIZ_SUBSETEVENTWIDGET__ */
