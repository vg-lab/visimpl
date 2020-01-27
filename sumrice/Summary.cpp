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
#include <QScrollBar>
#include <QApplication>
#include <QGroupBox>

unsigned int visimpl::Selection::_counter = 0;

unsigned int DEFAULT_BINS = 2500;
float DEFAULT_ZOOM_FACTOR = 1.5f;

float DEFAULT_SCALE = 1.0f;
float DEFAULT_SCALE_STEP = 0.3f;

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
  , _flagUpdateChartSize( true )
  , _sizeChartVerticalDefault( 50 )
  , _sizeChartHorizontal( 500 )
  , _sizeChartVertical( _sizeChartVerticalDefault )
  , _sizeView( 200 )
  , _sizeMargin( 0 )
  , _scaleCurrentHorizontal( DEFAULT_SCALE )
  , _scaleCurrentVertical( DEFAULT_SCALE )
  , _scaleStep( DEFAULT_SCALE_STEP )
  , _gridLinesNumber( 3 )
  , _simData( nullptr )
  , _spikeReport( nullptr )
  , _player( nullptr )
  , _mainHistogram( nullptr )
  , _detailHistogram( nullptr )
  , _focusedHistogram( nullptr )
  , _mousePressed( false )
  , _stackType( stackType )
  , _colorScaleLocal( visimpl::T_COLOR_LINEAR )
  , _colorScaleGlobal( visimpl::T_COLOR_LOGARITHMIC )
  , _colorLocal( 0, 0, 128, 50 )
  , _colorGlobal( 255, 0, 0, 100 )
  , _focusWidget( nullptr )
  , _spinBoxScaleHorizontal( nullptr )
  , _spinBoxScaleVertical( nullptr )
  , _layoutHistoLabels( nullptr )
  , _scrollHistoLabels( nullptr )
  , _layoutEventLabels( nullptr )
  , _eventLabelsScroll( nullptr )
  , _layoutHistograms( nullptr )
  , _scrollHistogram( nullptr )
  , _layoutEvents( nullptr )
  , _scrollEvent( nullptr )
  , _layoutMain( nullptr )
//  , _splitVertCentralFoot( nullptr )
  , _splitVertEventsHisto( nullptr )
  , _splitHorizEvents( nullptr )
  , _splitHorizHisto( nullptr )
  , _maxNumEvents( 8 )
  , _syncScrollsHorizontally( true )
  , _syncScrollsVertically( true )
  , _heightPerRow( 50 )
  , _maxLabelWidth( 200 )
  , _footHeightMax( 320)
  , _footHeightMin( _footHeightMax )
  , _footWidthMax( 250 )
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
  , _fillPlots( true )
  , _defaultCorrelationDeltaTime( 0.125f )
  {

    setMouseTracking( true );


//      QWidget* outerEventsContainer = new QWidget( );
//      QGridLayout* outerEventsLayout = new QGridLayout( );


//      outerEventsLayout->addWidget( _eventLabelsScroll, 0, 0, 1, 1 );
//      outerEventsLayout->addWidget( _eventScroll, 0, 1, 1, 20 );
//      outerEventsContainer->setLayout( outerEventsLayout );

//      QWidget* outerHistoContainer = new QWidget( );
//      QGridLayout* outerHistoLayout = new QGridLayout( );
//
//      outerHistoLayout->addWidget( _histoLabelsScroll, 0, 0, 1, 1 );
//      outerHistoLayout->addWidget( _histogramScroll, 0, 1, 1, 20 );
//      outerHistoContainer->setLayout( outerHistoLayout );

//      QWidget* headerContainer = new QWidget( );
//      QGridLayout* headerLayout = new QGridLayout( );
//      headerLayout->addWidget( new QLabel( "Name" ), 0, 0, 1, 1);
//      headerLayout->addWidget( new QLabel( "Activity" ), 0, 1, 1, _summaryColumns);
//      headerLayout->addWidget( new QLabel( "Select" ), 0, _maxColumns - 1, 1, 1);
//
//      headerContainer->setLayout( headerLayout );

      _initCentralGUI( );
      auto footWidget = _initFootGUI( );

//      _splitVertCentralFoot = new QSplitter( Qt::Vertical, this );
//      _splitVertCentralFoot->addWidget( )

      if( _stackType == T_STACK_EXPANDABLE )
      {
        _splitVertEventsHisto = new QSplitter( Qt::Vertical, this );
        _splitVertEventsHisto->addWidget( _splitHorizEvents );
        _splitVertEventsHisto->addWidget( _splitHorizHisto );
        _splitVertEventsHisto->addWidget( footWidget );

        _splitVertEventsHisto->setSizes( { 1000, 1000, 1000 } );

        _layoutMain->addWidget(_splitVertEventsHisto );
      }
//      _layoutMain->addWidget( foot );

      this->setLayout( _layoutMain );

    #ifdef VISIMPL_USE_ZEROEQ

      _insertionTimer.setSingleShot( false );
      _insertionTimer.setInterval( 250 );
      connect( &_insertionTimer, SIGNAL( timeout( )),
               this, SLOT(deferredInsertion( )));
      _insertionTimer.start( );


  #endif


    // Fill the palette with ColorbBewer qualitative palette with 10 classes
    // but rearranging to have consecutive colors with different hue.
    _eventsPalette = scoop::ColorPalette::colorBrewerQualitative(
        scoop::ColorPalette::ColorBrewerQualitative::Set1, 9 );

//    _subsetEventColorPalette.reserve( 10 );
////    _subsetEventColorPalette.push_back( QColor( "#a6cee3" ));
//    _subsetEventColorPalette.push_back( QColor( "#b2df8a" ));
////    _subsetEventColorPalette.push_back( QColor( "#fb9a99" ));
//    _subsetEventColorPalette.push_back( QColor( "#fdbf6f" ));
//    _subsetEventColorPalette.push_back( QColor( "#cab2d6" ));
//    _subsetEventColorPalette.push_back( QColor( "#b15928" ));
//    _subsetEventColorPalette.push_back( QColor( "#ffff99" ));
////    _subsetEventColorPalette.push_back( QColor( "#1f78b4" ));
//    _subsetEventColorPalette.push_back( QColor( "#33a02c" ));
////    _subsetEventColorPalette.push_back( QColor( "#e31a1c" ));
//    _subsetEventColorPalette.push_back( QColor( "#ff7f00" ));
//    _subsetEventColorPalette.push_back( QColor( "#6a3d9a" ));

  }

  void Summary::Init( simil::SimulationData* data_ )
  {

    _simData = data_;

    switch( data_->simulationType( ))
    {
      case simil::TSimSpikes:
      {
        simil::SpikeData* spikeData = dynamic_cast< simil::SpikeData* >( _simData );
        _spikeReport = spikeData;

        break;
      }
      case simil::TSimVoltages:
        break;
      default:
        break;
    }

    _gids = GIDUSet( data_->gids( ).begin( ), data_->gids( ).end( ));

    Init( );
  }

  void Summary::_initCentralGUI( void )
  {

    if( _stackType == T_STACK_FIXED )
    {
      _layoutHistograms = new QGridLayout( );
      this->setLayout( _layoutHistograms );
    }
    else if( _stackType == T_STACK_EXPANDABLE )
    {
      _maxColumns= 20;
      _regionWidth = 0.1f;
      _summaryColumns = _maxColumns - 2;

      _layoutMain = new QVBoxLayout( );
      _layoutMain->setAlignment( Qt::AlignTop );

      _layoutEventLabels = new QGridLayout( );
      _layoutEventLabels->setAlignment( Qt::AlignTop );
      _layoutEventLabels->setVerticalSpacing( 0 );

      QWidget* eventLabelsContainer = new QWidget( );
      eventLabelsContainer->setLayout( _layoutEventLabels );
      eventLabelsContainer->setMaximumWidth( 150 );

      _eventLabelsScroll = new QScrollArea( );
      _eventLabelsScroll->setWidgetResizable( true );
      _eventLabelsScroll->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
      _eventLabelsScroll->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
      _eventLabelsScroll->setVisible( false );
      _eventLabelsScroll->setWidget( eventLabelsContainer );

      _layoutEvents = new QGridLayout( );
      _layoutEvents->setAlignment( Qt::AlignTop );
      _layoutEvents->setVerticalSpacing( 0 );

      QWidget* eventsContainer = new QWidget( );
      eventsContainer->setLayout( _layoutEvents );

      _scrollEvent = new QScrollArea();
      _scrollEvent->setWidgetResizable( true );
      _scrollEvent->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
      _scrollEvent->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
      _scrollEvent->setVisible( false );
      _scrollEvent->setWidget( eventsContainer );

      connect( _scrollEvent->horizontalScrollBar( ), SIGNAL( actionTriggered( int )),
               this, SLOT( moveHoriScrollSync( int )));

      connect( _scrollEvent->verticalScrollBar( ), SIGNAL( actionTriggered( int )),
               this, SLOT( moveVertScrollSync( int )));

      connect( _eventLabelsScroll->verticalScrollBar( ), SIGNAL( actionTriggered( int )),
               this, SLOT( moveVertScrollSync( int )));

      _layoutHistoLabels = new QGridLayout( );
      _layoutHistoLabels->setAlignment( Qt::AlignTop );
      _layoutHistoLabels->setVerticalSpacing( 0 );

      QWidget* histoLabelsContainer = new QWidget( );
      histoLabelsContainer->setLayout( _layoutHistoLabels );
      histoLabelsContainer->setMaximumWidth( 150 );

      _scrollHistoLabels = new QScrollArea( );
      _scrollHistoLabels->setWidgetResizable( true );
      _scrollHistoLabels->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
      _scrollHistoLabels->setWidget( histoLabelsContainer );


      _layoutHistograms = new QGridLayout( );
      _layoutHistograms->setAlignment( Qt::AlignTop );
      _layoutHistograms->setVerticalSpacing( 0 );

      QWidget* histogramsContainer = new QWidget( );
      histogramsContainer->setLayout( _layoutHistograms );
//      histogramsContainer->setMinimumWidth( 300);

      _scrollHistogram = new QScrollArea( );
      _scrollHistogram->setWidgetResizable( true );
      _scrollHistogram->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
      _scrollHistogram->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
      _scrollHistogram->setWidget( histogramsContainer );

      connect( _scrollHistogram->horizontalScrollBar( ), SIGNAL( actionTriggered( int )),
               this, SLOT( moveHoriScrollSync( int )));

      connect( _scrollHistogram->verticalScrollBar( ), SIGNAL( actionTriggered( int )),
               this, SLOT( moveVertScrollSync( int )));

      connect( _scrollHistoLabels->verticalScrollBar( ), SIGNAL( actionTriggered( int )),
               this, SLOT( moveVertScrollSync( int )));

//      auto widgetSize = size( );
//      QList< int > initialSizes;
//      initialSizes.push_back( widgetSize.width( ) * 0.2f );
//      initialSizes.push_back( widgetSize.width( ) - initialSizes[ 0 ]);

      _splitHorizEvents = new QSplitter( Qt::Horizontal );
      _splitHorizHisto = new QSplitter( Qt::Horizontal );

      _splitHorizEvents->addWidget( _eventLabelsScroll );
      _splitHorizEvents->addWidget( _scrollEvent );

      _splitHorizHisto->addWidget( _scrollHistoLabels );
      _splitHorizHisto->addWidget( _scrollHistogram );

//      _eventsSplitter->setSizes( initialSizes );
//      _histoSplitter->setSizes( initialSizes );

      connect( _splitHorizEvents, SIGNAL( splitterMoved( int, int )),
               this, SLOT( syncSplitters( ) ));

      connect( _splitHorizHisto, SIGNAL( splitterMoved( int, int )),
               this, SLOT( syncSplitters( ) ));
    }
  }

  QWidget* Summary::_initFootGUI( void )
  {
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

    _focusWidget = new FocusFrame( );
    _focusWidget->colorLocal( _colorLocal );
    _focusWidget->colorGlobal( _colorGlobal );
//      _focusWidget->setMinimumWidth( 500 );
//      _focusWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  //  _detailHistogram = new visimpl::Histogram( );

    _localColorWidget = new QWidget( );
    _localColorWidget->setPalette( QPalette( _colorLocal ));
    _localColorWidget->setAutoFillBackground( true );
    _localColorWidget->setMaximumWidth( 30 );
    _localColorWidget->setMinimumWidth( 30 );
    _localColorWidget->setMinimumHeight( 30 );
    _localColorWidget->setMaximumHeight( 30 );

    _globalColorWidget = new QWidget( );
    _globalColorWidget->setPalette( QPalette( _colorGlobal ));
    _globalColorWidget->setAutoFillBackground( true );
    _globalColorWidget->setMaximumWidth( 30 );
    _globalColorWidget->setMinimumWidth( 30 );
    _globalColorWidget->setMinimumHeight( 30 );
    _globalColorWidget->setMaximumHeight( 30 );

    _currentValueLabel = new QLabel( "" );
    _currentValueLabel->setMaximumWidth( 50 );
    _globalMaxLabel = new QLabel( "" );
    _globalMaxLabel->setMaximumWidth( 50 );
    _localMaxLabel = new QLabel( "" );
    _localMaxLabel->setMaximumWidth( 50 );

    _spinBoxBins = new QSpinBox( );
    _spinBoxBins->setMinimum( 50 );
    _spinBoxBins->setMaximum( 100000 );
    _spinBoxBins->setSingleStep( 50 );
    _spinBoxBins->setValue( _bins );

    _spinBoxScaleHorizontal = new QDoubleSpinBox( );
    _spinBoxScaleHorizontal->setMinimum( 1.0 );
    _spinBoxScaleHorizontal->setMaximum( 500 );
    _spinBoxScaleHorizontal->setSingleStep( 0.1 );
    _spinBoxScaleHorizontal->setValue( DEFAULT_SCALE );

    _spinBoxScaleVertical = new QDoubleSpinBox( );
    _spinBoxScaleVertical->setMinimum( 1.0 );
    _spinBoxScaleVertical->setMaximum( 5 );
    _spinBoxScaleVertical->setSingleStep( 0.1 );
    _spinBoxScaleVertical->setValue( DEFAULT_SCALE );

    _spinBoxZoomFactor = new QDoubleSpinBox( );
    _spinBoxZoomFactor->setMinimum( 1.0 );
    _spinBoxZoomFactor->setMaximum( 1000.0 );
    _spinBoxZoomFactor->setSingleStep( 0.5 );
    _spinBoxZoomFactor->setValue( _zoomFactor );

    QSpinBox* gridSpinBox = new QSpinBox( );
    gridSpinBox->setMinimum( 0 );
    gridSpinBox->setMaximum( 10000 );
    gridSpinBox->setSingleStep( 1 );
    gridSpinBox->setValue( 3 );

    QPushButton* focusButton = new QPushButton( "Focus" );

    QScrollArea* scrollFootLeft = new QScrollArea( );
    scrollFootLeft->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    scrollFootLeft->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    scrollFootLeft->setWidgetResizable( true );
    scrollFootLeft->setMaximumWidth( 300 );

    QWidget* containerFootLeft = new QWidget( );
    QVBoxLayout* layoutFootLeft = new QVBoxLayout( );
    layoutFootLeft->setAlignment( Qt::AlignTop );
    containerFootLeft->setLayout( layoutFootLeft );
    containerFootLeft->setMinimumWidth( _footWidthMax );
    containerFootLeft->setMaximumWidth( _footWidthMax );
    containerFootLeft->setMinimumHeight( _footHeightMin );
    containerFootLeft->setMaximumHeight( _footHeightMax );

    tmpFootLeft = containerFootLeft;

    QGroupBox* groupBoxNorm = new QGroupBox( "Normalization: ");
    QGridLayout* layoutNorm = new QGridLayout( );
    layoutNorm->setAlignment( Qt::AlignTop );
    groupBoxNorm->setLayout( layoutNorm );

    layoutNorm->addWidget( new QLabel( "Local:" ), 0, 0, 1, 1);
    layoutNorm->addWidget( _localColorWidget, 0, 1, 1, 1 );
    layoutNorm->addWidget( localComboBox, 0, 2, 1, 1 );

    layoutNorm->addWidget( new QLabel( "Global:" ), 1, 0, 1, 1);
    layoutNorm->addWidget( _globalColorWidget, 1, 1, 1, 1 );
    layoutNorm->addWidget( globalComboBox, 1, 2, 1, 1 );

    QGroupBox* groupBoxRuleConfig = new QGroupBox( "Rule configuration: ");
    QGridLayout* layoutRuleConfig = new QGridLayout( );
    layoutRuleConfig->setAlignment( Qt::AlignTop );
    groupBoxRuleConfig->setLayout( layoutRuleConfig );

    layoutRuleConfig->addWidget( new QLabel( "Rule sectors: " ), 0, 0, 1, 1 );
    layoutRuleConfig->addWidget( gridSpinBox, 0, 1, 1, 1 );
  //  unsigned int totalRows = 10;

    scrollFootLeft->setWidget( containerFootLeft );



//      QGroupBox* groupBoxFocusWidget = new QGroupBox( "Focus" );
//      QGridLayout* layoutFocus = new QGridLayout( );
//      groupBoxFocusWidget->setLayout( layoutFocus );
//      layoutFocus->addWidget( _focusWidget, 0, 0, 1, 1 );

    QScrollArea* scrollFootRight = new QScrollArea( );
    scrollFootRight->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    scrollFootRight->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    scrollFootRight->setWidgetResizable( true );
    scrollFootRight->setMaximumWidth( 300 );

    QWidget* containerFootRight = new QWidget( );
    QVBoxLayout* layoutFootRight = new QVBoxLayout( );
    layoutFootRight->setAlignment( Qt::AlignTop );
    containerFootRight->setLayout( layoutFootRight );
    containerFootRight->setMinimumWidth( _footWidthMax );
    containerFootRight->setMaximumWidth( _footWidthMax );
    containerFootRight->setMinimumHeight( _footHeightMin );
    containerFootRight->setMaximumHeight( _footHeightMax );


    tmpFootRight = containerFootRight;

    QGroupBox* groupBoxScale = new QGroupBox( "Scale adjustment" );
    QGridLayout* layoutScale = new QGridLayout( );
    groupBoxScale->setLayout( layoutScale );
    layoutScale->addWidget( new QLabel( "Horizontal: "), 0, 0, 1, 1 );
    layoutScale->addWidget( _spinBoxScaleHorizontal, 0, 1, 1, 1 );
    layoutScale->addWidget( new QLabel( "Vertical: "), 1, 0, 1, 1 );
    layoutScale->addWidget( _spinBoxScaleVertical, 1, 1, 1, 1 );

    connect( _spinBoxScaleHorizontal, SIGNAL( editingFinished( void )),
             this, SLOT( _updateScaleHorizontal( void )));

    connect( _spinBoxScaleVertical, SIGNAL( editingFinished( void )),
             this, SLOT( _updateScaleVertical( void )));


    QGroupBox* groupBoxBinConfig = new QGroupBox( "Bin configuration:" );
    QGridLayout* layoutBinConfig = new QGridLayout( );
    layoutBinConfig->setAlignment( Qt::AlignTop );
    groupBoxBinConfig->setLayout( layoutBinConfig );

    //TODO
//      layoutBinConfig->addWidget( focusButton, 0, 11, 1, 1);

    layoutBinConfig->addWidget( new QLabel( "Bins number:" ), 0, 0, 1, 1 );
    layoutBinConfig->addWidget( _spinBoxBins, 0, 1, 1, 1 );
    layoutBinConfig->addWidget( new QLabel( "Zoom factor:" ), 1, 0, 1, 1 );
    layoutBinConfig->addWidget( _spinBoxZoomFactor, 1, 1, 1, 1 );

    QGroupBox* groupBoxInformation = new QGroupBox( "Data inspector: ");
    QGridLayout* layoutInformation = new QGridLayout( );
    layoutInformation->setAlignment( Qt::AlignTop );
    groupBoxInformation->setLayout( layoutInformation );

    layoutInformation->addWidget( new QLabel( "Current value: "), 2, 9, 1, 2 );
    layoutInformation->addWidget( _currentValueLabel, 2, 11, 1, 1 );
    layoutInformation->addWidget( new QLabel( "Local max: "), 3, 9, 1, 2 );
    layoutInformation->addWidget( _localMaxLabel, 3, 11, 1, 1 );
    layoutInformation->addWidget( new QLabel( "Global max: "), 4, 9, 1, 2 );
    layoutInformation->addWidget( _globalMaxLabel, 4, 11, 1, 1 );

    scrollFootRight->setWidget( containerFootRight );




    layoutFootLeft->addWidget( groupBoxNorm );
    layoutFootLeft->addWidget( groupBoxScale );


    layoutFootRight->addWidget( groupBoxBinConfig );
    layoutFootRight->addWidget( groupBoxInformation );
    layoutFootRight->addWidget( groupBoxRuleConfig );

    footLayout->addWidget( scrollFootLeft, 0, 0, 1, 1 );
    footLayout->addWidget( _focusWidget, 0, 1, 1, 3 );
    footLayout->addWidget( scrollFootRight, 0, 4, 1, 1 );



    localComboBox->setCurrentIndex( ( int ) _colorScaleLocal );
    globalComboBox->setCurrentIndex( ( int ) _colorScaleGlobal );

    connect( localComboBox, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( colorScaleLocal( int )));

    connect( globalComboBox, SIGNAL( currentIndexChanged( int ) ),
               this, SLOT( colorScaleGlobal( int )));

    connect( _spinBoxBins, SIGNAL( editingFinished( void )),
             this,  SLOT( binsChanged( void )));

    connect( _spinBoxZoomFactor, SIGNAL( editingFinished( void )),
             this,  SLOT( zoomFactorChanged( void )));

    connect( gridSpinBox, SIGNAL( valueChanged( int )),
             this, SLOT( gridLinesNumber( int )));

    connect( focusButton, SIGNAL( clicked( )),
             this, SLOT( focusPlayback( )));

    foot->setLayout( footLayout );

    return foot;
  }

  void Summary::Init( void )
  {
    if( !_spikeReport )
      return;

    _mainHistogram = new visimpl::HistogramWidget( *_spikeReport );
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
    _mainHistogram->setMinimumWidth( _sizeChartHorizontal );
    _mainHistogram->simPlayer( _player );
//    _focusedHistogram = _mainHistogram;

    TColorMapper colorMapper;
    colorMapper.Insert( 0.0f, glm::vec4( 157, 206, 111, 255 ));
    colorMapper.Insert( 0.25f, glm::vec4( 125, 195, 90, 255 ));
    colorMapper.Insert( 0.50f, glm::vec4( 109, 178, 113, 255 ));
    colorMapper.Insert( 0.75f, glm::vec4( 76, 165, 86, 255 ));
    colorMapper.Insert( 1.0f, glm::vec4( 63, 135, 61, 255 ));

    _mainHistogram->colorMapper( colorMapper );

    _mainHistogram->mousePosition( &_lastMousePosition );
    _mainHistogram->regionPosition( &_regionPercentage );
    connect( _mainHistogram, SIGNAL( mousePositionChanged( QPoint )),
             this, SLOT( updateMouseMarker( QPoint )));

    _mainHistogram->init( _bins, _zoomFactor );

    if( _stackType == T_STACK_FIXED)
    {
      _layoutHistograms->addWidget( _mainHistogram, 0, 1, 1, 1 );
      _mainHistogram->paintRegion( false );
      _mainHistogram->simPlayer( _player );
      _histogramWidgets.push_back( _mainHistogram );

      connect( _mainHistogram, SIGNAL( mousePressed( QPoint, float )),
               this, SLOT( childHistogramPressed( QPoint, float )));
    }
    else if( _stackType == T_STACK_EXPANDABLE )
    {

      HistogramRow mainRow;

      mainRow.histogram = _mainHistogram;
      mainRow.histogram->_events = &_events;
      mainRow.histogram->name( "All" );
      QString labelText( "All" );
      mainRow.label = new QLabel( labelText );
      mainRow.label->setMinimumWidth( _maxLabelWidth );
      mainRow.label->setMaximumWidth( _maxLabelWidth );
      mainRow.label->setMinimumHeight( _heightPerRow );
      mainRow.label->setMaximumHeight( _heightPerRow );

//      mainRow.checkBox = new QCheckBox( );


      unsigned int row = _histogramWidgets.size( );
      _layoutHistoLabels->addWidget( mainRow.label, row , 0, 1, 1 );
      _layoutHistograms->addWidget( _mainHistogram, row, 1, 1, _summaryColumns );
//      _histogramsLayout->addWidget( mainRow.checkBox, row, _maxColumns - 1, 1, 1 );
    //  mainRow.checkBox->setVisible( false );

      _histogramRows.push_back( mainRow );
      _histogramWidgets.push_back( _mainHistogram );


      //  AddGIDSelection( gids );

      connect( _mainHistogram, SIGNAL( mousePressed( QPoint, float )),
               this, SLOT( childHistogramPressed( QPoint, float )));

      connect( _mainHistogram, SIGNAL( mouseReleased( QPoint, float )),
               this, SLOT( childHistogramReleased( QPoint, float )));

      connect( _mainHistogram, SIGNAL( mouseModifierPressed( float, Qt::KeyboardModifiers )),
               this, SLOT( childHistogramClicked( float, Qt::KeyboardModifiers )));

      if( _eventLabelsScroll )
        _eventLabelsScroll->setVisible( false );

      if( _scrollEvent )
        _scrollEvent->setVisible( false );

    }
  //  CreateSummarySpikes( );
  //  UpdateGradientColors( );
    update( );
  }

  void Summary::generateEventsRep( void )
  {
    if( _simData->subsetsEvents( )->numEvents( ) > 0 )
    {
      _eventLabelsScroll->setVisible( true );
      _scrollEvent->setVisible( true );

      simil::EventRange timeFrames = _simData->subsetsEvents( )->events( );

      float invTotal = 1.0f / ( _spikeReport->endTime( ) - _spikeReport->startTime( ));

      float startPercentage;
      float endPercentage;

      unsigned int counter = 0;
      for( auto it = timeFrames.first; it != timeFrames.second; ++it, ++counter )
      {
        TEvent timeFrame;
        timeFrame.name = it->first;
        timeFrame.visible = true;

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

        timeFrame.color = _eventsPalette.colors( )[
          counter /* %  _eventsPalette.size( ) */ ];

        _events.push_back( timeFrame );

        QLabel* label = new QLabel( timeFrame.name.c_str( ));
        label->setMinimumHeight( 20 );
        label->setMinimumWidth( _maxLabelWidth );
        label->setMaximumWidth( _maxLabelWidth );
        label->setMinimumHeight( _heightPerRow );
        label->setMaximumHeight( _heightPerRow );
        label->setToolTip( timeFrame.name.c_str( ));

        EventWidget* eventWidget = new EventWidget( );
        eventWidget->name( timeFrame.name );
        eventWidget->setSizePolicy( QSizePolicy::Expanding,
                                    QSizePolicy::Expanding );
        eventWidget->timeFrames( &_events );
        eventWidget->setMinimumWidth( _sizeChartHorizontal );
        eventWidget->setMinimumHeight( _heightPerRow );
        eventWidget->setMaximumHeight( _heightPerRow );
        eventWidget->index( counter );

//            QCheckBox* checkbox = new QCheckBox();

        EventRow eventrow;
        eventrow.widget = eventWidget;
        eventrow.label = label;
//            eventrow.checkBox = checkbox;

        _eventRows.push_back( eventrow );
        _eventWidgets.push_back( eventWidget );

        _layoutEventLabels->addWidget( label, counter, 0, 1, 1 );
        _layoutEvents->addWidget( eventWidget, counter, 1, 1, _summaryColumns );
//            _eventsLayout->addWidget( checkbox, counter, _maxColumns, 1, 1 );

      }
    }
  }

  void Summary::importSubsetsFromSubsetMngr( void )
  {
    simil::SubsetMapRange subsets =
        _spikeReport->subsetsEvents( )->subsets( );

    for( auto it = subsets.first; it != subsets.second; ++it )
    {
      GIDUSet subset( it->second.begin( ), it->second.end( ));
      std::cout << " <<< inserting " << it->first << " " << subset.size( ) << std::endl;
      insertSubset( it->first, subset );
    }

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

      std::cout << "Adding new selection " << selection.name << " with size " << selected.size( ) << std::endl;
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
        insertSubset( selection );
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

  void Summary::UpdateHistograms( void )
  {
      for( auto histogram : _histogramWidgets )
      {
        //histogram->paintRegion( histogram == _focusedHistogram );
        histogram->Spikes(*_spikeReport);
        histogram->Update( );

      }


  }

  void Summary::insertSubset( const Selection& selection )
  {
    insertSubset( selection.name, selection.gids );
  }

  void Summary::insertSubset( const std::string& name, const GIDUSet& subset )
  {
    HistogramRow currentRow;

    visimpl::HistogramWidget* histogram =
        new visimpl::HistogramWidget( *_spikeReport );

    histogram->filteredGIDs( subset );
    histogram->name( name );
    histogram->colorMapper( _mainHistogram->colorMapper( ));
    histogram->colorScaleLocal( _colorScaleLocal );
    histogram->colorScaleGlobal( _colorScaleGlobal );
    histogram->colorLocal( _colorLocal );
    histogram->colorGlobal( _colorGlobal );
    histogram->normalizeRule( visimpl::T_NORM_MAX );
    histogram->representationMode( visimpl::T_REP_CURVE );
    histogram->regionWidth( _regionWidth );
    histogram->gridLinesNumber( _gridLinesNumber );

    histogram->init( _bins, _zoomFactor );

    if( histogram->empty( ))
    {
      std::cout << "Discarding empty histogram " << name << " with elements " << subset.size( ) << std::endl;

      delete histogram;
      return;
    }
    //TODO

    histogram->setMinimumHeight( _heightPerRow );
    histogram->setMaximumHeight( _heightPerRow );
    histogram->setMinimumWidth( _sizeChartHorizontal );
    //    histogram->setMinimumWidth( 500 );

    histogram->simPlayer( _player );

    histogram->_events = &_events;

    currentRow.histogram = histogram;
    currentRow.label = new QLabel( name.c_str( ));
    currentRow.label->setMinimumWidth( _maxLabelWidth );
    currentRow.label->setMaximumWidth( _maxLabelWidth );
    currentRow.label->setMinimumHeight( _heightPerRow );
    currentRow.label->setMaximumHeight( _heightPerRow );
    currentRow.label->setToolTip( name.c_str( ));
//    currentRow.checkBox = new QCheckBox( );

    unsigned int row = _histogramWidgets.size( );
    _layoutHistoLabels->addWidget( currentRow.label, row , 0, 1, 1 );
    _layoutHistograms->addWidget( histogram, row, 1, 1, _summaryColumns );
//    _histogramsLayout->addWidget( currentRow.checkBox, row, _maxColumns - 1, 1, 1 );

    _histogramRows.push_back( currentRow );

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

    _histogramWidgets.push_back( histogram );

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
//      if( !_focusedHistogram )
//        _regionWidth = 10.0f / nativeParentWidget( )->width( );

      _focusedHistogram =
              dynamic_cast< visimpl::HistogramWidget* >( sender( ));

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
      _selectedEdgeLower = false;
      _selectedEdgeUpper = true;
    }
    else
    {

      _regionGlobalPosition = position;
      _regionWidthPixels = _regionWidth * _focusedHistogram->width( );


       SetFocusRegionPosition( cursorLocalPoint );

      _selectedEdgeLower = _selectedEdgeUpper = false;
    }

    _mousePressed = true;

    for( auto histogram : _histogramWidgets )
    {
      histogram->paintRegion( histogram == _focusedHistogram );
      histogram->update( );
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
            dynamic_cast< visimpl::HistogramWidget* >( sender( )));
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

    calculateRegionBounds( );

    _focusWidget->viewRegion( *_focusedHistogram, _regionPercentage, _regionWidth );
    _focusWidget->update( );
  }

  void Summary::updateRegionBounds( void )
  {
    if( _focusedHistogram )
    {
      _regionLocalPosition =
          QPoint( _focusedHistogram->width( ) * _regionPercentage,
                  _focusedHistogram->height( ) * 0.5f );

  //    std::cout << "Focus width: " << _focusedHistogram->width( ) << std::endl;

      _regionWidthPixels = _regionWidth * _focusedHistogram->width( );

      _regionLocalPosition.setX(
          std::max( _regionWidthPixels, _regionLocalPosition.x(  )));

      _regionLocalPosition.setX( std::min( _regionLocalPosition.x(  ),
                            _focusedHistogram->width( ) - _regionWidthPixels));

    //    _regionPosition = position;
      calculateRegionBounds( );

      _focusWidget->update( );
    }
  }

  void Summary::calculateRegionBounds( void )
  {
    _regionEdgeLower = std::max( _regionPercentage - _regionWidth, _regionWidth );
    _regionEdgePointLower = _regionLocalPosition.x( ) - _regionWidthPixels;
    _regionEdgeUpper = std::min( _regionPercentage + _regionWidth, 1.0f - _regionWidth);
    _regionEdgePointUpper = _regionLocalPosition.x( ) + _regionWidthPixels;

  }


  void Summary::updateMouseMarker( QPoint point )
  {
    visimpl::HistogramWidget* focusedHistogram =
            dynamic_cast< visimpl::HistogramWidget* >( sender( ));

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

          float regionMinSize = 5.0f / focusedHistogram->width( );

//          std::cout << "Min size: " << regionMinSize << std::endl;

          _regionWidth = std::max( regionMinSize, std::min( 0.5f, _regionWidth ));

          _regionWidthPixels = _regionWidth * _focusedHistogram->width( );

          calculateRegionBounds( );

          _focusWidget->viewRegion( *_focusedHistogram, _regionPercentage, _regionWidth );
          _focusWidget->update( );

          QApplication::setOverrideCursor( Qt::SizeHorCursor );
        }
        else if ( _selectedEdgeUpper )
        {

          float diffPerc = ( cursorLocalPoint.x( ) - _regionEdgePointUpper ) * invWidth;

          _regionWidth += diffPerc;

          float regionMinSize = 5.0f / focusedHistogram->width( );

//          std::cout << "Min size: " << regionMinSize << std::endl;

          _regionWidth = std::max( regionMinSize,
                                   std::min( 0.5f, _regionWidth ));

          _regionWidthPixels = _regionWidth * _focusedHistogram->width( );

          calculateRegionBounds( );

          _focusWidget->viewRegion( *_focusedHistogram, _regionPercentage,
                                    _regionWidth );
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

    for( auto histogram : _histogramWidgets )
    {
      if( _stackType == T_STACK_EXPANDABLE && _mousePressed )
        histogram->paintRegion( histogram == focusedHistogram );
      histogram->update( );
    }
  }

  void Summary::CreateSummarySpikes( )
  {
    std::cout << "Creating histograms... Number of bins: " << _bins << std::endl;
    for( auto histogram : _histogramWidgets )
    {
      if( !histogram->isInitialized( ) )
        histogram->init( _bins, _zoomFactor );
    }
  }


  void Summary::UpdateGradientColors( bool replace )
  {
    for( auto histogram : _histogramWidgets )
    {
      if( !histogram->isInitialized( ) || replace)
        histogram->CalculateColors( );
    }
  }

  unsigned int Summary::histogramsNumber( void )
  {
    return _histogramWidgets.size( );
  }


  void Summary::binsChanged( void )
  {
    unsigned int binsNumber = _spinBoxBins->value( );
    bins( binsNumber );
  }

  void Summary::bins( int bins_ )
  {
    _bins = bins_;

#ifdef VISIMPL_USE_OPENMP
    #pragma omp parallel for
    for( int i = 0; i < ( int )_histogramWidgets.size( ); ++i )
    {
      auto histogram = _histogramWidgets[ i ];
#else
    for( auto histogram : _histogramWidgets )
    {
#endif
      histogram->bins( _bins );
      histogram->update( );
    }
  }

  void Summary::zoomFactorChanged( void )
  {
    double value = _spinBoxZoomFactor->value( );

    zoomFactor( value );
  }

  void Summary::zoomFactor( double zoom )
  {
    _zoomFactor = zoom;

#ifdef VISIMPL_USE_OPENMP
    #pragma omp parallel for
    for( int i = 0; i < ( int )_histogramWidgets.size( ); ++i )
    {
      auto histogram = _histogramWidgets[ i ];
#else
    for( auto histogram : _histogramWidgets )
    {
#endif

      histogram->zoomFactor( _zoomFactor );
      histogram->update( );
    }
  }

  void Summary::fillPlots( bool fillPlots_ )
  {
    _fillPlots = fillPlots_;

    for( auto histogram : _histogramWidgets )
    {
      histogram->fillPlots( _fillPlots );
      histogram->update( );
    }

    _focusWidget->fillPlots( _fillPlots );
    _focusWidget->update( );
  }

  void Summary::toggleAutoNameSelections( void )
  {
    _autoNameSelection = !_autoNameSelection;
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

    for( auto histogram : _histogramWidgets )
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

  const std::vector< EventWidget* >* Summary::eventWidgets( void ) const
  {
    return &_eventWidgets;
  }

  const std::vector< HistogramWidget* >* Summary::histogramWidgets( void ) const
  {
    return &_histogramWidgets;
  }

  void Summary::hideRemoveEvent( unsigned int i, bool hideDelete )
  {
    if( hideDelete )
    {
      eventVisibility( i, !_eventWidgets[ i ]->isVisible( ));
    }
    else
    {
      removeEvent( i );
    }
  }

  void Summary::hideRemoveSubset( unsigned int i, bool hideDelete )
  {
    if( hideDelete )
    {
      subsetVisibility( i, !_histogramWidgets[ i ]->isVisible( ));
    }
    else
    {
      removeSubset( i );
    }
  }

  void Summary::updateEventWidgets( void )
  {
    unsigned int counter = 0;
    for( auto e : _eventWidgets )
    {
      if( e->isVisible( ))
      {
        e->index( counter );
        ++counter;
      }

      e->update( );
    }

  }

  void Summary::updateHistogramWidgets( void )
  {
    unsigned int counter = 0;
    for( auto histogram : _histogramWidgets )
    {
      if( !histogram->isVisible( ))
        continue;

      histogram->firstHistogram( counter == 0 );
      ++counter;

      histogram->update( );
    }
  }

  void Summary::eventVisibility( unsigned int i, bool show )
  {
    EventRow& row = _eventRows[ i ];

    row.widget->setVisible( show );
    row.label->setVisible( show );

    _events[ i ].visible = show;

    updateEventWidgets( );

    for( auto h : _histogramWidgets )
      h->update( );
  }

  void Summary::subsetVisibility( unsigned int i, bool show )
  {
    HistogramRow& row = _histogramRows[ i ];

    row.histogram->setVisible( show );
    row.label->setVisible( show );

    updateHistogramWidgets( );
  }

  void Summary::clearEvents( void )
  {
    for( auto event : _eventRows )
    {
      _layoutEventLabels->removeWidget( event.label );
      _layoutEvents->removeWidget( event.widget );

      delete event.label;
      delete event.widget;

    }

    _events.clear( );
    _eventWidgets.clear( );
    _eventRows.clear( );

    updateEventWidgets( );

    _eventLabelsScroll->setVisible( false );
    _scrollEvent->setVisible( false );

    update( );
  }

  void Summary::removeEvent( unsigned int i )
  {
    auto& timeFrameRow = _eventRows[ i ];

    _layoutEventLabels->removeWidget( timeFrameRow.label );
    _layoutEvents->removeWidget( timeFrameRow.widget );

    delete timeFrameRow.label;
    delete timeFrameRow.widget;
//    delete timeFrameRow.checkBox;

    _events.erase( _events.begin( ) + i );
    _eventWidgets.erase( _eventWidgets.begin( ) + i );

    _eventRows.erase( _eventRows.begin( ) + i);

    updateEventWidgets( );

    if( _eventRows.size( ) == 0 )
    {
      _eventLabelsScroll->setVisible( false );
      _scrollEvent->setVisible( false );
    }

  }

  void Summary::removeSubset( unsigned int i )
  {

    auto& summaryRow = _histogramRows[ i ];

    // Avoid deleting last histogram
    if( _mainHistogram == summaryRow.histogram && _histogramRows.size( ) <= 1 )
        return;

    if( _focusedHistogram == summaryRow.histogram )
    {
      _focusedHistogram = nullptr;
      _focusWidget->clear( );
      _focusWidget->update( );
    }



    _layoutHistoLabels->removeWidget( summaryRow.label );
    _layoutHistograms->removeWidget( summaryRow.histogram );

    delete summaryRow.label;
    delete summaryRow.histogram;
    delete summaryRow.checkBox;

    _histogramWidgets.erase( _histogramWidgets.begin( ) + i );

    _histogramRows.erase( _histogramRows.begin( ) + i );

    updateHistogramWidgets( );

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

    for( auto histogram : _histogramWidgets )
    {
      histogram->colorScaleLocal( colorScale );
      histogram->CalculateColors( visimpl::HistogramWidget::T_HIST_MAIN );
      histogram->CalculateColors( visimpl::HistogramWidget::T_HIST_FOCUS );
      histogram->update( );
    }

    _focusWidget->update( );
  }

  visimpl::TColorScale Summary::colorScaleLocal( void )
  {
    return _colorScaleLocal;
  }

  void Summary::colorScaleGlobal( visimpl::TColorScale colorScale )
  {
    _colorScaleGlobal = colorScale;

    for( auto histogram : _histogramWidgets )
    {
      histogram->colorScaleGlobal( colorScale );
      histogram->CalculateColors( visimpl::HistogramWidget::T_HIST_MAIN );
      histogram->CalculateColors( visimpl::HistogramWidget::T_HIST_FOCUS );
      histogram->update( );
    }

    _focusWidget->update( );
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

    for( auto histogram : _histogramWidgets )
    {
      histogram->gridLinesNumber( linesNumber );
      histogram->update( );
    }
  }

  unsigned int Summary::gridLinesNumber( void )
  {
    return _gridLinesNumber;
  }

  void Summary::simulationPlayer( simil::SimulationPlayer* player )
  {
    _player = player;

    for( auto histogram : _histogramWidgets )
      histogram->simPlayer( _player );
  }

  void Summary::repaintHistograms( void )
  {
    for( auto histogram : _histogramWidgets )
      histogram->update( );
  }

  void Summary::_resizeCharts( unsigned int newMinSize, Qt::Orientation orientation )
  {
    for( auto histogram : _histogramRows )
    {
      if( orientation == Qt::Horizontal )
        histogram.histogram->setMinimumWidth( newMinSize );
      else
      {
        histogram.histogram->setMinimumHeight( newMinSize );
        histogram.histogram->setMaximumHeight( newMinSize );

        histogram.label->setMinimumHeight( newMinSize );
        histogram.label->setMaximumHeight( newMinSize );

        _scrollHistogram->setMinimumHeight( newMinSize );
      }
//        histogram->update( );
    }

//    for( auto histo : _histogramRows )
//    {
//      histo.
//    }
  }

  void Summary::_resizeEvents( unsigned int newMinSize )
  {
    for( auto subsetEventWidget : _eventWidgets )
    {
      subsetEventWidget->setMinimumWidth( newMinSize );
//        subsetEventWidget->update( );
    }
  }

  void Summary::_updateEventRepSizes( unsigned int newSize )
  {
    for( auto& e : _eventWidgets )
    {
      e->updateCommonRepSizeVert( newSize );
    }
  }

  void Summary::_updateScaleHorizontal( void )
  {
    _scaleCurrentHorizontal = std::max( 1.0, _spinBoxScaleHorizontal->value( ));

    unsigned int rightMarginFactor = 1;

    if( _scaleCurrentHorizontal == 1.0 )
      rightMarginFactor = 2;

    unsigned int viewSize = _scrollHistogram->width( );

     _sizeView = viewSize - _sizeMargin * rightMarginFactor;

    _sizeChartHorizontal = ( _scaleCurrentHorizontal * _sizeView );

   _resizeCharts( _sizeChartHorizontal, Qt::Horizontal );
   _resizeEvents( _sizeChartHorizontal );
  }

  void Summary::_updateScaleVertical( void )
  {
    _scaleCurrentVertical = std::max( 1.0, _spinBoxScaleVertical->value( ));

    _sizeChartVertical = _sizeChartVerticalDefault * _scaleCurrentVertical;

    _updateEventRepSizes( _sizeChartVertical );

    _resizeCharts( _sizeChartVertical, Qt::Vertical );
  }

  void Summary::resizeEvent( QResizeEvent* event_ )
  {
    QWidget::resizeEvent( event_ );

    if( ( _stackType != T_STACK_EXPANDABLE) || !_layoutMain || !_scrollHistogram )
      return;

    _layoutMain->activate( );
    _layoutHistograms->activate( );

    unsigned int splitterRightSide = _scrollHistogram->width( ); //_histoSplitter->sizes( )[ 1 ];

//
//    _sizeMargin =
//        ( _histogramScroll->contentsMargins( ).right( ) + _histogramsLayout->margin( ));
    _sizeChartHorizontal = _mainHistogram->size( ).width( );

    if( _flagUpdateChartSize && _mainHistogram->size( ).width( ) > 0)
    {
//      _sizeView = _mainHistogram->size( ).width( );

      _sizeMargin = ( splitterRightSide - _sizeChartHorizontal ) / 2;

      _flagUpdateChartSize = false;
    }

    unsigned int rightMarginFactor =
        splitterRightSide > _sizeChartHorizontal ? 2 : 1;
//
    _sizeView = splitterRightSide - _sizeMargin * rightMarginFactor;

    _scaleCurrentHorizontal = ( float )_sizeChartHorizontal / ( float )_sizeView;

    _spinBoxScaleHorizontal->setValue( _scaleCurrentHorizontal );

    std::cout << "RESIZE: view " << _sizeView
              << " charts " << _sizeChartHorizontal
              << " margin " << _sizeMargin
              << " scale " << _scaleCurrentHorizontal
              << std::endl;

    std::cout << "bottom left " << tmpFootLeft->size( ).width( )
              << ", " << tmpFootLeft->size( ).height( )
              << " - " << tmpFootRight->size( ).width( )
              << ", " << tmpFootRight->size( ).height( )
              << std::endl;

  }

  void Summary::wheelEvent( QWheelEvent* event_ )
  {

    float minScale = 1.0f;

    bool sign = ( event_->delta( ) > 0 );
    float adjustment = ( _scaleStep * ( sign ? 1 : ( -1 )));

    if( event_->modifiers( ).testFlag( Qt::ControlModifier ))
    {

      if( _scaleCurrentHorizontal == minScale )
        _sizeView = _mainHistogram->size( ).width( ) - _sizeMargin * 2;

      bool horizontalResize = false;
      auto mousePosition = event_->pos( );
      auto mousePosLocal = mapToGlobal( mousePosition );

      if( _scrollHistogram->geometry( ).contains( mousePosLocal ))
        horizontalResize = true;

      std::cout << "Mouse local (" << mousePosition.x( ) << ", " << mousePosition.y( )
                << " global (" << mousePosLocal.x( ) << ", " << mousePosLocal.y( )
                << ") over histograms " << std::boolalpha << horizontalResize
                << std::endl;

      _scaleCurrentHorizontal = std::max( minScale, _scaleCurrentHorizontal + adjustment );

      _sizeChartHorizontal = ( _scaleCurrentHorizontal * _sizeView );

      _resizeCharts( _sizeChartHorizontal, Qt::Horizontal );
      _resizeEvents( _sizeChartHorizontal );

      _spinBoxScaleHorizontal->setValue( _scaleCurrentHorizontal );

    }
    else if( event_->modifiers( ).testFlag( Qt::ShiftModifier ))
    {
      _scaleCurrentVertical =
          std::max( minScale, _scaleCurrentVertical + adjustment );

      _sizeChartVertical = ( _scaleCurrentVertical * _sizeChartVerticalDefault);

      _resizeCharts( _sizeChartVertical, Qt::Vertical );

      _spinBoxScaleVertical->setValue( _scaleCurrentVertical );

    }
    else
    {
      int scrollPos = 0;
      if( sender( ) == _scrollHistogram )
        scrollPos = _scrollHistogram->horizontalScrollBar( )->value( );
      else
        scrollPos = _scrollEvent->horizontalScrollBar( )->value( );

      moveVertScrollSync( scrollPos );

    }

//      float adjustment = ( _scaleStep * _scaleCurrent * ( sign ? 1 : ( -1 )));

//      std::cout << "ZOOM: view " << _sizeView
//                << " charts " << _sizeChartHorizontal
//                << " margin " << _sizeMargin
//                << " scale " << _scaleCurrentHorizontal
//                << std::endl;
//
//
//      std::cout << "Inner: " << _mainHistogram->size( ).width( ) << "x" << _mainHistogram->size( ).height( ) << " "
//                << "Outer: " << _scrollHistogram->size( ).width( ) << "x" << _scrollHistogram->size( ).height( ) << " "
//                << std::endl;


      updateRegionBounds( );

      event_->ignore( );
//      return;
//    }
//
//    event_->ignore( );
  }

  void Summary::moveHoriScrollSync( int /*action*/ )
  {
    if( _syncScrollsHorizontally &&
        ( sender( ) == _scrollEvent->horizontalScrollBar( ) ||
          sender( ) == _scrollHistogram->horizontalScrollBar( )))
    {
      auto author = dynamic_cast< QAbstractSlider* >( sender( ));
      int newPos = author->sliderPosition( );
      _scrollEvent->horizontalScrollBar( )->setValue( newPos );
      _scrollHistogram->horizontalScrollBar( )->setValue( newPos );
    }
  }

  void Summary::moveVertScrollSync( int /*action*/ )
  {
    if( _syncScrollsVertically )
    {

      if( sender( ) == _eventLabelsScroll->verticalScrollBar( ) ||
          sender( ) == _scrollEvent->verticalScrollBar( ))
      {
        auto author = dynamic_cast< QAbstractSlider* >( sender( ));
        int newPos = author->sliderPosition( );

        _eventLabelsScroll->verticalScrollBar( )->setValue( newPos );
        _scrollEvent->verticalScrollBar( )->setValue( newPos );
      }
      else if( sender( ) == _scrollHistoLabels->verticalScrollBar( ) ||
          sender( ) == _scrollHistogram->verticalScrollBar( ) )
      {
        auto author = dynamic_cast< QAbstractSlider* >( sender( ));
        int newPos = author->sliderPosition( );


        _scrollHistoLabels->verticalScrollBar( )->setValue( newPos );
        _scrollHistogram->verticalScrollBar( )->setValue( newPos );
      }
    }
  }

  void Summary::syncSplitters( )
  {
    if( sender( ) == _splitHorizEvents )
      _splitHorizHisto->setSizes( _splitHorizEvents->sizes( ));
    else
      _splitHorizEvents->setSizes( _splitHorizHisto->sizes( ));

    unsigned int splitterRightSide = _scrollHistogram->size( ).width( );

    unsigned int rightMarginFactor = splitterRightSide > _sizeChartHorizontal ? 2 : 1;

    _sizeView = splitterRightSide - ( _sizeMargin * rightMarginFactor );
    _sizeChartHorizontal =
        _mainHistogram ? _mainHistogram->size( ).width( ) : _sizeView * _scaleCurrentHorizontal;

    _scaleCurrentHorizontal = ( float )_sizeChartHorizontal / ( float )_sizeView;


//    std::cout << "Horizontal sizes " << horizontalSizes[ 0 ]
//              << "x" << horizontalSizes[ 1 ]
//              << " " << _histoSplitter->contentsMargins( ).left( )
//              << " " << _histoSplitter->contentsMargins( ).right( )
//              << " " << _histogramsLayout->contentsMargins( ).left( )
//              << " " << _histogramsLayout->contentsMargins( ).right( )
//              << " " << _histogramsLayout->spacing( )
//              << std::endl;

    std::cout << "SPLITTER: view " << _sizeView
              << " charts " << _sizeChartHorizontal
              << " margin " << _sizeMargin
              << " scale " << _scaleCurrentHorizontal
              << std::endl;

  }

  void Summary::adjustSplittersSize( )
  {
    int lhs = width( ) * 0.2;
    int rhs = width( ) - lhs;

    QList< int > sizes;
    sizes << lhs << rhs;

    _splitHorizEvents->setSizes( sizes );
    _splitHorizHisto->setSizes( sizes );
  }


  void Summary::focusPlayback( void )
  {
    float perc = _player->GetRelativeTime( );

    setFocusAt( perc );
  }

  void Summary::setFocusAt( float perc )
  {
    int max = _scrollHistogram->horizontalScrollBar( )->maximum( );
    int min = _scrollHistogram->horizontalScrollBar( )->minimum( );

    int value = ( max - min ) * perc;

    _scrollHistogram->horizontalScrollBar( )->setValue( value );
    _scrollEvent->horizontalScrollBar( )->setValue( value );
  }

}
