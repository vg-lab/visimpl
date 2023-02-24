/*
 * Copyright (c) 2015-2021 VG-Lab/URJC.
 *
 * Authors: Felix de las Pozas <felix.delaspozas@urjc.es>
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

// Sumrice
#include <future>
#include <simil/Network.h>
#include <simil/SimulationData.h>
#include <simil/SpikeData.h>
#include <sumrice/LoaderThread.h>

#ifdef SIMIL_WITH_REST_API

#include <simil/loaders/LoaderRestData.h>

#endif

LoaderThread::LoaderThread( )
  : QThread{ nullptr }
  , m_type{ simil::TDataType::TDataUndefined }
  , m_network{ nullptr }
  , m_data{ nullptr }
#ifdef SIMIL_WITH_REST_API
  , m_rest{ nullptr }
#endif
{
}

void LoaderThread::setData( const simil::TDataType type ,
                            const std::string& arg1 ,
                            const std::string& arg2 )
{
  assert( type != simil::TDataType::TREST );

  m_type = type;
  m_arg1 = arg1;
  m_arg2 = arg2;
}

std::shared_ptr< simil::Network >
LoaderThread::network( ) const
{
  return m_network;
}

std::shared_ptr< simil::SimulationData >
LoaderThread::simulationData( ) const
{
  return m_data;
}

std::string LoaderThread::errors( ) const
{
  return m_errors;
}

simil::TDataType LoaderThread::type( ) const
{
  return m_type;
}

void LoaderThread::run( )
{
  emit progress( 25 );

  try
  {
    switch ( m_type )
    {
      case simil::TDataType::TBlueConfig:
      case simil::TDataType::TCSV:
      case simil::TDataType::THDF5:
      {
        auto spikesData = new simil::SpikeData( m_arg1 , m_type , m_arg2 );
        spikesData->reduceDataToGIDS( );
        m_data = std::unique_ptr< simil::SimulationData >( spikesData );

        emit progress( 50 );

        emit network( m_data->positions( ).size( ));
        emit spikes( spikesData->spikes( ).size( ));

      }
        break;
      case simil::TDataType::TREST:
      {
#ifdef SIMIL_WITH_REST_API
        if ( m_rest == nullptr )
        {
          m_rest = std::make_unique< Loader >( );
        }

        m_rest->setConfiguration( m_restConfig );

        const auto url = m_restConfig.url;
        const auto port = std::to_string( m_restConfig.port );
        const auto version = m_rest->getVersion( url , m_restConfig.port );

        if ( version.api.empty( ) || version.insite.empty( ))
        {
          m_errors = std::string( "Unknown protocol connecting to " )
                     + m_restConfig.url + ":" +
                     std::to_string( m_restConfig.port );
          break;
        }


        std::packaged_task< std::unique_ptr< simil::SimulationData >( ) >
          simulationTask(
          [ this , url , port ]( )
          {
            return m_rest->loadSimulationData( url , port );
          } );

        auto simulationDataFuture = simulationTask.get_future( );
        std::thread simulationThread( std::move( simulationTask ));

        if ( m_network == nullptr )
        {
          // The override network is not defined.
          // Let's load it!
          std::packaged_task< std::unique_ptr< simil::Network >( ) > networkTask(
            [ this , url , port ]( )
            {
              return m_rest->loadNetwork( url , port );
            } );

          auto networkFuture = networkTask.get_future( );
          std::thread networkThread( std::move( networkTask ));
          networkThread.join( );
          m_network = networkFuture.get( );
        }

        simulationThread.join( );
        m_data = simulationDataFuture.get( );

        emit progress( 50 );
        emit network( m_data->positions( ).size( ));

        if ( auto* ptr = dynamic_cast<simil::SpikeData*>(m_data.get( )))
        {
          emit spikes( ptr->spikes( ).size( ));
        }
#else
        m_errors = "REST data loading is unsupported.";
#endif
      }
        break;
      default:
      case simil::TDataType::TCONE:
      case simil::TDataType::TDataUndefined:
        m_errors = "Unsupported data type.";
        break;
    }
  }
  catch ( const std::exception& ex )
  {
    m_errors = std::string( "sumrice::LoaderThread::run() -> " ) +
               std::string( ex.what( ));
  }
  catch ( ... )
  {
    m_errors = std::string(
      "sumrice::LoaderThread::run() -> Un-handled exception when loading data. " );
  }

  emit progress( 75 );

  if ( !m_errors.empty( ) && m_type != simil::TDataType::TREST )
  {
    m_errors += std::string( "\ndata type: " ) +
                std::to_string( static_cast<int>(m_type));
    m_errors += std::string( " argument 1: " ) + m_arg1;
    m_errors += std::string( " argument 2: " ) + m_arg2;
  }

  emit progress( 100 );
}

#ifdef SIMIL_WITH_REST_API

simil::LoaderRestData* LoaderThread::RESTLoader( ) const
{
  if ( m_type != simil::TDataType::TREST )
  {
    throw std::logic_error( "No REST data loader!" );
  }

  return m_rest.get( );
}

void LoaderThread::setRESTConfiguration(
  const simil::LoaderRestData::Configuration& config )
{
  m_type = simil::TDataType::TREST;
  m_restConfig = config;

  m_network = config.network.lock();

  if ( m_rest )
  {
    m_rest->setConfiguration( m_restConfig );
  }
}

#endif

std::string LoaderThread::filename( ) const
{
  return m_arg1;
}
