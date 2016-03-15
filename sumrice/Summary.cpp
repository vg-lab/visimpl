/*
 * SimulationSummaryWidget.cpp
 *
 *  Created on: 16 de feb. de 2016
 *      Author: sgalindo
 */

#include "Summary.h"

#include <QMouseEvent>
#include <QVBoxLayout>
#include <QScrollArea>

unsigned int visimpl::Selection::_counter = 0;

unsigned int DEFAULT_BINS = 250;

Summary::Summary( QWidget* parent_ )
: QWidget( parent_ )
, _bins( DEFAULT_BINS )
, _spikeReport( nullptr )
, _voltageReport( nullptr )
, _mainHistogram( nullptr )
, _selectionHistogram( nullptr )
, _stackType( T_STACK_FIXED )
, _heightPerRow( 50 )
{ }

Summary::Summary( QWidget* parent_,
                  TStackType stackType )
: QWidget( parent_ )
, _bins( DEFAULT_BINS )
, _spikeReport( nullptr )
, _voltageReport( nullptr )
, _mainHistogram( nullptr )
, _selectionHistogram( nullptr )
, _stackType( stackType )
, _heightPerRow( 50 )
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
  _summaryColumns = _maxColumns - 2;

  QVBoxLayout* upperLayout = new QVBoxLayout( );
  upperLayout->setAlignment( Qt::AlignTop );


  QWidget* header = new QWidget( );
  QGridLayout* headerLayout = new QGridLayout( );
  headerLayout->addWidget( new QLabel( "Name" ), 0, 0, 1, 1);
  headerLayout->addWidget( new QLabel( "Activity" ), 0, 1, 1, _summaryColumns);
  headerLayout->addWidget( new QLabel( "Select" ), 0, _maxColumns - 1, 1, 1);

  QFrame* line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  headerLayout->addWidget( line, 1, 0, 1, _maxColumns );

  header->setLayout( headerLayout );

  _mainLayout = new QGridLayout( );
  _mainLayout->setAlignment( Qt::AlignTop );

  _body = new QWidget( );
  _body->setLayout( _mainLayout );

////  std::cout << width( ) << std::endl;
//  _body->setMinimumWidth( width( ));
////  body->setMaximumWidth( width( ));
  QScrollArea* scrollArea = new QScrollArea( );
  scrollArea->setWidget( _body );
  scrollArea->setWidgetResizable( true );

  upperLayout->addWidget( header );
  upperLayout->addWidget( scrollArea );
//  upperLayout->addWidget( _body );

  this->setLayout( upperLayout );

#ifdef VISIMPL_USE_ZEQ

  _insertionTimer.setSingleShot( false );
  _insertionTimer.setInterval( 250 );
  connect( &_insertionTimer, SIGNAL( timeout( )),
           this, SLOT(deferredInsertion( )));
  _insertionTimer.start( );


#endif
  }
}

void Summary::Init( brion::SpikeReport* spikes_, brion::GIDSet gids )
{
  _spikeReport = spikes_;
  _gids = GIDUSet( gids.begin( ), gids.end( ));
//
//  _filteredGIDs = gids;
//
//  CreateSummarySpikes( gids );
//  UpdateGradientColors( );
  _mainHistogram = new visimpl::Histogram( *spikes_ );
  _mainHistogram->setMinimumHeight( _heightPerRow );
  _mainHistogram->setMaximumHeight( _heightPerRow );
  _mainHistogram->colorScale( visimpl::Histogram::T_COLOR_LOGARITHMIC );
  _mainHistogram->representationMode( visimpl::Histogram::T_REP_CURVE );
  //  _mainHistogram->setMinimumWidth( width( ));
  //  _mainLayout->addWidget( _mainHistogram );

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
    connect( _mainHistogram, SIGNAL( mousePositionChanged( QPoint )),
             this, SLOT( updateMouseMarker( QPoint )));



  if( _stackType == T_STACK_FIXED)
  {
    _mainLayout->addWidget( _mainHistogram, 0, 1, 1, 1 );
    _histograms.push_back( _mainHistogram );
  }
  else if( _stackType == T_STACK_EXPANDABLE )
  {

    StackRow mainRow;

    mainRow.histogram = _mainHistogram;
    QString labelText = QString( "All");
    mainRow.label = new QLabel( labelText );
    mainRow.checkBox = new QCheckBox( );

    unsigned int row = _histograms.size( );
    _mainLayout->addWidget( mainRow.label, row , 0, 1, 1 );
    _mainLayout->addWidget( _mainHistogram, row, 1, 1, _summaryColumns );
    _mainLayout->addWidget( mainRow.checkBox, row, _maxColumns - 1, 1, 1 );
  //  mainRow.checkBox->setVisible( false );

    _rows.push_back( mainRow );
    _histograms.push_back( _mainHistogram );

  }
  //  AddGIDSelection( gids );

  CreateSummarySpikes( );
//  UpdateGradientColors( );
  update( );
}

void Summary::AddNewHistogram( const visimpl::Selection& selection
#ifdef VISIMPL_USE_ZEQ
                       , bool deferred
#endif
                       )
{
  const GIDUSet& gids = selection.gids;

  if( gids.size( ) == _gids.size( ) || gids.size( ) == 0)
    return;


  if( _stackType == TStackType::T_STACK_EXPANDABLE )
  {

    std::cout << "Adding new selection with size " << gids.size( ) << std::endl;
#ifdef VISIMPL_USE_ZEQ

    if( deferred )
    {

      _pendingSelections.push_back( selection );

//      if( !_insertionTimer.isActive( ))
//        _insertionTimer.start( );

    }
    else
#endif

    {
    visimpl::Histogram* histogram = new visimpl::Histogram( *_spikeReport );
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


//    unsigned int newHeight = (_histograms.size( )) * _heightPerRow;
//    _body->setMinimumHeight( newHeight );
//    this->setMinimumHeight( newHeight + 100 );

//    this->parentWidget( )->resize( width( ), newHeight + 2 );
    }

  }

  CreateSummarySpikes( );
  UpdateGradientColors( );

  update( );
}

void Summary::deferredInsertion( void )
{
//  std::cout << "Deferred call " << thread( ) << std::endl;

  if( _pendingSelections.size( ) > 0 )
  {
    _mutex.lock( );
    StackRow currentRow;

    visimpl::Selection selection = _pendingSelections.front( );
    _pendingSelections.pop_front( );

    std::cout << "Deferred insertion: " << selection.name
              << " GIDs: " << selection.gids.size( )
              << std::endl;

    visimpl::Histogram* histogram = new visimpl::Histogram( *_spikeReport );
    histogram->filteredGIDs( selection.gids );
    histogram->colorMapper( _mainHistogram->colorMapper( ));
    histogram->colorScale( visimpl::Histogram::T_COLOR_LOGARITHMIC );
    histogram->normalizeRule( visimpl::Histogram::T_NORM_MAX );
    histogram->representationMode( visimpl::Histogram::T_REP_CURVE );
    histogram->setMinimumHeight( _heightPerRow );
    histogram->setMaximumHeight( _heightPerRow );
    //    histogram->setMinimumWidth( 500 );

    currentRow.histogram = histogram;
    QString labelText =
        QString( "Selection-").append( QString::number( selection.id ));
    currentRow.label = new QLabel( labelText );
    currentRow.checkBox = new QCheckBox( );

    unsigned int row = _histograms.size( );
    _mainLayout->addWidget( currentRow.label, row , 0, 1, 1 );
    _mainLayout->addWidget( histogram, row, 1, 1, _summaryColumns );
    _mainLayout->addWidget( currentRow.checkBox, row, _maxColumns - 1, 1, 1 );

    _rows.push_back( currentRow );

    histogram->mousePosition( &_lastMousePosition );
    connect( histogram, SIGNAL( mousePositionChanged( QPoint )),
             this, SLOT( updateMouseMarker( QPoint )));

    _histograms.push_back( histogram );

    CreateSummarySpikes( );
//    UpdateGradientColors( );

    GIDUSet tmp;
    histogram->filteredGIDs( tmp );

    _mutex.unlock( );
    update( );
  }

}

void Summary::mouseMoveEvent( QMouseEvent* event_ )
{
  QWidget::mouseMoveEvent( event_ );

  QPoint position = event_->pos( );

  _lastMousePosition = mapToGlobal( position );
  _showMarker = true;

  for( auto histogram : _histograms )
    histogram->update( );


}

void Summary::updateMouseMarker( QPoint point )
{
  if( point != _lastMousePosition )
  {
    _lastMousePosition = point;

    for( auto histogram : _histograms )
      histogram->update( );
  }
}

void Summary::CreateSummarySpikes( )
{
  std::cout << "Creating histograms... Number of bins: " << _bins << std::endl;
  for( auto histogram : _histograms )
  {
    if( !histogram->isInitialized( ) )
      histogram->CreateHistogram( _bins );
  }

//  _mainHistogram->CreateHistogram( _bins );
//
//  _selectionHistogram->CreateHistogram( _bins );


}

//void Summary::CreateSummaryVoltages( void )
//{
//
//}

void Summary::UpdateGradientColors( void )
{
  for( auto histogram : _histograms )
  {
    if( !histogram->isInitialized( ))
      histogram->CalculateColors( );
  }

//  _mainHistogram->CalculateColors( );
//
//  _selectionHistogram->CalculateColors( );

}

unsigned int Summary::histogramsNumber( void )
{
  return _histograms.size( );
}


void Summary::bins( unsigned int bins_ )
{
  _bins = bins_;

  CreateSummarySpikes( );
  UpdateGradientColors( );

  update( );
}

unsigned int Summary::bins( void )
{
  return _bins;
}

void Summary::heightPerRow( unsigned int height_ )
{
  _heightPerRow = height_;
}

unsigned int Summary::heightPerRow( void )
{
  return _heightPerRow;
}

void Summary::showMarker( bool show_ )
{
  _showMarker = show_;
}
