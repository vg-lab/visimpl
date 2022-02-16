/*
 * Copyright (c) 2015-2022 VG-Lab/URJC.
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

#include "Summary.h"

#include <QMouseEvent>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QInputDialog>
#include <QScrollBar>
#include <QApplication>
#include <QToolBox>
#include <QDebug>

unsigned int visimpl::Selection::_counter = 0;

constexpr unsigned int DEFAULT_BINS = 2500;
constexpr float DEFAULT_ZOOM_FACTOR = 1.5f;

constexpr float DEFAULT_SCALE = 1.0f;
constexpr float DEFAULT_SCALE_STEP = 0.3f;

static QString colorScaleToString( visimpl::TColorScale colorScale )
{
  switch( colorScale )
  {
    case visimpl::T_COLOR_LINEAR:
      return "Linear";
    case visimpl::T_COLOR_LOGARITHMIC:
      return "Logarithmic";
    default:
      return { };
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

    _initCentralGUI();
    auto footWidget = _initFootGUI();

    if( _stackType == T_STACK_EXPANDABLE )
    {
      _splitVertEventsHisto = new QSplitter( Qt::Vertical, this );
      _splitVertEventsHisto->addWidget( _splitHorizEvents );
      _splitVertEventsHisto->addWidget( _splitHorizHisto );
      _splitVertEventsHisto->addWidget( footWidget );
      _splitVertEventsHisto->setSizes( { 1000, 1000, 2000 } );

      _layoutMain->addWidget(_splitVertEventsHisto );
    }

  #ifdef VISIMPL_USE_ZEROEQ

    _insertionTimer.setSingleShot( false );
    _insertionTimer.setInterval( 250 );
    connect( &_insertionTimer, SIGNAL( timeout()),
             this, SLOT(deferredInsertion()));
    _insertionTimer.start();

  #endif

    // Fill the palette with ColorbBewer qualitative palette with 10 classes
    // but rearranging to have consecutive colors with different hue.
    _eventsPalette = scoop::ColorPalette::colorBrewerQualitative(
        scoop::ColorPalette::ColorBrewerQualitative::Set1, 9 );
  }

  void Summary::Init( simil::SimulationData* data_ )
  {
    _simData = data_;

    switch( data_->simulationType())
    {
      case simil::TSimSpikes:
      {
        auto *spikeData = dynamic_cast< simil::SpikeData * >( _simData );
        _spikeReport = spikeData;

        break;
      }
      default:
        break;
    }

    _gids = GIDUSet( data_->gids().begin(), data_->gids().end());

    Init();
  }

  void Summary::_initCentralGUI( )
  {
    if ( _stackType == T_STACK_FIXED )
    {
      _layoutHistograms = new QGridLayout( );
      this->setLayout( _layoutHistograms );
    }
    else if ( _stackType == T_STACK_EXPANDABLE )
    {
      _maxColumns = 20;
      _regionWidth = 0.1f;
      _summaryColumns = _maxColumns - 2;

      _layoutMain = new QVBoxLayout( );
      _layoutMain->setAlignment( Qt::AlignTop );
      this->setLayout( _layoutMain );

      _layoutEventLabels = new QGridLayout( );
      _layoutEventLabels->setAlignment( Qt::AlignTop );
      _layoutEventLabels->setVerticalSpacing( 0 );

      auto *eventLabelsContainer = new QWidget( );
      eventLabelsContainer->setLayout( _layoutEventLabels );
      eventLabelsContainer->setMaximumWidth( 150 );

      _eventLabelsScroll = new QScrollArea();
      _eventLabelsScroll->setWidgetResizable( true );
      _eventLabelsScroll->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
      _eventLabelsScroll->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
      _eventLabelsScroll->setVisible( false );
      _eventLabelsScroll->setWidget( eventLabelsContainer );

      _layoutEvents = new QGridLayout();
      _layoutEvents->setAlignment( Qt::AlignTop );
      _layoutEvents->setVerticalSpacing( 0 );

      auto *eventsContainer = new QWidget( );
      eventsContainer->setLayout( _layoutEvents );

      _scrollEvent = new QScrollArea( );
      _scrollEvent->setWidgetResizable( true );
      _scrollEvent->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
      _scrollEvent->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
      _scrollEvent->setVisible( false );
      _scrollEvent->setWidget( eventsContainer );

      connect(
        _scrollEvent->horizontalScrollBar( ) ,
        SIGNAL( actionTriggered( int )) ,
        this , SLOT( moveHoriScrollSync( int ))
      );

      connect(
        _scrollEvent->verticalScrollBar( ) , SIGNAL( actionTriggered( int )) ,
        this , SLOT( moveVertScrollSync( int ))
      );

      connect(
        _eventLabelsScroll->verticalScrollBar( ) ,
        SIGNAL( actionTriggered( int )) ,
        this , SLOT( moveVertScrollSync( int ))
      );

      _layoutHistoLabels = new QGridLayout( );
      _layoutHistoLabels->setAlignment( Qt::AlignTop );
      _layoutHistoLabels->setVerticalSpacing( 0 );

      auto *histogramLabelsContainer = new QWidget( );
      histogramLabelsContainer->setLayout( _layoutHistoLabels );
      histogramLabelsContainer->setMaximumWidth( 150 );

      _scrollHistoLabels = new QScrollArea( );
      _scrollHistoLabels->setWidgetResizable( true );
      _scrollHistoLabels->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
      _scrollHistoLabels->setWidget( histogramLabelsContainer );

      _layoutHistograms = new QGridLayout( );
      _layoutHistograms->setAlignment( Qt::AlignTop );
      _layoutHistograms->setVerticalSpacing( 0 );

      auto *histogramsContainer = new QWidget( );
      histogramsContainer->setLayout( _layoutHistograms );

      _scrollHistogram = new QScrollArea( );
      _scrollHistogram->setWidgetResizable( true );
      _scrollHistogram->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
      _scrollHistogram->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
      _scrollHistogram->setWidget( histogramsContainer );
      _scrollHistogram->horizontalScrollBar( )->setMinimum( 0 );
      _scrollHistogram->horizontalScrollBar( )->setMaximum( 1000 );

      connect(
        _scrollHistogram->horizontalScrollBar( ) ,
        SIGNAL( actionTriggered( int )) ,
        this , SLOT( moveHoriScrollSync( int ))
      );

      connect(
        _scrollHistogram->verticalScrollBar( ) ,
        SIGNAL( actionTriggered( int )) ,
        this , SLOT( moveVertScrollSync( int ))
      );

      connect(
        _scrollHistoLabels->verticalScrollBar( ) ,
        SIGNAL( actionTriggered( int )) ,
        this , SLOT( moveVertScrollSync( int ))
      );

      _splitHorizEvents = new QSplitter( Qt::Horizontal );
      _splitHorizHisto = new QSplitter( Qt::Horizontal );

      _splitHorizEvents->addWidget( _eventLabelsScroll );
      _splitHorizEvents->addWidget( _scrollEvent );

      _splitHorizHisto->addWidget( _scrollHistoLabels );
      _splitHorizHisto->addWidget( _scrollHistogram );

      connect(
        _splitHorizEvents , SIGNAL( splitterMoved( int , int )) ,
        this , SLOT( syncSplitters( ))
      );

      connect(
        _splitHorizHisto , SIGNAL( splitterMoved( int , int )) ,
        this , SLOT( syncSplitters( ))
      );
    }
  }

  QWidget* Summary::_initFootGUI( void )
  {
    auto *foot = new QWidget( );
    auto *footLayout = new QGridLayout( );

    auto *toolBox = new QToolBox( );
    toolBox->setMaximumWidth( 300 );

    // FOCUS

    _focusWidget = new FocusFrame( );
    _focusWidget->colorLocal( _colorLocal );
    _focusWidget->colorGlobal( _colorGlobal );
    _focusWidget->setMinimumHeight( 200 );
    _focusWidget->setMinimumWidth( 200 );

    // NORMALIZATION

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

    QStringList csItems;
    csItems.push_back( QString( colorScaleToString( visimpl::T_COLOR_LINEAR )));
    csItems.push_back(
      QString( colorScaleToString( visimpl::T_COLOR_LOGARITHMIC )));

    auto *localComboBox = new QComboBox( );
    localComboBox->addItems( csItems );

    auto *globalComboBox = new QComboBox( );
    globalComboBox->addItems( csItems );

    auto *groupBoxNorm = new QWidget( );
    auto *layoutNorm = new QGridLayout( );
    layoutNorm->setAlignment( Qt::AlignTop );
    groupBoxNorm->setLayout( layoutNorm );

    layoutNorm->addWidget( new QLabel( "Local:" ) , 0 , 0 , 1 , 1 );
    layoutNorm->addWidget( _localColorWidget , 0 , 1 , 1 , 1 );
    layoutNorm->addWidget( localComboBox , 0 , 2 , 1 , 1 );

    layoutNorm->addWidget( new QLabel( "Global:" ) , 1 , 0 , 1 , 1 );
    layoutNorm->addWidget( _globalColorWidget , 1 , 1 , 1 , 1 );
    layoutNorm->addWidget( globalComboBox , 1 , 2 , 1 , 1 );

    localComboBox->setCurrentIndex(( int ) _colorScaleLocal );
    globalComboBox->setCurrentIndex(( int ) _colorScaleGlobal );

    connect(
      localComboBox , SIGNAL( currentIndexChanged( int )) ,
      this , SLOT( colorScaleLocal( int ))
    );

    connect(
      globalComboBox , SIGNAL( currentIndexChanged( int )) ,
      this , SLOT( colorScaleGlobal( int ))
    );

    // SCALE ADJUSTMENT

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

    auto *groupBoxScale = new QWidget( );
    auto *layoutScale = new QGridLayout( );
    layoutScale->setAlignment( Qt::AlignTop );
    groupBoxScale->setLayout( layoutScale );
    layoutScale->addWidget( new QLabel( "Horizontal: " ) , 0 , 0 , 1 , 1 );
    layoutScale->addWidget( _spinBoxScaleHorizontal , 0 , 1 , 1 , 1 );
    layoutScale->addWidget( new QLabel( "Vertical: " ) , 1 , 0 , 1 , 1 );
    layoutScale->addWidget( _spinBoxScaleVertical , 1 , 1 , 1 , 1 );

    connect(
      _spinBoxScaleHorizontal , SIGNAL( editingFinished( void )) ,
      this , SLOT( _updateScaleHorizontal( void ))
    );

    connect(
      _spinBoxScaleVertical , SIGNAL( editingFinished( void )) ,
      this , SLOT( _updateScaleVertical( void ))
    );

    // BIN CONFIGURATION

    _spinBoxBins = new QSpinBox( );
    _spinBoxBins->setMinimum( 50 );
    _spinBoxBins->setMaximum( 100000 );
    _spinBoxBins->setSingleStep( 50 );
    _spinBoxBins->setValue( static_cast<int>(_bins));

    _spinBoxZoomFactor = new QDoubleSpinBox( );
    _spinBoxZoomFactor->setMinimum( 1.0 );
    _spinBoxZoomFactor->setMaximum( 1000.0 );
    _spinBoxZoomFactor->setSingleStep( 0.5 );
    _spinBoxZoomFactor->setValue( _zoomFactor );

    auto *groupBoxBinConfig = new QWidget( );
    auto *layoutBinConfig = new QGridLayout( );
    layoutBinConfig->setAlignment( Qt::AlignTop );
    groupBoxBinConfig->setLayout( layoutBinConfig );

    layoutBinConfig->addWidget( new QLabel( "Bins number:" ) , 0 , 0 , 1 , 1 );
    layoutBinConfig->addWidget( _spinBoxBins , 0 , 1 , 1 , 1 );
    layoutBinConfig->addWidget( new QLabel( "Zoom factor:" ) , 1 , 0 , 1 , 1 );
    layoutBinConfig->addWidget( _spinBoxZoomFactor , 1 , 1 , 1 , 1 );

    connect(
      _spinBoxBins , SIGNAL( editingFinished( void )) ,
      this , SLOT( binsChanged( void ))
    );

    connect(
      _spinBoxZoomFactor , SIGNAL( editingFinished( void )) ,
      this , SLOT( zoomFactorChanged( void ))
    );

    // DATA INSPECTOR

    _currentValueLabel = new QLabel( "" );
    _currentValueLabel->setMaximumWidth( 50 );
    _globalMaxLabel = new QLabel( "" );
    _globalMaxLabel->setMaximumWidth( 50 );
    _localMaxLabel = new QLabel( "" );
    _localMaxLabel->setMaximumWidth( 50 );

    auto *groupBoxInformation = new QWidget( );
    auto *layoutInformation = new QGridLayout( );
    layoutInformation->setAlignment( Qt::AlignTop );
    groupBoxInformation->setLayout( layoutInformation );

    layoutInformation->addWidget( new QLabel( "Current value: " ) , 2 , 9 , 1 ,
                                  2 );
    layoutInformation->addWidget( _currentValueLabel , 2 , 11 , 1 , 1 );
    layoutInformation->addWidget( new QLabel( "Local max: " ) , 3 , 9 , 1 , 2 );
    layoutInformation->addWidget( _localMaxLabel , 3 , 11 , 1 , 1 );
    layoutInformation->addWidget( new QLabel( "Global max: " ) , 4 , 9 , 1 ,
                                  2 );
    layoutInformation->addWidget( _globalMaxLabel , 4 , 11 , 1 , 1 );

    // RULE CONFIGURATION

    auto *gridSpinBox = new QSpinBox( );
    gridSpinBox->setMinimum( 0 );
    gridSpinBox->setMaximum( 10000 );
    gridSpinBox->setSingleStep( 1 );
    gridSpinBox->setValue( 3 );

    auto *groupBoxRuleConfig = new QWidget( );
    auto *layoutRuleConfig = new QGridLayout( );
    layoutRuleConfig->setAlignment( Qt::AlignTop );
    groupBoxRuleConfig->setLayout( layoutRuleConfig );

    layoutRuleConfig->addWidget( new QLabel( "Rule sectors: " ) , 0 , 0 , 1 ,
                                 1 );
    layoutRuleConfig->addWidget( gridSpinBox , 0 , 1 , 1 , 1 );

    connect(
      gridSpinBox , SIGNAL( valueChanged( int )) ,
      this , SLOT( gridLinesNumber( int ))
    );

    //

    toolBox->addItem( groupBoxNorm , "Normalization" );
    toolBox->addItem( groupBoxScale , "Scale adjustment" );
    toolBox->addItem( groupBoxBinConfig , "Bin configuration" );
    toolBox->addItem( groupBoxInformation , "Data inspector" );
    toolBox->addItem( groupBoxRuleConfig , "Rule configuration" );

    footLayout->addWidget( _focusWidget , 0 , 0 , 1 , 1 );
    footLayout->addWidget( toolBox , 0 , 1 , 1 , 1 );

    foot->setLayout( footLayout );

    return foot;
  }

  void Summary::Init()
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
      const QString text{"All"};

      HistogramRow mainRow;

      mainRow.histogram = _mainHistogram;
      mainRow.histogram->_events = &_events;
      mainRow.histogram->name( text.toStdString() );

      mainRow.label = new QLabel( text );
      mainRow.label->setMinimumWidth( _maxLabelWidth );
      mainRow.label->setMaximumWidth( _maxLabelWidth );
      mainRow.label->setMinimumHeight( _heightPerRow );
      mainRow.label->setMaximumHeight( _heightPerRow );

      const unsigned int row = _histogramWidgets.size();
      _layoutHistoLabels->addWidget( mainRow.label, row , 0, 1, 1 );
      _layoutHistograms->addWidget( _mainHistogram, row, 1, 1, _summaryColumns );

      _histogramRows.push_back( mainRow );
      _histogramWidgets.push_back( _mainHistogram );

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

    update();
  }

  void Summary::generateEventsRep( void )
  {
    if( _simData->subsetsEvents()->numEvents() > 0 )
    {
      _eventLabelsScroll->setVisible( true );
      _scrollEvent->setVisible( true );

      simil::EventRange timeFrames = _simData->subsetsEvents()->events();

      float invTotal = 1.0f / ( _spikeReport->endTime() - _spikeReport->startTime());

      unsigned int counter = 0;
      for( auto it = timeFrames.first; it != timeFrames.second; ++it, ++counter )
      {
        TEvent timeFrame;
        timeFrame.name = it->first;
        timeFrame.visible = true;

        for( auto time : it->second )
        {

          float startPercentage =
              std::max( 0.0f,
                        ( time.first - _spikeReport->startTime()) * invTotal);

          float endPercentage =
              std::min( 1.0f,
                        ( time.second - _spikeReport->startTime()) * invTotal);

          timeFrame.percentages.push_back(
              std::make_pair( startPercentage, endPercentage ));
        }

        timeFrame.color = _eventsPalette.colors()[
          counter /* %  _eventsPalette.size() */ ];

        _events.push_back( timeFrame );

        QLabel* label = new QLabel( timeFrame.name.c_str());
        label->setMinimumHeight( 20 );
        label->setMinimumWidth( _maxLabelWidth );
        label->setMaximumWidth( _maxLabelWidth );
        label->setMinimumHeight( _heightPerRow );
        label->setMaximumHeight( _heightPerRow );
        label->setToolTip( timeFrame.name.c_str());

        EventWidget* eventWidget = new EventWidget();
        eventWidget->name( timeFrame.name );
        eventWidget->setSizePolicy( QSizePolicy::Expanding,
                                    QSizePolicy::Expanding );
        eventWidget->timeFrames( &_events );
        eventWidget->setMinimumWidth( _sizeChartHorizontal );
        eventWidget->setMinimumHeight( _heightPerRow );
        eventWidget->setMaximumHeight( _heightPerRow );
        eventWidget->index( counter );

        EventRow eventrow;
        eventrow.widget = eventWidget;
        eventrow.label = label;

        _eventRows.push_back( eventrow );
        _eventWidgets.push_back( eventWidget );

        _layoutEventLabels->addWidget( label, counter, 0, 1, 1 );
        _layoutEvents->addWidget( eventWidget, counter, 1, 1, _summaryColumns );
      }
    }
  }

  void Summary::importSubsetsFromSubsetMngr( void )
  {
    simil::SubsetMapRange subsets =
        _spikeReport->subsetsEvents()->subsets();

    for( auto it = subsets.first; it != subsets.second; ++it )
    {
      GIDUSet subset( it->second.begin(), it->second.end());
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

    if( selected.size() == _gids.size() || selected.size() == 0)
      return;


    if( _stackType == TStackType::T_STACK_EXPANDABLE )
    {
  #ifdef VISIMPL_USE_ZEROEQ

      if( deferred )
      {
        _pendingSelections.push_back( selection );
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
    if( _pendingSelections.size() > 0 )
    {
      auto selection = _pendingSelections.front();
      _pendingSelections.pop_front();

      QString labelText = "Selection ";
      if(selection.id != 0)
      {
        labelText += QString::number(selection.id);
      }
      else
      {
        labelText += QString::number(histogramsNumber());
      }

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

      selection.name = labelText.toStdString();

      insertSubset( selection );
    }

  }

  #endif

  void Summary::UpdateHistograms( void )
  {
    auto updateHistogram = [&](HistogramWidget *h)
    {
      h->Spikes(*_spikeReport);
      h->Update();
      h->update();
    };
    std::for_each(_histogramWidgets.begin(), _histogramWidgets.end(), updateHistogram);
  }

  void Summary::insertSubset( const Selection& selection )
  {
    insertSubset( selection.name, selection.gids );
  }

  void Summary::insertSubset( const std::string& name, const GIDUSet& subset )
  {
    HistogramRow currentRow;

    auto histogram = new visimpl::HistogramWidget( *_spikeReport );

    histogram->filteredGIDs( subset );
    histogram->name( name );
    histogram->colorMapper( _mainHistogram->colorMapper());
    histogram->colorScaleLocal( _colorScaleLocal );
    histogram->colorScaleGlobal( _colorScaleGlobal );
    histogram->colorLocal( _colorLocal );
    histogram->colorGlobal( _colorGlobal );
    histogram->normalizeRule( visimpl::T_NORM_MAX );
    histogram->representationMode( visimpl::T_REP_CURVE );
    histogram->regionWidth( _regionWidth );
    histogram->gridLinesNumber( _gridLinesNumber );

    histogram->init( _bins, _zoomFactor );

    if( histogram->empty())
    {
      delete histogram;
      return;
    }

    histogram->setMinimumHeight( _heightPerRow );
    histogram->setMaximumHeight( _heightPerRow );
    histogram->setMinimumWidth( _sizeChartHorizontal );

    histogram->simPlayer( _player );

    histogram->_events = &_events;

    currentRow.histogram = histogram;
    currentRow.label = new QLabel( name.c_str());
    currentRow.label->setMinimumWidth( _maxLabelWidth );
    currentRow.label->setMaximumWidth( _maxLabelWidth );
    currentRow.label->setMinimumHeight( _heightPerRow );
    currentRow.label->setMaximumHeight( _heightPerRow );
    currentRow.label->setToolTip( name.c_str());

    const unsigned int row = _histogramWidgets.size();
    _layoutHistoLabels->addWidget( currentRow.label, row , 0, 1, 1 );
    _layoutHistograms->addWidget( histogram, row, 1, 1, _summaryColumns );

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

    update();
  }

  void Summary::childHistogramPressed( const QPoint& position, float /*percentage*/ )
  {
    if( _stackType == T_STACK_FIXED )
    {
      const QPoint cursorLocalPoint = _mainHistogram->mapFromGlobal( position );
      const float percentage = float( cursorLocalPoint.x()) / _mainHistogram->width();

      emit histogramClicked( percentage );
      return;
    }

    if( _focusedHistogram != sender())
    {
      _focusedHistogram =
              dynamic_cast< visimpl::HistogramWidget* >( sender());

      _focusedHistogram->regionWidth( _regionWidth );
    }

    const QPoint cursorLocalPoint = _focusedHistogram->mapFromGlobal( position );
    const float percentage = float( cursorLocalPoint.x()) / _focusedHistogram->width();

    _currentValueLabel->setText(
        QString::number( _focusedHistogram->focusValueAt( percentage )));
    _localMaxLabel->setText( QString::number( _focusedHistogram->focusMaxLocal()));
    _globalMaxLabel->setText( QString::number( _focusedHistogram->focusMaxGlobal()));

    if( _overRegionEdgeLower )
    {
      _selectedEdgeLower = true;
      _selectedEdgeUpper = false;
    }
    else if( _overRegionEdgeUpper )
    {
      _selectedEdgeLower = false;
      _selectedEdgeUpper = true;
    }
    else
    {
      _regionGlobalPosition = position;
      _regionWidthPixels = _regionWidth * _focusedHistogram->width();

       SetFocusRegionPosition( cursorLocalPoint );

      _selectedEdgeLower = _selectedEdgeUpper = false;
    }

    _mousePressed = true;

    auto focusedHistogram = _focusedHistogram;
    auto setPaintRegion = [&focusedHistogram](HistogramWidget *w)
    {
      w->paintRegion(w == focusedHistogram);
      w->update();
    };
    std::for_each(_histogramWidgets.begin(), _histogramWidgets.end(), setPaintRegion);
  }

  void Summary::childHistogramReleased( const QPoint& /*position*/, float /*percentage*/ )
  {
    _mousePressed = false;
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
            dynamic_cast< visimpl::HistogramWidget* >( sender()));
      else if( modifiers == Qt::ShiftModifier )
        emit histogramClicked( percentage );
    }
  }

  void Summary::mouseMoveEvent( QMouseEvent* event_ )
  {
    QWidget::mouseMoveEvent( event_ );

    QApplication::setOverrideCursor( Qt::ArrowCursor );
  }

  constexpr int regionEditMarginPixels = 2;

  void Summary::SetFocusRegionPosition( const QPoint& cursorLocalPosition )
  {
    if( _stackType == T_STACK_FIXED )
      return;

    _regionLocalPosition = cursorLocalPosition;

    // Limit the region position with borders
    _regionLocalPosition.setX( std::max( _regionWidthPixels, _regionLocalPosition.x(  )));
    _regionLocalPosition.setX( std::min( _regionLocalPosition.x(  ),
                          _focusedHistogram->width() - _regionWidthPixels));

    setFocusAt( static_cast<float>(_regionLocalPosition.x()) / _focusedHistogram->width() );
  }

  void Summary::updateRegionBounds( void )
  {
    if( _focusedHistogram )
    {
      _regionLocalPosition =
          QPoint( _focusedHistogram->width() * _regionPercentage,
                  _focusedHistogram->height() * 0.5f );

      _regionWidthPixels = _regionWidth * _focusedHistogram->width();

      _regionLocalPosition.setX(
          std::max( _regionWidthPixels, _regionLocalPosition.x(  )));

      _regionLocalPosition.setX( std::min( _regionLocalPosition.x(  ),
                            _focusedHistogram->width() - _regionWidthPixels));

      calculateRegionBounds();

      _focusWidget->update();
    }
  }

  void Summary::calculateRegionBounds( void )
  {
    _regionEdgeLower = std::max( _regionPercentage - _regionWidth, _regionWidth );
    _regionEdgePointLower = _regionLocalPosition.x() - _regionWidthPixels;
    _regionEdgeUpper = std::min( _regionPercentage + _regionWidth, 1.0f - _regionWidth);
    _regionEdgePointUpper = _regionLocalPosition.x() + _regionWidthPixels;
  }

  void Summary::updateMouseMarker( QPoint point )
  {
    auto focusedHistogram = qobject_cast< visimpl::HistogramWidget* >( sender());
    if(!focusedHistogram)
      return;

    _lastMousePosition = point;

    if( _stackType != T_STACK_EXPANDABLE )
    {
      _mainHistogram->update();
      return;
    }

    const float invWidth = 1.0f / float( focusedHistogram->width());
    const QPoint cursorLocalPoint = focusedHistogram->mapFromGlobal( point );

    if( focusedHistogram == _focusedHistogram )
    {
      if( _mousePressed )
      {
        if( _selectedEdgeLower )
        {

          float diffPerc = ( cursorLocalPoint.x() - _regionEdgePointLower ) * invWidth;

          _regionWidth -= diffPerc;

          const float regionMinSize = 5.0f / focusedHistogram->width();

          _regionWidth = std::max( regionMinSize, std::min( 0.5f, _regionWidth ));

          _regionWidthPixels = _regionWidth * _focusedHistogram->width();

          calculateRegionBounds();

          _focusWidget->viewRegion( *_focusedHistogram, _regionPercentage, _regionWidth );
          _focusWidget->update();

          QApplication::setOverrideCursor( Qt::SizeHorCursor );
        }
        else if ( _selectedEdgeUpper )
        {
          const float diffPerc = ( cursorLocalPoint.x() - _regionEdgePointUpper ) * invWidth;

          _regionWidth += diffPerc;

          const float regionMinSize = 5.0f / focusedHistogram->width();

          _regionWidth = std::max( regionMinSize,
                                   std::min( 0.5f, _regionWidth ));

          _regionWidthPixels = _regionWidth * _focusedHistogram->width();

          calculateRegionBounds();

          _focusWidget->viewRegion( *_focusedHistogram, _regionPercentage,
                                    _regionWidth );
          _focusWidget->update();

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
        if( abs(cursorLocalPoint.x() - _regionEdgePointLower) < regionEditMarginPixels )
        {
          _overRegionEdgeLower = true;

          QApplication::setOverrideCursor( Qt::SizeHorCursor );
        }
        else if( abs(_regionEdgePointUpper - cursorLocalPoint.x()) < regionEditMarginPixels )
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

    if( _stackType == T_STACK_EXPANDABLE && _mousePressed )
    {
      auto setPaintRegion = [&focusedHistogram](HistogramWidget *w)
      {
        w->paintRegion(w == focusedHistogram);
        w->update();
      };
      std::for_each(_histogramWidgets.begin(), _histogramWidgets.end(), setPaintRegion);
    }
  }

  void Summary::CreateSummarySpikes()
  {
    auto initHistogram = [this](HistogramWidget *w)
    {
      if( !w->isInitialized() )
        w->init( _bins, _zoomFactor );
    };
    std::for_each(_histogramWidgets.begin(), _histogramWidgets.end(), initHistogram);
  }

  void Summary::UpdateGradientColors( bool replace )
  {
    auto updateColors = [&replace](HistogramWidget *w)
    {
      if( !w->isInitialized() || replace)
        w->CalculateColors();
    };
    std::for_each( _histogramWidgets.begin(), _histogramWidgets.end(), updateColors);
  }

  unsigned int Summary::histogramsNumber( void )
  {
    return _histogramWidgets.size();
  }

  void Summary::binsChanged( void )
  {
    const unsigned int binsNumber = _spinBoxBins->value();
    bins( binsNumber );
  }

  void Summary::bins( unsigned int bins_ )
  {
    if(_bins == bins_) return;

    _bins = bins_;

#ifdef VISIMPL_USE_OPENMP
    #pragma omp parallel for
    for( int i = 0; i < static_cast<int>(_histogramWidgets.size()); ++i )
    {
      auto histogram = _histogramWidgets[ i ];
#else
    for( auto histogram : _histogramWidgets )
    {
#endif
      histogram->bins( _bins );
      histogram->update();
    }

    if(_focusedHistogram)
    {
      _focusWidget->viewRegion( *_focusedHistogram, _regionPercentage, _regionWidth );
      _focusWidget->update();
    }
  }

  void Summary::zoomFactorChanged( void )
  {
    double value = _spinBoxZoomFactor->value();

    zoomFactor( value );
  }

  void Summary::zoomFactor( double zoom )
  {
    if(_zoomFactor == zoom) return;

    _zoomFactor = zoom;

#ifdef VISIMPL_USE_OPENMP
    #pragma omp parallel for
    for( int i = 0; i < static_cast<int>(_histogramWidgets.size()); ++i )
    {
      auto histogram = _histogramWidgets[ i ];
#else
    for( auto histogram : _histogramWidgets )
    {
#endif
      histogram->zoomFactor( _zoomFactor );
      histogram->update();
    }
  }

  void Summary::fillPlots( bool fillPlots_ )
  {
    _fillPlots = fillPlots_;

    auto setFill = [&fillPlots_](HistogramWidget *w)
    {
      w->fillPlots( fillPlots_ );
      w->update();
    };
    std::for_each(_histogramWidgets.begin(), _histogramWidgets.end(), setFill);

    _focusWidget->fillPlots( _fillPlots );
    _focusWidget->update();
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

    auto updateHeight = [&height_](HistogramWidget *w)
    {
      w->setMinimumHeight( height_ );
      w->setMaximumHeight( height_ );
      w->update();
    };
    std::for_each(_histogramWidgets.begin(), _histogramWidgets.end(), updateHeight);
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
      eventVisibility( i, !_eventWidgets[ i ]->isVisible());
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
      subsetVisibility( i, !_histogramWidgets[ i ]->isVisible());
    }
    else
    {
      removeSubset( i );
    }
  }

  void Summary::updateEventWidgets( void )
  {
    unsigned int counter = 0;

    auto updateWidget = [&counter](EventWidget *w)
    {
      if( w->isVisible())
      {
        w->index( counter );
        ++counter;
      }

      w->update();
    };
    std::for_each(_eventWidgets.begin(), _eventWidgets.end(), updateWidget);
  }

  void Summary::updateHistogramWidgets( void )
  {
    unsigned int counter = 0;

    auto updateHistogram = [&counter](HistogramWidget *w)
    {
      if( !w->isVisible())
        return;

      w->firstHistogram( counter == 0 );
      ++counter;

      w->update();
    };
    std::for_each(_histogramWidgets.begin(), _histogramWidgets.end(), updateHistogram);
  }

  void Summary::eventVisibility( unsigned int i, bool show )
  {
    EventRow& row = _eventRows[ i ];

    row.widget->setVisible( show );
    row.label->setVisible( show );

    _events[ i ].visible = show;

    updateEventWidgets();

    repaintHistograms();
  }

  void Summary::subsetVisibility( unsigned int i, bool show )
  {
    HistogramRow& row = _histogramRows[ i ];

    row.histogram->setVisible( show );
    row.label->setVisible( show );

    updateHistogramWidgets();
  }

  void Summary::clearEvents( void )
  {
    auto removeRow = [this](visimpl::Summary::EventRow &event)
    {
      _layoutEventLabels->removeWidget( event.label );
      _layoutEvents->removeWidget( event.widget );

      delete event.label;
      delete event.widget;
    };
    std::for_each(_eventRows.begin(), _eventRows.end(), removeRow);

    _events.clear();
    _eventWidgets.clear();
    _eventRows.clear();

    updateEventWidgets();

    _eventLabelsScroll->setVisible( false );
    _scrollEvent->setVisible( false );

    update();
  }

  void Summary::removeEvent( unsigned int i )
  {
    auto& timeFrameRow = _eventRows[ i ];

    _layoutEventLabels->removeWidget( timeFrameRow.label );
    _layoutEvents->removeWidget( timeFrameRow.widget );

    delete timeFrameRow.label;
    delete timeFrameRow.widget;

    _events.erase( _events.begin() + i );
    _eventWidgets.erase( _eventWidgets.begin() + i );

    _eventRows.erase( _eventRows.begin() + i);

    updateEventWidgets();

    if( _eventRows.size() == 0 )
    {
      _eventLabelsScroll->setVisible( false );
      _scrollEvent->setVisible( false );
    }
  }

  void Summary::removeSubset( unsigned int i )
  {
    auto& summaryRow = _histogramRows[ i ];

    // Avoid deleting last histogram
    if( _mainHistogram == summaryRow.histogram && _histogramRows.size() <= 1 )
        return;

    if( _focusedHistogram == summaryRow.histogram )
    {
      _focusedHistogram = nullptr;
      _focusWidget->clear();
      _focusWidget->update();
    }

    _layoutHistoLabels->removeWidget( summaryRow.label );
    _layoutHistograms->removeWidget( summaryRow.histogram );

    delete summaryRow.label;
    delete summaryRow.histogram;
    delete summaryRow.checkBox;

    _histogramWidgets.erase( _histogramWidgets.begin() + i );

    _histogramRows.erase( _histogramRows.begin() + i );

    updateHistogramWidgets();
  }

  void Summary::colorScaleLocal( int value )
  {
    if( value >= 0 && value < visimpl::T_COLOR_UNDEFINED )
    {
      colorScaleLocal( static_cast<visimpl::TColorScale>(value) );
    }
  }

  void Summary::colorScaleGlobal( int value )
  {
    if( value >= 0 && value < visimpl::T_COLOR_UNDEFINED )
    {
      colorScaleGlobal( static_cast<visimpl::TColorScale>(value) );
    }
  }

  void Summary::colorScaleLocal( visimpl::TColorScale colorScale )
  {
    _colorScaleLocal = colorScale;

    auto setScaleLocal = [&colorScale](HistogramWidget *w)
    {
      w->colorScaleLocal( colorScale );
      w->CalculateColors( visimpl::HistogramWidget::T_HIST_MAIN );
      w->CalculateColors( visimpl::HistogramWidget::T_HIST_FOCUS );
      w->update();
    };
    std::for_each(_histogramWidgets.begin(), _histogramWidgets.end(), setScaleLocal);

    if(_focusedHistogram)
    {
      _focusWidget->viewRegion( *_focusedHistogram, _regionPercentage, _regionWidth );
    }
    _focusWidget->update();
  }

  visimpl::TColorScale Summary::colorScaleLocal( void )
  {
    return _colorScaleLocal;
  }

  void Summary::colorScaleGlobal( visimpl::TColorScale colorScale )
  {
    _colorScaleGlobal = colorScale;

    auto setScaleGlobal = [&colorScale](HistogramWidget *w)
    {
      w->colorScaleGlobal( colorScale );
      w->CalculateColors( visimpl::HistogramWidget::T_HIST_MAIN );
      w->CalculateColors( visimpl::HistogramWidget::T_HIST_FOCUS );
      w->update();
    };
    std::for_each(_histogramWidgets.begin(), _histogramWidgets.end(), setScaleGlobal);

    if(_focusedHistogram)
    {
      _focusWidget->viewRegion( *_focusedHistogram, _regionPercentage, _regionWidth );
    }
    _focusWidget->update();
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

    auto updateGridLinesNumber = [&linesNumber](HistogramWidget *w)
    {
      w->gridLinesNumber( linesNumber );
      w->update();
    };
    std::for_each(_histogramWidgets.begin(), _histogramWidgets.end(), updateGridLinesNumber);
  }

  unsigned int Summary::gridLinesNumber( void )
  {
    return _gridLinesNumber;
  }

  void Summary::simulationPlayer( simil::SimulationPlayer* player )
  {
    _player = player;

    auto assignPlayer = [&player](HistogramWidget *w)
    {
      w->simPlayer(player);
    };
    std::for_each(_histogramWidgets.begin(), _histogramWidgets.end(), assignPlayer);
  }

  void Summary::repaintHistograms( void )
  {
    auto updateHistograms = [](HistogramWidget *w)
    {
      w->update();
    };
    std::for_each(_histogramWidgets.begin(), _histogramWidgets.end(), updateHistograms);
  }

  void Summary::_resizeCharts( unsigned int newMinSize, Qt::Orientation orientation )
  {
    auto resizeHistogramRow = [&newMinSize, &orientation](visimpl::Summary::HistogramRow &h)
    {
      if( orientation == Qt::Horizontal )
        h.histogram->setMinimumWidth( newMinSize );
      else
      {
        h.histogram->setMinimumHeight( newMinSize );
        h.histogram->setMaximumHeight( newMinSize );

        h.label->setMinimumHeight( newMinSize );
        h.label->setMaximumHeight( newMinSize );
      }
    };
    std::for_each(_histogramRows.begin(), _histogramRows.end(), resizeHistogramRow);

    if(orientation != Qt::Horizontal)
      _scrollHistogram->setMinimumHeight( newMinSize );
  }

  void Summary::_resizeEvents( unsigned int newMinSize )
  {
    auto updateEventWidth = [&newMinSize](EventWidget *w)
    {
      w->setMinimumWidth(newMinSize);
    };
    std::for_each(_eventWidgets.begin(), _eventWidgets.end(), updateEventWidth);
  }

  void Summary::_updateEventRepSizes( unsigned int newSize )
  {
    auto updateEventSize = [&newSize](EventWidget *w)
    {
      w->updateCommonRepSizeVert(newSize);
    };
    std::for_each(_eventWidgets.begin(), _eventWidgets.end(), updateEventSize);
  }

  void Summary::_updateScaleHorizontal( void )
  {
    _scaleCurrentHorizontal = std::max( 1.0, _spinBoxScaleHorizontal->value());

    unsigned int rightMarginFactor = 1;

    if( _scaleCurrentHorizontal == 1.0 )
      rightMarginFactor = 2;

    const unsigned int viewSize = _scrollHistogram->width();

     _sizeView = viewSize - _sizeMargin * rightMarginFactor;

    _sizeChartHorizontal = ( _scaleCurrentHorizontal * _sizeView );

   _resizeCharts( _sizeChartHorizontal, Qt::Horizontal );
   _resizeEvents( _sizeChartHorizontal );
  }

  void Summary::_updateScaleVertical( void )
  {
    _scaleCurrentVertical = std::max( 1.0, _spinBoxScaleVertical->value());

    _sizeChartVertical = _sizeChartVerticalDefault * _scaleCurrentVertical;

    _updateEventRepSizes( _sizeChartVertical );

    _resizeCharts( _sizeChartVertical, Qt::Vertical );
  }

  void Summary::resizeEvent( QResizeEvent* event_ )
  {
    QWidget::resizeEvent( event_ );

    if( ( _stackType != T_STACK_EXPANDABLE) || !_layoutMain || !_scrollHistogram )
      return;

    _layoutMain->activate();
    _layoutHistograms->activate();

    const unsigned int splitterRightSide = _scrollHistogram->width();

    _sizeChartHorizontal = _mainHistogram->size().width();

    if( _flagUpdateChartSize && _mainHistogram->size().width() > 0)
    {
      _sizeMargin = ( splitterRightSide - _sizeChartHorizontal ) / 2;

      _flagUpdateChartSize = false;
    }

    const auto rightMarginFactor = splitterRightSide > _sizeChartHorizontal ? 2 : 1;

    _sizeView = splitterRightSide - _sizeMargin * rightMarginFactor;

    _scaleCurrentHorizontal = static_cast<float>(_sizeChartHorizontal) / _sizeView;

    _spinBoxScaleHorizontal->setValue( _scaleCurrentHorizontal );
  }

  void Summary::wheelEvent( QWheelEvent* event_ )
  {
    constexpr float minScale = 1.0f;

    const bool sign = ( event_->angleDelta().y() > 0 );
    const float adjustment = ( _scaleStep * ( sign ? 1 : ( -1 )));

    if( event_->modifiers().testFlag( Qt::ControlModifier ))
    {
      if( _scaleCurrentHorizontal == minScale )
        _sizeView = _mainHistogram->size().width() - _sizeMargin * 2;

      _scaleCurrentHorizontal = std::max( minScale, _scaleCurrentHorizontal + adjustment );

      _sizeChartHorizontal = ( _scaleCurrentHorizontal * _sizeView );

      _resizeCharts( _sizeChartHorizontal, Qt::Horizontal );
      _resizeEvents( _sizeChartHorizontal );

      _spinBoxScaleHorizontal->setValue( _scaleCurrentHorizontal );
    }
    else if( event_->modifiers().testFlag( Qt::ShiftModifier ))
    {
      _scaleCurrentVertical =
          std::max( minScale, _scaleCurrentVertical + adjustment );

      _sizeChartVertical = ( _scaleCurrentVertical * _sizeChartVerticalDefault);

      _resizeCharts( _sizeChartVertical, Qt::Vertical );

      _spinBoxScaleVertical->setValue( _scaleCurrentVertical );

    }
    else
    {
      if(_scrollHistogram)
      {
        const auto pos = _scrollHistogram->horizontalScrollBar()->value() + (10 * adjustment);

        _scrollHistoLabels->horizontalScrollBar()->setValue( pos );
        _scrollHistogram->horizontalScrollBar()->setValue( pos );
      }

      if(_scrollEvent)
      {
        const auto pos = _scrollEvent->horizontalScrollBar()->value() + (10 * adjustment);

        _eventLabelsScroll->horizontalScrollBar()->setValue( pos );
        _scrollEvent->horizontalScrollBar()->setValue( pos );
      }
    }

    updateRegionBounds();

    event_->ignore();
  }

  void Summary::moveHoriScrollSync( int /*action*/ )
  {
    const auto sbar = qobject_cast<QScrollBar *>(sender());
    if(!sbar) return;

    if( sbar && _syncScrollsHorizontally &&
        (sbar == _scrollEvent->horizontalScrollBar() ||
         sbar == _scrollHistogram->horizontalScrollBar()))
    {
      int newPos = sbar->sliderPosition();
      _scrollEvent->horizontalScrollBar()->setValue( newPos );
      _scrollHistogram->horizontalScrollBar()->setValue( newPos );
    }
  }

  void Summary::moveVertScrollSync( int /*action*/ )
  {
    if( _syncScrollsVertically )
    {
      const auto sbar = qobject_cast<QScrollBar *>(sender());
      if(!sbar) return;

      const auto newPos = sbar->sliderPosition();

      if( sbar == _eventLabelsScroll->verticalScrollBar() ||
          sbar == _scrollEvent->verticalScrollBar())
      {
        _eventLabelsScroll->verticalScrollBar()->setValue( newPos );
        _scrollEvent->verticalScrollBar()->setValue( newPos );
      }
      else if( sbar == _scrollHistoLabels->verticalScrollBar() ||
               sbar == _scrollHistogram->verticalScrollBar() )
      {
        _scrollHistoLabels->verticalScrollBar()->setValue( newPos );
        _scrollHistogram->verticalScrollBar()->setValue( newPos );
      }
    }
  }

  void Summary::syncSplitters()
  {
    if( sender() == _splitHorizEvents )
      _splitHorizHisto->setSizes( _splitHorizEvents->sizes());
    else
      _splitHorizEvents->setSizes( _splitHorizHisto->sizes());

    unsigned int splitterRightSide = _scrollHistogram->size().width();

    unsigned int rightMarginFactor = splitterRightSide > _sizeChartHorizontal ? 2 : 1;

    _sizeView = splitterRightSide - ( _sizeMargin * rightMarginFactor );
    _sizeChartHorizontal =
        _mainHistogram ? _mainHistogram->size().width() : _sizeView * _scaleCurrentHorizontal;

    _scaleCurrentHorizontal = static_cast<float> (_sizeChartHorizontal) / _sizeView;
  }

  void Summary::adjustSplittersSize()
  {
    int lhs = width() * 0.2;
    int rhs = width() - lhs;

    QList< int > sizes;
    sizes << lhs << rhs;

    _splitHorizEvents->setSizes( sizes );
    _splitHorizHisto->setSizes( sizes );
  }

  void Summary::focusPlayback( void )
  {
    setFocusAt( _player->GetRelativeTime() );
  }

  void Summary::setFocusAt( float perc )
  {
    perc = std::max(0.f, std::min(perc, 1.f));
    _regionPercentage = perc;

    if(!_histogramWidgets.empty())
    {
      if(!_focusedHistogram)
      {
        _focusedHistogram = _histogramWidgets.front();
      }

      updateRegionBounds();

      _focusWidget->viewRegion( *_focusedHistogram, _regionPercentage, _regionWidth );
      _focusWidget->update();

      _focusedHistogram->regionWidth( _regionWidth );
      _focusedHistogram->paintRegion(true);
      _focusedHistogram->focusValueAt(perc);
      _focusedHistogram->update();

      auto setPaintRegion = [&](HistogramWidget *w)
      {
        if(w == _focusedHistogram)
        {
          w->regionWidth(_regionWidth);
          w->focusValueAt(perc);
        }
        w->paintRegion(w == _focusedHistogram);
        w->update();
      };
      std::for_each(_histogramWidgets.begin(), _histogramWidgets.end(), setPaintRegion);
    }
  }

  void Summary::changeHistogramName(unsigned int idx, const QString &name)
  {
    if(idx < _histogramRows.size())
    {
      auto row = _histogramRows.at(idx);
      row.label->setText(name);
      row.histogram->name(name.toStdString());
    }
  }

  void Summary::changeHistogramVisibility(unsigned int idx, const bool state)
  {
    if(idx < _histogramRows.size())
    {
      auto row = _histogramRows.at(idx);
      row.label->setVisible(state);

      auto histogram = _histogramWidgets.at(idx);
      histogram->setVisible(state);
    }

  }

}
