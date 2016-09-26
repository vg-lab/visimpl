/*
 * SimulationSummaryWidget.cpp
 *
 *  Created on: 16 de feb. de 2016
 *      Author: sgalindo
 */

#include "Summary.h"

#include <QMouseEvent>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QInputDialog>
#include <QSpinBox>

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
, _mousePressed( false )
, _stackType( stackType )
, _colorScaleLocal( visimpl::T_COLOR_LINEAR )
, _colorScaleGlobal( visimpl::T_COLOR_LOGARITHMIC )
, _colorLocal( 0, 0, 128, 50 )
, _colorGlobal( 255, 0, 0, 100 )
, _heightPerRow( 50 )
, _overRegionEdgeLower( false )
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


    QWidget* header = new QWidget( );
    QGridLayout* headerLayout = new QGridLayout( );
    headerLayout->addWidget( new QLabel( "Name" ), 0, 0, 1, 1);
    headerLayout->addWidget( new QLabel( "Activity" ), 0, 1, 1, _summaryColumns);
    headerLayout->addWidget( new QLabel( "Select" ), 0, _maxColumns - 1, 1, 1);

    header->setLayout( headerLayout );

    _mainLayout = new QGridLayout( );
    _mainLayout->setAlignment( Qt::AlignTop );

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
    QScrollArea* scrollArea = new QScrollArea( );
    scrollArea->setWidget( _body );
    scrollArea->setWidgetResizable( true );

    upperLayout->addWidget( header );
    upperLayout->addWidget( scrollArea );
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

//void Summary::Init( brion::SpikeReport* spikes_, brion::GIDSet gids_ )
//{
//  _spikeReport = spikes_;
//  _gids = GIDUSet( gids_.begin( ), gids_.end( ));
//}

void Summary::Init( simil::SpikeData* spikes_, const simil::TGIDSet& gids_ )
{
  _spikeReport = spikes_;
  _gids = GIDUSet( gids_.begin( ), gids_.end( ));

  Init( );
}

void Summary::Init( void )
{

//
//  _filteredGIDs = gids;
//
//  CreateSummarySpikes( gids );
//  UpdateGradientColors( );
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
  //  _mainHistogram->setMinimumWidth( width( ));
  //  _mainLayout->addWidget( _mainHistogram );
  _focusedHistogram = _mainHistogram;

    TColorMapper colorMapper;
  //  colorMapper.Insert(0.0f, glm::vec4( 0.0f, 0.0f, 255, 255 ));
  //  colorMapper.Insert(0.25f, glm::vec4( 128, 128, 255, 255 ));
  //  colorMapper.Insert(0.33f, glm::vec4( 0.0f, 255, 0.0f, 255 ));
  //  colorMapper.Insert(0.66f, glm::vec4( 255, 255, 0.0f, 255 ));
  //  colorMapper.Insert(1.0f, glm::vec4( 255, 0.0f, 0.0f, 255 ));

    colorMapper.Insert(0.0f, glm::vec4( 157, 206, 111, 255 ));
    colorMapper.Insert(0.25f, glm::vec4( 125, 195, 90, 255 ));
    colorMapper.Insert(0.50f, glm::vec4( 109, 178, 113, 255 ));
    colorMapper.Insert(0.75f, glm::vec4( 76, 165, 86, 255 ));
    colorMapper.Insert(1.0f, glm::vec4( 63, 135, 61, 255 ));

  //  colorMapper.Insert(0.0f, glm::vec4( 85, 0, 0, 255 ));
  //  colorMapper.Insert(0.25f, glm::vec4( 128, 21, 21, 255 ));
  //  colorMapper.Insert(0.50f, glm::vec4( 170, 57, 57, 255 ));
  //  colorMapper.Insert(0.75f, glm::vec4( 212, 106, 106, 255 ));
  //  colorMapper.Insert(1.0f, glm::vec4( 255, 170, 170, 255 ));

    _mainHistogram->colorMapper( colorMapper );

    _mainHistogram->mousePosition( &_lastMousePosition );
    _mainHistogram->regionPosition( &_regionGlobalPosition );
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
    QString labelText = QString( "All");
    mainRow.label = new QLabel( labelText );
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

    }
    else
#endif

    {
      InsertSummary( selection );
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

    InsertSummary( selection );
  }

}

#endif

void Summary::InsertSummary( const visimpl::Selection& selection )
{
  StackRow currentRow;

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

  visimpl::MultiLevelHistogram* histogram = new visimpl::MultiLevelHistogram( *_spikeReport );
  histogram->filteredGIDs( selection.gids );
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
  //    histogram->setMinimumWidth( 500 );

  currentRow.histogram = histogram;
  currentRow.label = new QLabel( labelText );
  currentRow.checkBox = new QCheckBox( );

  unsigned int row = _histograms.size( );
  _mainLayout->addWidget( currentRow.label, row , 0, 1, 1 );
  _mainLayout->addWidget( histogram, row, 1, 1, _summaryColumns );
  _mainLayout->addWidget( currentRow.checkBox, row, _maxColumns - 1, 1, 1 );

  _rows.push_back( currentRow );

  histogram->mousePosition( &_lastMousePosition );
  histogram->regionPosition( &_regionGlobalPosition );

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
  }
  else if( _overRegionEdgeUpper )
  {
    std::cout << "Selected upper edge" << std::endl;
    _selectedEdgeUpper = true;
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
int regionEditMarginPixels = 5;

void Summary::SetFocusRegionPosition( const QPoint& cursorLocalPosition )
{
  if( _stackType == T_STACK_FIXED )
    return;

  _regionLocalPosition = cursorLocalPosition;

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
      }
      else
      {

        _regionGlobalPosition = point;
        _regionWidthPixels = _regionWidth * focusedHistogram->width( );

        SetFocusRegionPosition( cursorLocalPoint );

      }

      _focusedHistogram->regionWidth( _regionWidth );

    } // end mousePressed

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
