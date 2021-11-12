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
#include <simil/Network.h>
#include <simil/SimulationData.h>
#include <simil/SpikeData.h>
#include <sumrice/LoaderThread.h>

#ifdef SIMIL_WITH_REST_API
#include <simil/loaders/LoaderRestData.h>

const float REST_DELTATIME = 0.005f; /* defined in application for rest data. */
#endif

LoaderThread::LoaderThread()
: QThread{nullptr}
, m_type{simil::TDataType::TDataUndefined}
, m_network{nullptr}
, m_data{nullptr}
#ifdef SIMIL_WITH_REST_API
, m_rest{nullptr}
#endif
{
}

LoaderThread::~LoaderThread()
{
#ifdef SIMIL_WITH_REST_API
  if(m_rest) delete m_rest;
#endif
}

void LoaderThread::setData(const simil::TDataType type,
                           const std::string &arg1,
                           const std::string &arg2)
{
  m_type = type;
  m_arg1 = arg1;
  m_arg2 = arg2;
}

simil::Network* LoaderThread::network() const
{
  return m_network;
}

simil::SimulationData* LoaderThread::simulationData() const
{
  return m_data;
}

std::string LoaderThread::errors() const
{
  return m_errors;
}

simil::TDataType LoaderThread::type() const
{
  return m_type;
}

void LoaderThread::run()
{
  emit progress(25);

  try
  {
    switch(m_type)
    {
      case simil::TDataType::TBlueConfig:
      case simil::TDataType::TCSV:
      case simil::TDataType::THDF5:
        {
          auto spikesData = new simil::SpikeData(m_arg1, m_type, m_arg2);
          spikesData->reduceDataToGIDS();
          m_data = spikesData;

          emit progress(50);

          emit network(m_data->positions().size());
          emit spikes(spikesData->spikes().size());
        }
        break;
      case simil::TDataType::TREST:
        {
#ifdef SIMIL_WITH_REST_API
          m_rest = new simil::LoaderRestData();
          m_rest->deltaTime(REST_DELTATIME);
          m_network = m_rest->loadNetwork(m_arg1, m_arg2);
          m_data = m_rest->loadSimulationData(m_arg1, m_arg2);

          unsigned int oldSpikes = 0, oldNetwork = 0;
          for(int i = 0; i < 5; ++i)
          {
            QThread::sleep(1);
            const auto newNetwork = m_network->gidsSize();
            if(newNetwork != oldNetwork)
            {
              oldNetwork = newNetwork;
              emit network(newNetwork);
            }

            const auto newSpikes = dynamic_cast<simil::SpikeData *>(m_data)->spikes().size();
            if(oldSpikes != newSpikes)
            {
              oldSpikes = newSpikes;
              emit spikes(newSpikes);
            }
          }
          emit progress(50);
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
  catch(const std::exception &ex)
  {
    m_errors = std::string("simil::LoaderThread::run() -> ") + std::string(ex.what());
  }
  catch(...)
  {
    m_errors = std::string("simil::LoaderThread::run() -> Unhandled exception when loading data. ");
  }

  emit progress(75);

  if(!m_errors.empty())
  {
    m_errors += std::string("\ndata type: ") + std::to_string(static_cast<int>(m_type));
    m_errors += std::string(" argument 1: ") + m_arg1;
    m_errors += std::string(" argument 2: ") + m_arg2;
  }

  emit progress(100);
}
