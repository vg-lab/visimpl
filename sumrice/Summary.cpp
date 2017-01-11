/*
 * @file  Summary.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "Summary.h"

#include <QMouseEvent>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QInputDialog>
#include <QSpinBox>
#include <QScrollBar>
#include <QApplication>

unsigned int visimpl::Selection::_counter = 0;

unsigned int DEFAULT_BINS = 250;
float DEFAULT_ZOOM_FACTOR = 1.5f;

static QString colorScaleToString( visimpl::TColorScale colorScale )
{
  switch( colorScale )
  {
    case visimpl::T_COLOR_LINEAR:
      return QString( "Linear" );
//    case visimpl::T_COLOR_EXPONENTIAL:
//      return QString( "Exponential" );
    case visimpl::T_COLOR_LOGARITHMIC:
      return QString( "Logarithmic");
    default:
      return QString( );
  }
}

namespace visimpl
{

  Summary::Summary( QWidget* parent_,
                    TStackType stackType )
  : QWidget( parent_ )
  , _bins( DEFAULT_BINS )
  , _zoomFactor( DEFAULT_ZOOM_FACTOR )
  , _gridLinesNumber( 3 )
  , _spikeReport( nullptr )
  , _voltageReport( nullptr )
  , _mainHistogram( nullptr )
  , _detailHistogram( nullptr )
  , _focusedHistogram( nullptr )
  , _histogramScroll( nullptr )
  , _mousePressed( false )
  , _stackType( stackType )
  , _colorScaleLocal( visimpl::T_COLOR_LINEAR )
  , _colorScaleGlobal( visimpl::T_COLOR_LOGARITHMIC )
  , _colorLocal( 0, 0, 128, 50 )
  , _colorGlobal( 255, 0, 0, 100 )
  , _subsetEventManager( nullptr )
  , _subsetLayout( nullptr )
  , _subsetScroll( nullptr )
  , _syncScrollsVertically( true )
  , _heightPerRow( 50 )
  , _maxLabelWidth( 100 )
  , _currentCentralMinWidth( 200 )
  , _showMarker( false )
  , _regionPercentage( 0.0f )
  , _regionWidthPixels( -1 )
  , _overRegionEdgeLower( false )
  , _selectedEdgeLower( false )
  , _regionEdgePointLower( -1 )
  , _regionEdgeLower( 0.0f )
  , _overRegionEdgeUpper( false )
  , _selectedEdgeUpper( false )
  , _regionEdgePointUpper( -1 )
  , _regionEdgeUpper( 0.0f )
  , _autoNameSelection( false )
  {

    setMouseTracking( true );

    if( _stackType == T_STACK_FIXED )
    {
      _mainLayout = new QGridLayout( );
      this->setLayout( _mainLayout );
    }
    else if( _stackType == T_STACK_EXPANDABLE )
    {
      _maxColumns= 20;
      _regionWidth = 0.1f;
      _summaryColumns = _maxColumns - 2;

      QVBoxLayout* upperLayout = new QVBoxLayout( );
      upperLayout->setAlignment( Qt::AlignTop );

      _subsetLayout = new QGridLayout( );
      _subsetLayout->setAlignment( Qt::AlignTop );
      _subsetLayout->setVerticalSpacing( 0 );

      _subsetScroll = new QScrollArea();
      QWidget* subsetWidget = new QWidget( );

      subsetWidget->setLayout( _subsetLayout );
      _subsetScroll->setWidget( subsetWidget );
      _subsetScroll->setWidgetResizable( true );
      _subsetScroll->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
      _subsetScroll->setVisible( false );
      _subsetScroll->setMaximumHeight( 200 );

      connect( _subsetScroll->horizontalScrollBar( ),
               SIGNAL( sliderMoved( int )), this, SLOT( moveScrollSync( int )));

      QWidget* header = new QWidget( );
      QGridLayout* headerLayout = new QGridLayout( );
      headerLayout->addWidget( new QLabel( "Name" ), 0, 0, 1, 1);
      headerLayout->addWidget( new QLabel( "Activity" ), 0, 1, 1, _summaryColumns);
      headerLayout->addWidget( new QLabel( "Select" ), 0, _maxColumns - 1, 1, 1);

      header->setLayout( headerLayout );

      _mainLayout = new QGridLayout( );
      _mainLayout->setAlignment( Qt::AlignTop );
      _mainLayout->setVerticalSpacing( 0 );

      _body = new QWidget( );
      _body->setLayout( _mainLayout );

      QWidget* foot = new QWidget( );
      QGridLayout* footLayout = new QGridLayout( );

      QStringList csItems;
      csItems.push_back( QString( colorScaleToString( visimpl::T_COLOR_LINEAR )));
    //  csItems.push_back( QString( colorScaleToString( visimpl::T_COLOR_EXPONENTIAL )));
      csItems.push_back( QString( colorScaleToString( visimpl::T_COLOR_LOGARITHMIC )));

      QComboBox* localComboBox = new QComboBox( );
      localComboBox->addItems( csItems );

      QComboBox* globalComboBox = new QComboBox( );
      globalComboBox->addItems( csItems );

      QPushButton* removeButton = new QPushButton( "Delete" );

      _focusWidget = new FocusFrame( );
      _focusWidget->colorLocal( _colorLocal );
      _focusWidget->colorGlobal( _colorGlobal );
    //  _detailHistogram = new visimpl::Histogram( );

      _localColorWidget = new QWidget( );
      _localColorWidget->setPalette( QPalette( _colorLocal ));
      _localColorWidget->setAutoFillBackground( true );
      _localColorWidget->setMaximumWidth( 30 );
      _localColorWidget->setMinimumHeight( 30 );

      _globalColorWidget = new QWidget( );
      _globalColorWidget->setPalette( QPalette( _colorGlobal ));
      _globalColorWidget->setAutoFillBackground( true );
      _globalColorWidget->setMaximumWidth( 30 );
      _globalColorWidget->setMinimumHeight( 30 );

      _currentValueLabel = new QLabel( "" );
      _currentValueLabel->setMaximumWidth( 50 );
      _globalMaxLabel = new QLabel( "" );
      _globalMaxLabel->setMaximumWidth( 50 );
      _localMaxLabel = new QLabel( "" );
      _localMaxLabel->setMaximumWidth( 50 );

      QSpinBox* binSpinBox = new QSpinBox( );
      binSpinBox->setMinimum( 50 );
      binSpinBox->setMaximum( 100000 );
      binSpinBox->setSingleStep( 50 );
      binSpinBox->setValue( _bins );

      QDoubleSpinBox* zoomFactorSpinBox = new QDoubleSpinBox( );
      zoomFactorSpinBox->setMinimum( 1.0 );
      zoomFactorSpinBox->setMaximum( 1000.0 );
      zoomFactorSpinBox->setSingleStep( 0.5 );
      zoomFactorSpinBox->setValue( _zoomFactor );

      QSpinBox* gridSpinBox = new QSpinBox( );
      gridSpinBox->setMinimum( 0 );
      gridSpinBox->setMaximum( 10000 );
      gridSpinBox->setSingleStep( 1 );
      gridSpinBox->setValue( 3 );

    //  unsigned int totalRows = 10;
      footLayout->addWidget( new QLabel( "Local normalization:" ), 0, 0, 1, 1);
      footLayout->addWidget( _localColorWidget, 0, 1, 1, 1 );
      footLayout->addWidget( localComboBox, 1, 0, 1, 2 );

      footLayout->addWidget( new QLabel( "Global normalization:" ), 2, 0, 1, 1);
      footLayout->addWidget( _globalColorWidget, 2, 1, 1, 1 );
      footLayout->addWidget( globalComboBox, 3, 0, 1, 2 );
      footLayout->addWidget( new QLabel( "Rule lines: " ), 4, 0, 1, 1 );
      footLayout->addWidget( gridSpinBox, 4, 1, 1, 1 );

      footLayout->addWidget( _focusWidget, 0, 2, 5, 7 );

      footLayout->addWidget( new QLabel( "Bins:" ), 0, 9, 1, 1 );
      footLayout->addWidget( binSpinBox, 0, 10, 1, 1 );
      footLayout->addWidget( new QLabel( "ZoomFactor:" ), 1, 9, 1, 1 );
      footLayout->addWidget( zoomFactorSpinBox, 1, 10, 1, 1 );
      footLayout->addWidget( removeButton, 0, 11, 1, 1 );
      footLayout->addWidget( new QLabel( "Current value: "), 2, 9, 1, 2 );
      footLayout->addWidget( _currentValueLabel, 2, 11, 1, 1 );
      footLayout->addWidget( new QLabel( "Local max: "), 3, 9, 1, 2 );
      footLayout->addWidget( _localMaxLabel, 3, 11, 1, 1 );
      footLayout->addWidget( new QLabel( "Global max: "), 4, 9, 1, 2 );
      footLayout->addWidget( _globalMaxLabel, 4, 11, 1, 1 );

      localComboBox->setCurrentIndex( ( int ) _colorScaleLocal );
      globalComboBox->setCurrentIndex( ( int ) _colorScaleGlobal );

      connect( localComboBox, SIGNAL( currentIndexChanged( int ) ),
               this, SLOT( colorScaleLocal( int )));

      connect( globalComboBox, SIGNAL( currentIndexChanged( int ) ),
                 this, SLOT( colorScaleGlobal( int )));

      connect( removeButton, SIGNAL( clicked( void )),
               this, SLOT( removeSelections( void )));

      connect( binSpinBox, SIGNAL( valueChanged( int )),
               this,  SLOT( bins( int )));

      connect( zoomFactorSpinBox, SIGNAL( valueChanged( double )),
               this,  SLOT( zoomFactor( double )));

      connect( gridSpinBox, SIGNAL( valueChanged( int )),
               this, SLOT( gridLinesNumber( int )));

      foot->setLayout( footLayout );

    ////  std::cout << width( ) << std::endl;
    //  _body->setMinimumWidth( width( ));
    ////  body->setMaximumWidth( width( ));
      _histogramScroll = new QScrollArea( );
      _histogramScroll->setWidget( _body );
      _histogramScroll->setWidgetResizable( true );
      _histogramScroll->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

      connect( _histogramScroll->horizontalScrollBar( ),
               SIGNAL( sliderMoved( int )), this, SLOT( moveScrollSync( int )));

      upperLayout->addWidget( _subsetScroll );
  //    upperLayout->addWidget( header );
      upperLayout->addWidget( _histogramScroll );
      upperLayout->addWidget( foot );
    //  upperLayout->addWidget( _body );

      this->setLayout( upperLayout );

    #ifdef VISIMPL_USE_ZEROEQ

      _insertionTimer.setSingleShot( false );
      _insertionTimer.setInterval( 250 );
      connect( &_insertionTimer, SIGNAL( timeout( )),
               this, SLOT(deferredInsertion( )));
      _insertionTimer.start( );


  #endif
    }
  }

  void Summary::Init( simil::SpikeData* spikes_,
                      const simil::TGIDSet& gids_,
                      simil::SubsetEventManager* subsetEventMng )
  {
    _spikeReport = spikes_;
    _gids = GIDUSet( gids_.begin( ), gids_.end( ));

    _subsetEventManager = subsetEventMng;

    Init( );
  }

  void Summary::Init( void )
  {

    _mainHistogram = new visimpl::MultiLevelHistogram( *_spikeReport );
    _mainHistogram->setMinimumHeight( _heightPerRow );
    _mainHistogram->setMaximumHeight( _heightPerRow );
    _mainHistogram->colorScaleLocal( _colorScaleLocal );
    _mainHistogram->colorScaleGlobal( _colorScaleGlobal );
    _mainHistogram->colorLocal( _colorLocal );
    _mainHistogram->colorGlobal( _colorGlobal );
    _mainHistogram->representationMode( visimpl::T_REP_CURVE );
    _mainHistogram->regionWidth( _regionWidth );
    _mainHistogram->gridLinesNumber( _gridLinesNumber );
    _mainHistogram->firstHistogram( true );
    _mainHistogram->setMinimumWidth( _currentCentralMinWidth );
    _focusedHistogram = _mainHistogram;

      TColorMapper colorMapper;
      colorMapper.Insert(0.0f, glm::vec4( 157, 206, 111, 255 ));
      colorMapper.Insert(0.25f, glm::vec4( 125, 195, 90, 255 ));
      colorMapper.Insert(0.50f, glm::vec4( 109, 178, 113, 255 ));
      colorMapper.Insert(0.75f, glm::vec4( 76, 165, 86, 255 ));
      colorMapper.Insert(1.0f, glm::vec4( 63, 135, 61, 255 ));

      _mainHistogram->colorMapper( colorMapper );

      _mainHistogram->mousePosition( &_lastMousePosition );
      _mainHistogram->regionPosition( &_regionPercentage );
      connect( _mainHistogram, SIGNAL( mousePositionChanged( QPoint )),
               this, SLOT( updateMouseMarker( QPoint )));



    if( _stackType == T_STACK_FIXED)
    {
      _mainLayout->addWidget( _mainHistogram, 0, 1, 1, 1 );
      _mainHistogram->paintRegion( false );
      _histograms.push_back( _mainHistogram );
    }
    else if( _stackType == T_STACK_EXPANDABLE )
    {

      StackRow mainRow;

      mainRow.histogram = _mainHistogram;
      mainRow.histogram->_timeFrames = &_timeFrames;
      QString labelText( "All" );
      mainRow.label = new QLabel( labelText );
      mainRow.label->setMinimumWidth( _maxLabelWidth );
      mainRow.label->setMaximumWidth( _maxLabelWidth );
  //    mainRow.label->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
      mainRow.checkBox = new QCheckBox( );
  //    mainRow.checkBox->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

      unsigned int row = _histograms.size( );
      _mainLayout->addWidget( mainRow.label, row , 0, 1, 1 );
      _mainLayout->addWidget( _mainHistogram, row, 1, 1, _summaryColumns );
      _mainLayout->addWidget( mainRow.checkBox, row, _maxColumns - 1, 1, 1 );
    //  mainRow.checkBox->setVisible( false );

      _rows.push_back( mainRow );
      _histograms.push_back( _mainHistogram );

    }
    //  AddGIDSelection( gids );

    connect( _mainHistogram, SIGNAL( mousePressed( QPoint, float )),
             this, SLOT( childHistogramPressed( QPoint, float )));

    connect( _mainHistogram, SIGNAL( mouseReleased( QPoint, float )),
             this, SLOT( childHistogramReleased( QPoint, float )));

    connect( _mainHistogram, SIGNAL( mouseModifierPressed( float, Qt::KeyboardModifiers )),
             this, SLOT( childHistogramClicked( float, Qt::KeyboardModifiers )));

    _mainHistogram->init( _bins, _zoomFactor );

    if( _subsetEventManager )
    {
      _subsetScroll->setVisible( true );

      simil::TimeFrameRange timeFrames = _subsetEventManager->timeFrames( );

      float invTotal = 1.0f / ( _spikeReport->endTime( ) - _spikeReport->startTime( ));

      float startPercentage;
      float endPercentage;

      unsigned int counter = 0;
      for( auto it = timeFrames.first; it != timeFrames.second; ++it, ++counter )
      {
        TimeFrame timeFrame;
        timeFrame.name = it->first;

//        std::cout << "Parsing time frame " << timeFrame.name << std::endl;

        for( auto time : it->second )
        {

          startPercentage =
              std::max( 0.0f,
                        ( time.first - _spikeReport->startTime( )) * invTotal);

          endPercentage =
              std::min( 1.0f,
                        ( time.second - _spikeReport->startTime( )) * invTotal);

          timeFrame.percentages.push_back(
              std::make_pair( startPercentage, endPercentage ));

//          std::cout << "Region from " << startPercentage
//                            << " to " << endPercentage
//                            << std::endl;

        }

        QColor color( rand( ) % 256, rand( ) % 256, rand( ) % 256, 255 );
        timeFrame.color = color;


        std::cout << "Using color: " << color.red( )
                      << ", " << color.green( )
                      << ", " << color.blue( )
                      << std::endl;

        _timeFrames.push_back( timeFrame );

        QLabel* label = new QLabel( timeFrame.name.c_str( ));
        label->setMinimumHeight( 20 );
        label->setMinimumWidth( _maxLabelWidth );
        label->setMaximumWidth( _maxLabelWidth );
        label->setToolTip( timeFrame.name.c_str( ));

        SubsetEventWidget* subsetWidget = new SubsetEventWidget( );
        subsetWidget->setSizePolicy( QSizePolicy::Expanding,
                                           QSizePolicy::Expanding );
        subsetWidget->timeFrames( &_timeFrames );
        subsetWidget->setMinimumWidth( _currentCentralMinWidth );
        subsetWidget->index( counter );

        QCheckBox* checkbox = new QCheckBox();

        TimeFrameRow row;
        row.widget = subsetWidget;
        row.label = label;
        row.checkBox = checkbox;

        _subsetRows.push_back( row );
        _subsetEventWidgets.push_back( subsetWidget );

        _subsetLayout->addWidget( label, counter, 0, 1, 1 );
        _subsetLayout->addWidget( subsetWidget, counter, 1, 1, _summaryColumns );
        _subsetLayout->addWidget( checkbox, counter, _maxColumns, 1, 1 );

      }

      simil::GIDMapRange subsets = _subsetEventManager->subsets( );

      for( auto it = subsets.first; it != subsets.second; ++it )
      {
        GIDUSet subset( it->second.begin( ), it->second.end( ));
        insertSubset( it->first, subset );
      }

    }
    else
    {
      if( _subsetScroll )
        _subsetScroll->setVisible( false );
    }

  //  CreateSummarySpikes( );
  //  UpdateGradientColors( );
    update( );
  }

  void Summary::AddNewHistogram( const visimpl::Selection& selection
  #ifdef VISIMPL_USE_ZEROEQ
                         , bool deferred
  #endif
                         )
  {
    const GIDUSet& selected = selection.gids;

    if( selected.size( ) == _gids.size( ) || selected.size( ) == 0)
      return;


    if( _stackType == TStackType::T_STACK_EXPANDABLE )
    {

      std::cout << "Adding new selection with size " << selected.size( ) << std::endl;
  #ifdef VISIMPL_USE_ZEROEQ

      if( deferred )
      {

        _pendingSelections.push_back( selection );

  //      if( !_insertionTimer.isActive( ))
  //        _insertionTimer.start( );

      }
      else
  #endif

      {
      visimpl::MultiLevelHistogram* histogram = new visimpl::MultiLevelHistogram( *_spikeReport );
  //    histogram->filteredGIDs( gids );
      histogram->colorMapper( _mainHistogram->colorMapper( ));
  //    histogram->colorScale( visimpl::Histogram::T_COLOR_EXPONENTIAL );
  //    histogram->normalizeRule( visimpl::Histogram::T_NORM_MAX );
  //    histogram->mousePosition( &_lastMousePosition );
      histogram->setMinimumHeight( _heightPerRow );
      histogram->setMaximumHeight( _heightPerRow );
  //    histogram->setMinimumWidth( 500 );
      std::cout << "Thread " << this->thread( ) << std::endl;
      _mainLayout->addWidget( histogram, _histograms.size( ), 0, _maxColumns - 2, 1 );
  //    _mainLayout->addWidget( histogram );
      _histograms.push_back( histogram );

      histogram->init( _bins, _zoomFactor );
  //    CreateSummarySpikes( );
  //    UpdateGradientColors( );

      update( );

      }

    }


  }

  #ifdef VISIMPL_USE_ZEROEQ
  void Summary::deferredInsertion( void )
  {
  //  std::cout << "Deferred call " << thread( ) << std::endl;

    if( _pendingSelections.size( ) > 0 )
    {


      visimpl::Selection selection = _pendingSelections.front( );
      _pendingSelections.pop_front( );

      QString labelText =
          QString( "Selection-").append( QString::number( selection.id ));

      bool ok = true;
      if ( !_autoNameSelection )
        labelText = QInputDialog::getText( this,
                                           tr( "Selection Name" ),
                                           tr( "Please, introduce selection name: "),
                                           QLineEdit::Normal,
                                           labelText,
                                           &ok );
      if( !ok )
        return;

      std::cout << "Deferred insertion: " << selection.name
                << " GIDs: " << selection.gids.size( )
                << std::endl;

      selection.name = labelText.toStdString( );

      insertSubset( selection );
    }

  }

  #endif

  void Summary::insertSubset( const Selection& selection )
  {
    insertSubset( selection.name, selection.gids );
  }

  void Summary::insertSubset( const std::string& name, const GIDUSet& subset )
  {
    StackRow currentRow;

    visimpl::MultiLevelHistogram* histogram =
        new visimpl::MultiLevelHistogram( *_spikeReport );

    histogram->filteredGIDs( subset );
    histogram->colorMapper( _mainHistogram->colorMapper( ));
    histogram->colorScaleLocal( _colorScaleLocal );
    histogram->colorScaleGlobal( _colorScaleGlobal );
    histogram->colorLocal( _colorLocal );
    histogram->colorGlobal( _colorGlobal );
    histogram->normalizeRule( visimpl::T_NORM_MAX );
    histogram->representationMode( visimpl::T_REP_CURVE );
    histogram->regionWidth( _regionWidth );
    histogram->gridLinesNumber( _gridLinesNumber );

    histogram->setMinimumHeight( _heightPerRow );
    histogram->setMaximumHeight( _heightPerRow );
    histogram->setMinimumWidth( _currentCentralMinWidth );
    //    histogram->setMinimumWidth( 500 );

    histogram->_timeFrames = &_timeFrames;

    currentRow.histogram = histogram;
    currentRow.label = new QLabel( name.c_str( ));
    currentRow.label->setMinimumWidth( _maxLabelWidth );
    currentRow.label->setMaximumWidth( _maxLabelWidth );
    currentRow.label->setToolTip( name.c_str( ));
    currentRow.checkBox = new QCheckBox( );

    unsigned int row = _histograms.size( );
    _mainLayout->addWidget( currentRow.label, row , 0, 1, 1 );
    _mainLayout->addWidget( histogram, row, 1, 1, _summaryColumns );
    _mainLayout->addWidget( currentRow.checkBox, row, _maxColumns - 1, 1, 1 );

    _rows.push_back( currentRow );

    histogram->mousePosition( &_lastMousePosition );
    histogram->regionPosition( &_regionPercentage );

    connect( histogram, SIGNAL( mousePositionChanged( QPoint )),
             this, SLOT( updateMouseMarker( QPoint )));

    connect( histogram, SIGNAL( mousePressed( QPoint, float )),
             this, SLOT( childHistogramPressed( QPoint, float )));

    connect( histogram, SIGNAL( mouseReleased( QPoint, float )),
             this, SLOT( childHistogramReleased( QPoint, float )));

    connect( histogram, SIGNAL( mouseModifierPressed( float, Qt::KeyboardModifiers )),
             this, SLOT( childHistogramClicked( float, Qt::KeyboardModifiers )));

    _histograms.push_back( histogram );

    histogram->init( _bins, _zoomFactor );
//    CreateSummarySpikes( );
//    UpdateGradientColors( );

    GIDUSet tmp;
    histogram->filteredGIDs( tmp );

    update( );
  }

  int pixelMargin = 10;

  void Summary::childHistogramPressed( const QPoint& position, float /*percentage*/ )
  {

    if( _stackType == T_STACK_FIXED )
    {

      QPoint cursorLocalPoint = _mainHistogram->mapFromGlobal( position );
      float percentage = float( cursorLocalPoint.x( )) / _mainHistogram->width( );

      emit histogramClicked( percentage );
      return;
    }
    if( _focusedHistogram != sender( ))
    {
      _focusedHistogram =
              dynamic_cast< visimpl::MultiLevelHistogram* >( sender( ));

      _focusedHistogram->regionWidth( _regionWidth );
    }

    QPoint cursorLocalPoint = _focusedHistogram->mapFromGlobal( position );
    float percentage = float( cursorLocalPoint.x( )) / _focusedHistogram->width( );

    _currentValueLabel->setText(
        QString::number( _focusedHistogram->focusValueAt( percentage )));
    _localMaxLabel->setText( QString::number( _focusedHistogram->focusMaxLocal( )));
    _globalMaxLabel->setText( QString::number( _focusedHistogram->focusMaxGlobal( )));

    if( _overRegionEdgeLower )
    {
      std::cout << "Selected lower edge" << std::endl;
      _selectedEdgeLower = true;
      _selectedEdgeUpper = false;
    }
    else if( _overRegionEdgeUpper )
    {
      std::cout << "Selected upper edge" << std::endl;
      _selectedEdgeUpper = true;
      _selectedEdgeLower = false;
    }
    else
    {

      _regionGlobalPosition = position;
      _regionWidthPixels = _regionWidth * _focusedHistogram->width( );


       SetFocusRegionPosition( cursorLocalPoint );

  //    _regionGlobalPosition.setX( std::max( _regionWidthPixels, cursorLocalPoint.x(  )));
  //    _regionGlobalPosition.setX( std::min( cursorLocalPoint.x(  ),
  //                          _focusedHistogram->width( ) - _regionWidthPixels));
  //
  //    //TODO
  //    _regionPercentage = percentage;
  //    _regionEdgeLower = std::max( percentage - _regionWidth, _regionWidth );
  //    _regionEdgePointLower = _regionGlobalPosition.x( ) - _regionWidthPixels;
  //    _regionEdgeUpper = std::min( percentage + _regionWidth, _regionWidth );
  //    _regionEdgePointUpper = _regionGlobalPosition.x( ) + _regionWidthPixels;
  //    std::cout << "Focus region bounds " << _regionEdgeLower
  //              << " " << percentage
  //              << " " << _regionEdgeUpper
  //              << std::endl;

      _selectedEdgeLower = _selectedEdgeUpper = false;
    }

    _mousePressed = true;

    if( _stackType == T_STACK_EXPANDABLE )
    {
      for( auto histogram : _histograms )
      {
          histogram->paintRegion( histogram == _focusedHistogram );
        histogram->update( );
      }
    }

  //  std::cout << "Focused mouse pressed" << std::endl;
  //  else if
  }

  void Summary::childHistogramReleased( const QPoint& /*position*/, float /*percentage*/ )
  {
    _mousePressed = false;
  //  std::cout << "Mouse released" << std::endl;
  }

  void Summary::childHistogramClicked( float percentage,
                                       Qt::KeyboardModifiers modifiers )
  {
    if( _stackType == T_STACK_FIXED )
    {
      emit histogramClicked( percentage );
    }
    else if( _stackType == T_STACK_EXPANDABLE )
    {
      if( modifiers == Qt::ControlModifier )
        emit histogramClicked(
            dynamic_cast< visimpl::MultiLevelHistogram* >( sender( )));
      else if( modifiers == Qt::ShiftModifier )
        emit histogramClicked( percentage );
    }
  }

  void Summary::mouseMoveEvent( QMouseEvent* event_ )
  {
    QWidget::mouseMoveEvent( event_ );

    QApplication::setOverrideCursor( Qt::ArrowCursor );
  //  if( _mousePressed )
  //  {
  //    QPoint position = event_->pos( );
  //
  //    _lastMousePosition = mapToGlobal( position );
  //    _showMarker = true;
  //
  //    for( auto histogram : _histograms )
  //      histogram->update( );
  //
  //  }
  }

  float regionEditMargin = 0.005f;
  int regionEditMarginPixels = 2;

  void Summary::SetFocusRegionPosition( const QPoint& cursorLocalPosition )
  {
    if( _stackType == T_STACK_FIXED )
      return;

    _regionLocalPosition = cursorLocalPosition;

    // Limit the region position with borders
    _regionLocalPosition.setX( std::max( _regionWidthPixels, _regionLocalPosition.x(  )));
    _regionLocalPosition.setX( std::min( _regionLocalPosition.x(  ),
                          _focusedHistogram->width( ) - _regionWidthPixels));

    _regionPercentage = float( _regionLocalPosition.x( )) / _focusedHistogram->width( );

  //    _regionPosition = position;
    _regionEdgeLower = std::max( _regionPercentage - _regionWidth, _regionWidth );
    _regionEdgePointLower = _regionLocalPosition.x( ) - _regionWidthPixels;
    _regionEdgeUpper = std::min( _regionPercentage + _regionWidth, 1.0f - _regionWidth);
    _regionEdgePointUpper = _regionLocalPosition.x( ) + _regionWidthPixels;

    _focusWidget->viewRegion( *_focusedHistogram, _regionPercentage, _regionWidth );
    _focusWidget->update( );
  }

  void Summary::updateRegionBounds( void )
  {
    _regionLocalPosition =
        QPoint( _focusedHistogram->width( ) * _regionPercentage,
                _focusedHistogram->height( ) * 0.5f );

    _regionWidthPixels = _regionWidth * _focusedHistogram->width( );

    _regionLocalPosition.setX( std::max( _regionWidthPixels, _regionLocalPosition.x(  )));
    _regionLocalPosition.setX( std::min( _regionLocalPosition.x(  ),
                          _focusedHistogram->width( ) - _regionWidthPixels));

  //    _regionPosition = position;
    _regionEdgeLower = std::max( _regionPercentage - _regionWidth, _regionWidth );
    _regionEdgePointLower = _regionLocalPosition.x( ) - _regionWidthPixels;
    _regionEdgeUpper = std::min( _regionPercentage + _regionWidth, 1.0f - _regionWidth);
    _regionEdgePointUpper = _regionLocalPosition.x( ) + _regionWidthPixels;

    _focusWidget->update( );
  }

  void Summary::updateMouseMarker( QPoint point )
  {
    visimpl::MultiLevelHistogram* focusedHistogram =
            dynamic_cast< visimpl::MultiLevelHistogram* >( sender( ));

    _lastMousePosition = point;

    if( _stackType != T_STACK_EXPANDABLE )
    {
      _mainHistogram->update( );
      return;
    }

    float invWidth = 1.0f / float( focusedHistogram->width( ));

    QPoint cursorLocalPoint = focusedHistogram->mapFromGlobal( point );
  //  float percentage = float( cursorLocalPoint.x( )) * invWidth ;

    if( focusedHistogram == _focusedHistogram )
    {
      if( _mousePressed )
      {
        if( _selectedEdgeLower )
        {

          float diffPerc = ( cursorLocalPoint.x( ) - _regionEdgePointLower ) * invWidth;

          _regionWidth -= diffPerc;

          _regionWidth = std::max( 0.01f, std::min( 0.5f, _regionWidth ));

          _regionWidthPixels = _regionWidth * focusedHistogram->width( );

          _regionEdgeLower = std::max( _regionPercentage - _regionWidth, _regionWidth );
          _regionEdgePointLower = _regionLocalPosition.x( ) - _regionWidthPixels;
          _regionEdgeUpper = std::min( _regionPercentage + _regionWidth, 1.0f - _regionWidth);
          _regionEdgePointUpper = _regionLocalPosition.x( ) + _regionWidthPixels;

           _focusWidget->viewRegion( *_focusedHistogram, _regionPercentage, _regionWidth );
          _focusWidget->update( );

          QApplication::setOverrideCursor( Qt::SizeHorCursor );
        }
        else if ( _selectedEdgeUpper )
        {

          float diffPerc = ( cursorLocalPoint.x( ) - _regionEdgePointUpper ) * invWidth;

          _regionWidth += diffPerc;
          _regionWidth = std::max( 0.01f, std::min( 0.5f, _regionWidth ));

          _regionWidthPixels = _regionWidth * focusedHistogram->width( );

          _regionEdgeLower = std::max( _regionPercentage - _regionWidth, _regionWidth );
          _regionEdgePointLower = _regionLocalPosition.x( ) - _regionWidthPixels;
          _regionEdgeUpper = std::min( _regionPercentage + _regionWidth, 1.0f - _regionWidth);
          _regionEdgePointUpper = _regionLocalPosition.x( ) + _regionWidthPixels;

          _focusWidget->viewRegion( *_focusedHistogram, _regionPercentage, _regionWidth );
          _focusWidget->update( );

          QApplication::setOverrideCursor( Qt::SizeHorCursor );
        }
        else
        {
          _regionGlobalPosition = point;

          SetFocusRegionPosition( cursorLocalPoint );

          QApplication::setOverrideCursor( Qt::ArrowCursor );

        }

        _focusedHistogram->regionWidth( _regionWidth );

      } // end mousePressed
      else
      {
        if( abs(cursorLocalPoint.x( ) - _regionEdgePointLower) < regionEditMarginPixels )
        {
          _overRegionEdgeLower = true;

          QApplication::setOverrideCursor( Qt::SizeHorCursor );
        }
        else if( abs(_regionEdgePointUpper - cursorLocalPoint.x( )) < regionEditMarginPixels )
        {
          _overRegionEdgeUpper = true;

          QApplication::setOverrideCursor( Qt::SizeHorCursor );
        }
        else
        {
          QApplication::setOverrideCursor( Qt::ArrowCursor );
          _overRegionEdgeLower = _overRegionEdgeUpper = false;
        }
      }
    }
    else
    {
      QApplication::setOverrideCursor( Qt::ArrowCursor );
      _overRegionEdgeLower = _overRegionEdgeUpper = false;
    }

    for( auto histogram : _histograms )
    {
      if( _stackType == T_STACK_EXPANDABLE && _mousePressed )
        histogram->paintRegion( histogram == focusedHistogram );
      histogram->update( );
    }
  }

  void Summary::CreateSummarySpikes( )
  {
    std::cout << "Creating histograms... Number of bins: " << _bins << std::endl;
    for( auto histogram : _histograms )
    {
      if( !histogram->isInitialized( ) )
        histogram->init( _bins, _zoomFactor );
    }

  //  _mainHistogram->CreateHistogram( _bins );
  //
  //  _selectionHistogram->CreateHistogram( _bins );


  }

  //void Summary::CreateSummaryVoltages( void )
  //{
  //
  //}

  void Summary::UpdateGradientColors( bool replace )
  {
    for( auto histogram : _histograms )
    {
      if( !histogram->isInitialized( ) || replace)
        histogram->CalculateColors( );
    }

  //  _mainHistogram->CalculateColors( );
  //
  //  _detailHistogram->CalculateColors( );

  }

  unsigned int Summary::histogramsNumber( void )
  {
    return _histograms.size( );
  }


  void Summary::bins( int bins_ )
  {
    _bins = bins_;

    for( auto histogram : _histograms )
    {
      histogram->bins( _bins );
      update( );
    }
  }

  void Summary::zoomFactor( double zoom )
  {
    _zoomFactor = zoom;

    for( auto histogram : _histograms )
    {
      histogram->zoomFactor( _zoomFactor );
      update( );
    }
  }

  unsigned int Summary::bins( void )
  {
    return _bins;
  }

  float Summary::zoomFactor( void )
  {
    return _zoomFactor;
  }

  void Summary::heightPerRow( unsigned int height_ )
  {
    _heightPerRow = height_;

    for( auto histogram : _histograms )
    {
      histogram->setMinimumHeight( _heightPerRow );
      histogram->setMaximumHeight( _heightPerRow );
      histogram->update( );
    }
  }

  unsigned int Summary::heightPerRow( void )
  {
    return _heightPerRow;
  }

  void Summary::showMarker( bool show_ )
  {
    _showMarker = show_;
  }

  void Summary::removeSelections( void )
  {
    std::vector< unsigned int > toDelete;

    unsigned int counter = _rows.size( ) - 1;
    for( unsigned int i = 0; i < _rows.size( ); ++i )
    {
      auto& summaryRow = _rows[ counter ];

      if( summaryRow.checkBox->isChecked( ))
      {
        _mainLayout->removeWidget( summaryRow.label );
        _mainLayout->removeWidget( summaryRow.histogram );
        _mainLayout->removeWidget( summaryRow.checkBox );

        delete summaryRow.label;
        delete summaryRow.histogram;
        delete summaryRow.checkBox;

        _histograms.erase( _histograms.begin( ) + counter );

        toDelete.push_back( counter );
      }

      --counter;
    }

    for( unsigned int i = 0; i < toDelete.size( ); ++i )
      _rows.erase( _rows.begin( ) + toDelete[ i ]);

    counter = 0;
    for( auto histogram : _histograms )
    {
      histogram->firstHistogram( counter == 0 );
      ++counter;

      histogram->update( );
    }

    toDelete.clear( );

    counter = _subsetRows.size( ) - 1;
    for( unsigned int i = 0; i < _subsetRows.size( ); ++i )
    {
      auto& timeFrameRow = _subsetRows[ counter ];

      if( timeFrameRow.checkBox->isChecked( ))
      {
        _subsetLayout->removeWidget( timeFrameRow.label );
        _subsetLayout->removeWidget( timeFrameRow.widget );
        _subsetLayout->removeWidget( timeFrameRow.checkBox );

        delete timeFrameRow.label;
        delete timeFrameRow.widget;
        delete timeFrameRow.checkBox;

        _timeFrames.erase( _timeFrames.begin( ) + counter );
        _subsetEventWidgets.erase( _subsetEventWidgets.begin( ) + counter );

        toDelete.push_back( counter );
      }

      --counter;
    }

    for( unsigned int i = 0; i < toDelete.size( ); ++i )
      _subsetRows.erase(
          _subsetRows.begin( ) + toDelete[ i ]);

    counter = 0;
    for( auto timeFrame : _subsetEventWidgets )
    {
      timeFrame->index( counter );
      ++counter;

      timeFrame->update( );
    }

    if( _subsetRows.size( ) == 0 )
      _subsetScroll->setVisible( false );

    update( );
  }

  void Summary::colorScaleLocal( int value )
  {
    if( value >= 0 )
    {
      colorScaleLocal( ( visimpl::TColorScale ) value );
    }
  }

  void Summary::colorScaleGlobal( int value )
  {
    if( value >= 0 )
    {
      colorScaleGlobal( ( visimpl::TColorScale ) value );
    }
  }

  void Summary::colorScaleLocal( visimpl::TColorScale colorScale )
  {
    _colorScaleLocal = colorScale;

    for( auto histogram : _histograms )
    {
      histogram->colorScaleLocal( colorScale );
      histogram->CalculateColors( visimpl::MultiLevelHistogram::T_HIST_MAIN );
      histogram->CalculateColors( visimpl::MultiLevelHistogram::T_HIST_FOCUS );
      histogram->update( );
    }
  }

  visimpl::TColorScale Summary::colorScaleLocal( void )
  {
    return _colorScaleLocal;
  }

  void Summary::colorScaleGlobal( visimpl::TColorScale colorScale )
  {
    _colorScaleGlobal = colorScale;

    for( auto histogram : _histograms )
    {
      histogram->colorScaleGlobal( colorScale );
      histogram->CalculateColors( visimpl::MultiLevelHistogram::T_HIST_MAIN );
      histogram->CalculateColors( visimpl::MultiLevelHistogram::T_HIST_FOCUS );
      histogram->update( );
    }
  }

  visimpl::TColorScale Summary::colorScaleGlobal( void )
  {
    return _colorScaleGlobal;
  }

  void Summary::regionWidth( float region )
  {
    _regionWidth = region;
  }

  float Summary::regionWidth( void )
  {
    return _regionWidth;
  }

  const GIDUSet& Summary::gids( void )
  {
    return _gids;
  }

  void Summary::gridLinesNumber( int linesNumber )
  {
    _gridLinesNumber = linesNumber;

    for( auto histogram : _histograms )
    {
      histogram->gridLinesNumber( linesNumber );
      histogram->update( );
    }
  }

  unsigned int Summary::gridLinesNumber( void )
  {
    return _gridLinesNumber;
  }

  void Summary::wheelEvent( QWheelEvent* event_ )
  {
    if( event_->modifiers( ).testFlag( Qt::ControlModifier ))
    {

      _currentCentralMinWidth = _mainHistogram->width( );
      _currentCentralMinWidth += event_->delta( );

      if( _currentCentralMinWidth < 200 )
        _currentCentralMinWidth = 200;

      for( auto histogram : _histograms )
        histogram->setMinimumWidth( _currentCentralMinWidth );

      for( auto subsetEventWidget : _subsetEventWidgets )
        subsetEventWidget->setMinimumWidth( _currentCentralMinWidth );

      updateRegionBounds( );

      event_->accept( );
      return;
    }

    event_->ignore( );
  }

  void Summary::moveScrollSync( int newPos )
  {
    if( _syncScrollsVertically )
    {
        _subsetScroll->horizontalScrollBar( )->setValue( newPos );
        _histogramScroll->horizontalScrollBar( )->setValue( newPos );
    }
  }

}
