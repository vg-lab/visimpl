#include "OpenGLWidget.h"
#include <QOpenGLContext>
#include <QMouseEvent>
#include <QColorDialog>
#include <QShortcut>

#include <sstream>
#include <string>
#include <iostream>
#include <glm/glm.hpp>

#include <map>

#include "MainWindow.h"

#include <nlrender/Config.h>


#include "prefr/ColorEmissionNode.h"
#include "prefr/CompositeColorEmitter.h"
#include "prefr/CompositeColorUpdater.h"

#include "prefr/DirectValuedEmissionNode.h"
#include "prefr/DirectValuedEmitter.h"
#include "prefr/DirectValuedUpdater.h"

using namespace visimpl;

OpenGLWidget::OpenGLWidget( QWidget* parent_,
                            Qt::WindowFlags windowsFlags_,
                            bool paintNeurons_,
                            const std::string&
#ifdef VISIMPL_USE_ZEQ
                            zeqUri
#endif
  )
  : QOpenGLWidget( parent_, windowsFlags_ )
#ifdef VISIMPL_USE_ZEQ
  , _zeqUri( zeqUri )
#endif
  , _fpsLabel( this )
  , _showFps( false )
  , _wireframe( false )
  , _focusOnSelection( paintNeurons_ )
  , _frameCount( 0 )
  , _mouseX( 0 )
  , _mouseY( 0 )
  , _rotation( false )
  , _translation( false )
  , _idleUpdate( true )
  , _paint( false )
  , _currentClearColor( 20, 20, 20, 0 )
  , _particlesShader( nullptr )
  , _ps( nullptr )
  , _simulationType( TSimulationType::TSimNetwork )
  , _player( nullptr )
  , _firstFrame( true )
  , _elapsedTimeRenderAcc( 0.0f )
  , _elapsedTimeSliderAcc( 0.0f )
  , _elapsedTimeSimAcc( 0.0f )

{
#ifdef VISIMPL_USE_ZEQ
  if ( zeqUri != "" )
    _camera = new nlrender::Camera( zeqUri );
  else
#endif
  _camera = new nlrender::Camera( );

  _lastCameraPosition = glm::vec3( 0, 0, 0 );

  _fpsLabel.setStyleSheet(
    "QLabel { background-color : #333;"
    "color : white;"
    "padding: 3px;"
    "margin: 10px;"
    " border-radius: 10px;}" );

  // This is needed to get key evends
  this->setFocusPolicy( Qt::WheelFocus );

  _maxFPS = 30.0f;
  _renderPeriod = 1.0f / _maxFPS;

//  _playbackSpeed = 5.0f;
  _playbackSpeed = 1.f;
  _renderSpeed = 1.f;

  new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Minus ),
                 this, SLOT( reducePlaybackSpeed( ) ));

  new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Plus ),
                   this, SLOT( increasePlaybackSpeed( ) ));
}


OpenGLWidget::~OpenGLWidget( void )
{
  delete _camera;

  if( _particlesShader )
    delete _particlesShader;

  if( _ps )
    delete _ps;

  if( _player )
    delete _player;

}




void OpenGLWidget::loadData( const std::string& fileName,
                             const visimpl::TDataType fileType,
                             TSimulationType simulationType,
                             const std::string& report)
{

  makeCurrent( );

  _simulationType = simulationType;

  switch( _simulationType )
  {
    case TSimSpikes:
    {
      _deltaTime = 0.5f;
//        _player = new SpikesPlayer( fileName, true );
      SpikesPlayer* spPlayer = new SpikesPlayer( );
      spPlayer->LoadData( fileType, fileName, report );
      _player = spPlayer;
      _player->deltaTime( _deltaTime );

      break;
    }
    case TSimVoltages:
    {
      _player = new VoltagesPlayer( fileName, report, true);
      _deltaTime = _player->deltaTime( );
      break;
    }
    default:
      VISIMPL_THROW("Cannot load an undefined simulation type.");

  }

  float scale = 1.0f;
  if( fileType == visimpl::THDF5 )
  {
    scale = 500.f;
  }

  createParticleSystem( scale );
  _player->connectZeq( _zeqUri );

  switch( fileType )
  {
    case visimpl::TBlueConfig:
      _playbackSpeed = 5.0f;
      changeSimulationDecayValue( 5.0f );
      break;
    case visimpl::THDF5:
      _playbackSpeed = 0.05f;
      changeSimulationDecayValue( 1.0f );
      break;

    default:
      break;
  }

  this->_paint = true;
  update( );

}


void OpenGLWidget::initializeGL( void )
{
  initializeOpenGLFunctions( );

  glEnable( GL_DEPTH_TEST );
  glClearColor( float( _currentClearColor.red( )) / 255.0f,
                float( _currentClearColor.green( )) / 255.0f,
                float( _currentClearColor.blue( )) / 255.0f,
                float( _currentClearColor.alpha( )) / 255.0f );
  glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
  glEnable( GL_CULL_FACE );

  glLineWidth( 1.5 );

  _then = std::chrono::system_clock::now( );
  _lastFrame = std::chrono::system_clock::now( );


  QOpenGLWidget::initializeGL( );

}

void OpenGLWidget::configureSimulation( void )
{
  if( !_player || !_player->isPlaying( ))
    return;

  _player->Frame( );

  switch( _simulationType )
  {
    case TSimSpikes:
    {
      SpikesCRange spikes =
          dynamic_cast< visimpl::SpikesPlayer* >( _player )->spikesNow( );

      unsigned int count = 0;
      for( SpikesCIter spike = spikes.first; spike != spikes.second; spike++)
      {
        if( _selectedGIDs.find( ( *spike ).second ) != _selectedGIDs.end( )
            || _selectedGIDs.size( ) == 0 || !_showSelection )
        {
          auto res = gidNodesMap.find( (*spike).second );
          if( res != gidNodesMap.end( ))
          {
            dynamic_cast< prefr::ColorEmissionNode* >(( *res ).second )->
                killParticles( true  );
          }
//          else
//          {
//            std::cout << "Spike not found for " << spike->second << std::endl;
//          }
          count += 1;
        }
//        else
//        {
//          std::cout << "Spike not found for " << spike->second << std::endl;
//        }
      }

//      std::cout << "Fired " << count << " spikes." << std::endl;
      break;
    }
    case TSimVoltages:
    {
      visimpl::VoltagesPlayer* vplayer =
          dynamic_cast< visimpl::VoltagesPlayer* >(_player);
//      VoltCIter begin = vplayer->begin( );
//      VoltCIter end = vplayer->end( );

      float normFactor = vplayer->getNormVoltageFactor( );

//      std::cout << "Current time: " << vplayer->currentTime() << std::endl;

//      brion::GIDSetCIter gid = _player->gids( ).begin( );
      unsigned int counter = 0;
      for( auto gid : vplayer->gids( ))
      {
        auto res = gidNodesMap.find( gid );
        if( res == gidNodesMap.end( ))
        {
          std::cerr << "Err: Node not found " << gid << std::endl;
          continue;
        }

        float volt = vplayer->getVoltage( gid );
        float relVal = volt + std::abs( vplayer->minVoltage( ));
        relVal *= normFactor;

//        std::cout << "GID: " << gid
//                  << " voltage: " << volt
//                  << " -> value: " << relVal
//                  << " num: " << counter
//                  << std::endl;

        dynamic_cast< prefr::DirectValuedEmissionNode* >( res->second )->
            particlesLife( relVal );

//        gid++;

        counter++;
      }

      break;
    }
    default:
      break;
  }

}

void OpenGLWidget::createNeuronsCollection( void )
{
  makeCurrent( );
  nlrender::Config::init( );
//  _neuronsCollection = new neurolots::NeuronsCollection( _camera );
}

void OpenGLWidget::createParticleSystem( float scale )
{
  makeCurrent( );
  prefr::Config::init( );

  unsigned int maxParticles = _player->gids( ).size( );
  unsigned int maxEmitters = maxParticles;

  _ps = new prefr::ParticleSystem( maxParticles, maxParticles, true );
  _ps->renderDeadParticles = true;

  const brion::Vector3fs& positions = _player->positions( );

  std::string prefrShadersPath;

  if ( getenv( "PREFR_SHADERS_PATH" ) == nullptr )
  {
    std::cerr << "Environment Variable PREFR_SHADERS_PATH not defined"
              << std::endl;
    exit(-1);
  }
  else
    prefrShadersPath = std::string( getenv( "PREFR_SHADERS_PATH" ));

  std::string vertPath, fragPath;
  fragPath = vertPath = std::string( prefrShadersPath );
  vertPath.append( "/GL-vert.glsl" );
  fragPath.append( "/GL-frag.glsl" );
  _particlesShader = new CShader( false, false,
                                 vertPath.c_str( ) ,
                                 fragPath.c_str( ));

  std::cout << "Loading shaders: " << std::endl;
  std::cout << "- Vertex: " << vertPath.c_str( ) << std::endl;
  std::cout << "- Fragments: " << fragPath.c_str( ) << std::endl;

  _maxLife = 20.0f;

//  prefr::ColorOperationPrototype* prototype =

  _offPrototype =
      new prefr::ColorOperationPrototype(
        _maxLife, _maxLife,
        prefr::ParticleCollection( _ps->particles, 0, maxParticles));

  _offPrototype->color.Insert( 0.0f, ( glm::vec4(0.1f, 0.1f, 0.1f, 0.5f)));

  _offPrototype->velocity.Insert( 0.0f, 0.0f );

//  _offPrototype->size.Insert( 0.0f, 30.0f );
  _offPrototype->size.Insert( 1.0f, 10.0f );

  _ps->AddPrototype( _offPrototype );


  _prototype =
    new prefr::ColorOperationPrototype(
      _maxLife, _maxLife,
      prefr::ParticleCollection( _ps->particles, 0, maxParticles));

//  prototype->color.Insert( 0.0f, ( glm::vec4(0.2, 0.2, 0.2, 0.05)));
//  prototype->color.Insert( 0.1f, ( glm::vec4(0, 1, 1, 0.5 )));
//  prototype->color.Insert( 1.0f, ( glm::vec4(0.2, 0.2, 0.2, 0.05 )));

  _ps->AddPrototype( _prototype );


  prefr::EmissionNode* emissionNode;

  int partPerEmitter = 1;

  std::cout << "Creating " << maxEmitters << " emitters with "
            << partPerEmitter << std::endl;

  unsigned int start;
  unsigned int end;

  unsigned int i = 0;
  glm::vec3 cameraPivot;
  brion::GIDSetCIter gid = _player->gids( ).begin();
  for ( auto brionPos : positions )
  {

//    nsol::Matrix4_4f m = it.second->transform( );
//    vmml::Vector3f t = it.second->morphology( )->soma( )->center( );
//    glm::vec3 position ( t.x( ), t.y( ), t.z( ));
//    glm::vec3 position ( m.at( 0, 3 ), m.at( 1, 3 ), m.at( 2, 3 ));

//    std::cout << position.x << ", "
//              << position.y << ", "
//              << position.z << std::endl;



    glm::vec3 position( brionPos.x( ), brionPos.y( ), brionPos.z( ));

    if( scale != 1.0f )
      position *= scale;

    cameraPivot += position;

    start = i * partPerEmitter;
    end = start + partPerEmitter;

    prefr::ParticleCollection collection ( _ps->particles, start, end );

    switch( _simulationType )
    {
    case TSimSpikes:
      emissionNode = new prefr::ColorEmissionNode( collection, position,
                                                   glm::vec4( 0, 0, 0, 0 ),
                                                   true );
      break;
    case TSimVoltages:
      emissionNode = new prefr::DirectValuedEmissionNode(
          collection, position, glm::vec4( 0, 0, 0, 0 ), true );

      break;
    default:
      emissionNode = nullptr;
      VISIMPL_THROW("Node type undefined");
      break;
    }

    _ps->AddEmissionNode( emissionNode );
//    emissionNode->active = false;
    emissionNode->killParticlesIfInactive = true;
    emissionNode->maxEmissionCycles = 1;

    gidNodesMap.insert( std::make_pair(( *gid ), emissionNode ));
    nodesGIDMap.insert( std::make_pair( emissionNode, ( *gid )));

    i++;
    gid++;
  }

  cameraPivot /= i;

  _camera->Pivot( Eigen::Vector3f( cameraPivot.x,
                                         cameraPivot.y,
                                         cameraPivot.z ));

  prefr::ParticleEmitter* emitter;
  prefr::ParticleUpdater* updater;

  switch( _simulationType )
  {
    case TSimSpikes:
      emitter = new prefr::CompositeColorEmitter( *_ps->particles, 1.f, true );
      std::cout << "Created Spikes Emitter" << std::endl;
      updater = new prefr::CompositeColorUpdater( *_ps->particles );
      std::cout << "Created Spikes Updater" << std::endl;

//      _prototype->color.Insert( 0.0f, ( glm::vec4(0.1, 0.1, 0.3, 0.05)));
//      _prototype->color.Insert( 0.1f, ( glm::vec4(1, 0, 0, 0.2 )));
//      _prototype->color.Insert( 0.7f, ( glm::vec4(1, 0.5, 0, 0.2 )));
//      _prototype->color.Insert( 1.0f, ( glm::vec4(0, 0, 0.3, 0.05 )));

      _prototype->color.Insert( 0.0f, ( glm::vec4(0.f, 1.f, 0.f, 0.05)));
      _prototype->color.Insert( 0.35f, ( glm::vec4(1, 0, 0, 0.2 )));
      _prototype->color.Insert( 0.7f, ( glm::vec4(1.f, 1.f, 0, 0.2 )));
      _prototype->color.Insert( 1.0f, ( glm::vec4(0, 0, 1.f, 0.2 )));

      _prototype->velocity.Insert( 0.0f, 0.0f );

      _prototype->size.Insert( 0.0f, 20.0f );
      _prototype->size.Insert( 1.0f, 10.0f );

      break;
    case TSimVoltages:

      emitter = new prefr::DirectValuedEmitter( *_ps->particles, 1000.f, true );
      std::cout << "Created Voltages Emitter" << std::endl;
      updater = new prefr::DirectValuedUpdater( *_ps->particles );
      std::cout << "Created Voltages Updater" << std::endl;

      _prototype->color.Insert( 0.0f, ( glm::vec4(0.1, 0.1, 0.3, 0.05)));
      _prototype->color.Insert( 0.25f, ( glm::vec4(0.2, 0.2, 0.3, 0.07 )));
      _prototype->color.Insert( 0.5f, ( glm::vec4(0, 0.5, 0, 0.10 )));
      _prototype->color.Insert( 0.75f, ( glm::vec4(0.3, 0.3, 0.1, 0.15 )));
      _prototype->color.Insert( 1.0f, ( glm::vec4(1, 0.1, 0.1, 0.2 )));

      _prototype->velocity.Insert( 0.0f, 0.0f );

      _prototype->size.Insert( 0.0f, 5.0f );
      _prototype->size.Insert( 1.0f, 20.0f );


      break;
    default:
      VISIMPL_THROW("Simulation type undefined.");
      break;
  }



  prefr::ParticleSorter* sorter;

  #if (PREFR_USE_CUDA)
  std::cout << "CUDA sorter" << std::endl;
  sorter = new prefr::ThrustParticleSorter( *_ps->particles );
  #else
  sorter = new prefr::ParticleSorter( *_ps->particles );
  #endif

  std::cout << "Created sorter" << std::endl;

  prefr::GLDefaultParticleRenderer* renderer =
    new prefr::GLDefaultParticleRenderer( *_ps->particles );

  std::cout << "Created systems" << std::endl;

  _ps->AddEmitter( emitter );
  _ps->AddUpdater( updater );
  _ps->SetSorter( sorter );
  _ps->SetRenderer( renderer );

  _ps->Start();

  resetParticles( );

}

void OpenGLWidget::resetParticles( void )
{
  for( auto node = _ps->emissionNodes->begin( );
         node != _ps->emissionNodes->end( );
         node++ )
  {
    switch( _simulationType )
    {
      case TSimSpikes:
        dynamic_cast< prefr::ColorEmissionNode* >( *node )->killParticles( false );
      break;
      case TSimVoltages:
        dynamic_cast< prefr::DirectValuedEmissionNode* >( *node )->killParticles( false );
      break;
      default:
      break;
    }
    (*node)->Restart( );
  }
  _ps->UpdateUnified( 0.0f  );
}

void OpenGLWidget::SetAlphaBlendingAccumulative( bool accumulative )
{
  _alphaBlendingAccumulative = accumulative;
}



void OpenGLWidget::changeSimulationColorMapping( const TTransferFunction& colors )
{

  if( _ps )
  {

    prefr::vectortvec4 gcolors;

    for( auto c : colors )
    {
      glm::vec4 gColor( c.second.red( ) / 255.0f,
                        c.second.green( ) / 255.0f,
                        c.second.blue( ) / 255.0f,
                        c.second.alpha( ) / 255.0f );
      gcolors.Insert( c.first, gColor );
    }

    _prototype->color = gcolors;

    _ps->UpdateUnified( 0.0f );
  }
}

TTransferFunction OpenGLWidget::getSimulationColorMapping( void )
{
  TTransferFunction result;

  if( _ps )
  {
    prefr::vectortvec4 colors = _prototype->color;

    for( unsigned int i = 0; i < colors.size; i++ )
    {
      glm::vec4 c = colors.values[ i ];
      QColor color( c.r * 255, c.g * 255, c.b * 255, c.a * 255 );
      result.push_back( std::make_pair( colors.times[ i ], color ));
    }
  }

  return result;
}

void OpenGLWidget::changeSimulationSizeFunction( const TSizeFunction& sizes )
{
  if( _ps )
  {
    utils::InterpolationSet< float > newSize;
    for( auto s : sizes )
    {
      std::cout << s.first << " " << s.second << std::endl;
      newSize.Insert( s.first, s.second );
    }
    _prototype->size = newSize;

    _ps->UpdateUnified( 0.0f );
  }
}

TSizeFunction OpenGLWidget::getSimulationSizeFunction( void )
{
  TSizeFunction result;
  if( _ps )
  {
    auto v = _prototype->size.values.begin( );
    for( auto s : _prototype->size.times)
    {
      result.push_back( std::make_pair( s, *v ));
      v++;
    }
  }
  return result;
}

void OpenGLWidget::changeSimulationDecayValue( float value )
{
  _prototype->SetLife( value, value );
}

float OpenGLWidget::getSimulationDecayValue( void )
{
  return _prototype->maxLife;
}

#ifdef VISIMPL_USE_ZEQ

void OpenGLWidget::setSelectedGIDs( const std::unordered_set< uint32_t >& gids )
{
  if( gids.size( ) > 0 )
    _selectedGIDs = gids;

  std::cout << "Received " << _selectedGIDs.size( ) << " ids" << std::endl;

  updateSelection( );

}


void ExpandBoundingBox( Eigen::Vector3f& minBounds,
                        Eigen::Vector3f& maxBounds,
                        glm::vec3 position)
{
  for( unsigned int i = 0; i < 3; ++i )
  {
    minBounds( i ) = std::min( minBounds( i ), position[ i ] );
    maxBounds( i ) = std::max( maxBounds( i ), position[ i ] );
  }
}

void OpenGLWidget::updateSelection( void )
{
  if( _ps )
  {
    _ps->Run( false );

    bool baseOn = !_showSelection || _selectedGIDs.size( ) == 0;

    Eigen::Vector3f boundingBoxMin( std::numeric_limits< float >::max( ),
                                    std::numeric_limits< float >::max( ),
                                    std::numeric_limits< float >::max( ));
    Eigen::Vector3f boundingBoxMax( std::numeric_limits< float >::min( ),
                                    std::numeric_limits< float >::min( ),
                                    std::numeric_limits< float >::min( ));

    for( auto node : *_ps->emissionNodes )
    {
      auto it = nodesGIDMap.find( node );
      unsigned int id = it->second;

//      auto res = _selectedGIDs.find( id );

      bool on =  baseOn || _selectedGIDs.find( id ) != _selectedGIDs.end( );

      prefr::tparticle_ptr particle = *( node->particles->start );

      _ps->particlePrototype[ particle->id ] =
                ( unsigned int )( on ? PROTOTYPE_ON : PROTOTYPE_OFF);

      if( ( _focusOnSelection && on ) || !_focusOnSelection )
        ExpandBoundingBox( boundingBoxMin, boundingBoxMax, particle->position );

    }

    Eigen::Vector3f center = ( boundingBoxMax + boundingBoxMin ) * 0.5f;
    float radius = ( boundingBoxMax - center ).norm( );

    _camera->TargetPivotRadius( center, radius );

    _ps->Run( true );
    _ps->UpdateUnified( 0.0f  );

  }
}

void OpenGLWidget::showSelection( bool showSelection_ )
{

  _showSelection = showSelection_;

  updateSelection( );

}

void OpenGLWidget::ClearSelection( void )
{
  _selectedGIDs.clear( );
  updateSelection( );
}

#endif


void OpenGLWidget::updateSimulation( void )
{
  if( _player->isPlaying( ) || _firstFrame )
  {

    _ps->UpdateUnified( _elapsedTimeRenderAcc );
    _firstFrame = false;
  }
}

void OpenGLWidget::paintParticles( void )
{
  if( !_ps )
    return;

  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  if( _alphaBlendingAccumulative )
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
  else
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  glFrontFace(GL_CCW);

  _particlesShader->activate();
      // unsigned int shader;
      // shader = _particlesShader->getID();
  unsigned int shader;
  shader = _particlesShader->getID();

  unsigned int uModelViewProjM, cameraUp, cameraRight;

  uModelViewProjM = glGetUniformLocation( shader, "modelViewProjM" );
  glUniformMatrix4fv( uModelViewProjM, 1, GL_FALSE,
                     _camera->ViewProjectionMatrix( ));

  cameraUp = glGetUniformLocation( shader, "cameraUp" );
  cameraRight = glGetUniformLocation( shader, "cameraRight" );

  float* viewM = _camera->ViewMatrix( );

  glUniform3f( cameraUp, viewM[1], viewM[5], viewM[9] );
  glUniform3f( cameraRight, viewM[0], viewM[4], viewM[8] );

  glm::vec3 cameraPosition ( _camera->Position( )[ 0 ],
                             _camera->Position( )[ 1 ],
                             _camera->Position( )[ 2 ] );

//  bool cameraMoved = ( _lastCameraPosition == cameraPosition );

//  if( cameraMoved )
  {

//    std::cout << cameraPosition.x << ", "
//                << cameraPosition.y << ", "
//                << cameraPosition.z << std::endl;
//    std::cout << "Camera moved..." << std::endl;
    _ps->UpdateCameraDistances( cameraPosition );
    

    _lastCameraPosition = cameraPosition;
  }

  _ps->UpdateRender( );
  _ps->Render( );

}

//#ifdef VISIMPL_USE_ZEQ
//void OpenGLWidget::_setZeqUri( const std::string&
//                                   uri_
//  )
//{
//  _zeqConnection = true;
//  _uri =  servus::URI( uri_ );
//  _subscriber = new zeq::Subscriber( _uri );
//
//  _subscriber->registerHandler( zeq::hbp::EVENT_SELECTEDIDS,
//      boost::bind( &OpenGLWidget::_onSelectionEvent , this, _1 ));
//
//  pthread_create( &_subscriberThread, NULL, _Subscriber, _subscriber );
//
//}
//
//void* OpenGLWidget::_Subscriber( void* subs )
//{
//  std::cout << "------------Waiting Selection Events..." << std::endl;
//  zeq::Subscriber* subscriber = static_cast< zeq::Subscriber* >( subs );
//  while ( true )
//  {
//    subscriber->receive( 10000 );
//  }
//  pthread_exit( NULL );
//}
//
//void OpenGLWidget::_onSelectionEvent( const zeq::Event& event_ )
//{
//
//  std::vector< unsigned int > selected =
//      zeq::hbp::deserializeSelectedIDs( event_ );
//
//  std::set< unsigned int > selectedSet( selected.begin( ), selected.end( ));
//  std::cout << "Received " << selected.size( ) << " ids" << std::endl;
//
//  _ps->Run( false );
//
//  for( auto node : *_ps->emissionNodes )
//  {
//    auto it = nodesGIDMap.find( node );
//    unsigned int id = it->second;
//
//    auto res = selectedSet.find( id );
//
//    if( res != selectedSet.end( ))
//      node->active = true;
//    else
//    {
//      node->active = false;
//
////      switch( _simulationType )
////      {
////        case TSpikes:
////          dynamic_cast< prefr::ColorEmissionNode* >( node )->killParticles( true );
////        break;
////        case TVoltages:
////          dynamic_cast< prefr::DirectValuedEmissionNode* >( node )->killParticles( true );
////        break;
////        default:
////        break;
////      }
//    }
//  }
//
////  for( auto id : selected )
////  {
////    auto it = gidNodesMap.find( id );
////    if( it != gidNodesMap.end( ))
////    {
////      it->second->active = true;
////    }
////  }
//
//  _ps->Run( true );
//  _ps->UpdateUnified( 0.0f  );
//
//}
//
//#endif

void OpenGLWidget::paintGL( void )
{
  std::chrono::time_point< std::chrono::system_clock > now =
      std::chrono::system_clock::now( );

  unsigned int elapsedMilliseconds =
      std::chrono::duration_cast< std::chrono::milliseconds >
        ( now - _lastFrame ).count( );

  _lastFrame = now;

  _deltaTime = elapsedMilliseconds * 0.001f;
//  std::cout << elapsedMilliseconds << std::endl;

  _elapsedTimeRenderAcc += _deltaTime;
  _elapsedTimeSliderAcc += _deltaTime;
//  _elapsedTimeSimAcc += _deltaTime;


  _frameCount++;
  glDepthMask(GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  if ( _paint )
  {
    _camera->Anim( );

//    if ( _neuronsCollection && _focusOnSelection )
//      _neuronsCollection->Paint( );

    if ( _ps )
    {

      if( _elapsedTimeRenderAcc >= _renderPeriod )
      {
        _elapsedTimeSimAcc = _elapsedTimeRenderAcc * _playbackSpeed;

        _player->deltaTime( _elapsedTimeSimAcc );
//        std::cout << _elapsedTimeRenderAcc << std::endl;

        configureSimulation( );
        updateSimulation( );
        _elapsedTimeRenderAcc = 0.0f;
      }

      paintParticles( );

    }
    glUseProgram( 0 );
    glFlush( );

  }

   if( _player && _elapsedTimeSliderAcc > SIM_SLIDER_UPDATE_PERIOD )
   {
     _elapsedTimeSliderAcc = 0.0f;

#ifdef VISIMPL_USE_ZEQ
     if( _zeqUri != "" )
     {
       _player->sendCurrentTimestamp( );
     }
#endif

     emit updateSlider( _player->GetRelativeTime( ));
   }


  #define FRAMES_PAINTED_TO_MEASURE_FPS 10
  if ( _frameCount == FRAMES_PAINTED_TO_MEASURE_FPS )
  {

    auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>( now - _then );
    _then = now;

    MainWindow* mainWindow = dynamic_cast< MainWindow* >( parent( ));
    if ( mainWindow )
    {

      unsigned int ellapsedMiliseconds = duration.count( );

      unsigned int fps = roundf( 1000.0f *
                                 float( FRAMES_PAINTED_TO_MEASURE_FPS ) /
                                 float( ellapsedMiliseconds ));

      // mainWindow->showStatusBarMessage(
      //   QString::number( fps ) + QString( " FPS" ));
      if ( _showFps )
      {
        _fpsLabel.setVisible( true );
        _fpsLabel.setText( QString::number( fps ) + QString( " FPS" ));
        _fpsLabel.adjustSize( );
      }
      else
        _fpsLabel.setVisible( false );
    }

    _frameCount = 0;
  }

  if ( _idleUpdate )
  {
    // std::cout << _frameCount << std::endl;
    update( );
  }
  else
  {
    _fpsLabel.setVisible( false );
  }

}

void OpenGLWidget::resizeGL( int w , int h )
{
  _camera->Ratio((( double ) w ) / h );
  glViewport( 0, 0, w, h );


}


void OpenGLWidget::mousePressEvent( QMouseEvent* event_ )
{

  if ( event_->button( ) == Qt::LeftButton )
  {
    _rotation = true;
    _mouseX = event_->x( );
    _mouseY = event_->y( );
  }

  if ( event_->button( ) ==  Qt::MidButton )
  {
    _translation = true;
    _mouseX = event_->x( );
    _mouseY = event_->y( );
  }

  update( );

}

void OpenGLWidget::mouseReleaseEvent( QMouseEvent* event_ )
{
  if ( event_->button( ) == Qt::LeftButton)
  {
    _rotation = false;
  }

  if ( event_->button( ) ==  Qt::MidButton )
  {
    _translation = false;
  }

  update( );

}

void OpenGLWidget::mouseMoveEvent( QMouseEvent* event_ )
{
  if( _rotation )
  {
    _camera->LocalRotation( -( _mouseX - event_->x( )) * 0.01,
                          ( _mouseY - event_->y( )) * 0.01 );
    _mouseX = event_->x( );
    _mouseY = event_->y( );
  }
  if( _translation )
  {
    _mouseX = event_->x( );
    _mouseY = event_->y( );
  }

  this->update( );
}


void OpenGLWidget::wheelEvent( QWheelEvent* event_ )
{

  int delta = event_->angleDelta( ).y( );

  if ( delta > 0 )
    _camera->Radius( _camera->Radius( ) / 1.1f );
  else
    _camera->Radius( _camera->Radius( ) * 1.1f );

  update( );

}



void OpenGLWidget::keyPressEvent( QKeyEvent* event_ )
{
  makeCurrent( );

  switch ( event_->key( ))
  {
  case Qt::Key_C:
    _camera->Pivot( Eigen::Vector3f( 0.0f, 0.0f, 0.0f ));
    _camera->Radius( 1000.0f );
    _camera->Rotation( 0.0f, 0.0f );
    update( );
    break;

//  case Qt::Key_W:
//    _neuronsCollection->AddLod( 1.0f );
//    update( );
//    break;
//
//  case Qt::Key_S:
//    _neuronsCollection->AddLod( -1.0f );
//    update( );
//    break;
//
//  case Qt::Key_E:
//    _neuronsCollection->AddTng( 0.1f );
//    update( );
//    break;
//
//  case Qt::Key_D:
//    _neuronsCollection->AddTng( -0.1f );
//    update( );
//    break;
//
//  case Qt::Key_R:
//    _neuronsCollection->AddMaxDist( 1 );
//    update( );
//    break;
//
//  case Qt::Key_F:
//    _neuronsCollection->AddMaxDist( -1 );
//    update( );
//    break;
  }
}


void OpenGLWidget::changeClearColor( void )
{
  QColor color =
    QColorDialog::getColor( _currentClearColor, parentWidget( ),
                            "Choose new background color",
                            QColorDialog::DontUseNativeDialog);

  if ( color.isValid( ))
  {
    _currentClearColor = color;

    makeCurrent( );
    glClearColor( float( _currentClearColor.red( )) / 255.0f,
                  float( _currentClearColor.green( )) / 255.0f,
                  float( _currentClearColor.blue( )) / 255.0f,
                  float( _currentClearColor.alpha( )) / 255.0f );
    update( );
  }
}


void OpenGLWidget::toggleUpdateOnIdle( void )
{
  _idleUpdate = !_idleUpdate;
  if ( _idleUpdate )
    update( );
}

void OpenGLWidget::toggleShowFPS( void )
{
  _showFps = !_showFps;
  if ( _idleUpdate )
    update( );
}

void OpenGLWidget::toggleWireframe( void )
{
  makeCurrent( );
  _wireframe = !_wireframe;

  if ( _wireframe )
  {
    glEnable( GL_POLYGON_OFFSET_LINE );
    glPolygonOffset( -1, -1 );
    glLineWidth( 1.5 );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  }
  else
  {
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDisable( GL_POLYGON_OFFSET_LINE );
  }

  update( );
}

void OpenGLWidget::togglePaintNeurons( void )
{
  _focusOnSelection = !_focusOnSelection;
  update( );
}


visimpl::SimulationPlayer* OpenGLWidget::player( )
{
  return _player;
}

void OpenGLWidget::Play( void )
{
  if( _player )
  {
    _player->Play( );
  }
}

void OpenGLWidget::Pause( void )
{
  if( _player )
  {
    _player->Pause( );
  }
}

void OpenGLWidget::PlayPause( void )
{

  if( _player )
  {
    if( !_player->isPlaying( ))
      _player->Play( );
    else
      _player->Pause( );
  }
}

void OpenGLWidget::Stop( void )
{
  if( _player )
  {
    _player->Stop( );
    resetParticles( );
    _firstFrame = true;
  }
}

void OpenGLWidget::Repeat( bool repeat )
{
  if( _player )
  {
    _player->loop( repeat );
  }
}

void OpenGLWidget::PlayAt( float percentage )
{
  if( _player )
  {
    _ps->Run( false );
    std::cout << "Play at " << percentage << std::endl;
    _player->PlayAt( percentage );
    _ps->Run( true );
  }
}

void OpenGLWidget::Restart( void )
{
  if( _player )
  {
    bool playing = _player->isPlaying( );
    _player->Stop( );
    if( playing )
      _player->Play( );
    resetParticles( );
    _firstFrame = true;
  }
}

void OpenGLWidget::GoToEnd( void )
{

}


void OpenGLWidget::reducePlaybackSpeed( )
{
  _playbackSpeed -= 0.01f;
  if( _playbackSpeed < 0.01f )
    _playbackSpeed = 0.01f;

  std::cout << "Playback speed: x" << _playbackSpeed << std::endl;
}

void OpenGLWidget::increasePlaybackSpeed( )
{

  _playbackSpeed += 0.01f;

  std::cout << "Playback speed: x" << _playbackSpeed << std::endl;
}
