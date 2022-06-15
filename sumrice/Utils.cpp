/*
 * Copyright (c) 2021 VG-Lab/URJC.
 *
 * Authors: Felix de las Pozas Alvarez <felix.delaspozas@urjc.es>
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

// Project
#include "Utils.h"

// C++
#include <algorithm>
#include <cctype>

//----------------------------------------------------------------------------
bool visimpl::isValidIPAddress(const std::string &address)
{
  std::string address_c = address;
  std::for_each(address_c.begin(), address_c.end(), tolower);

  if(address_c.length() < 7 || address_c.length() > 15) return false;
  if(address_c.compare("localhost") == 0) return true;

   char tail[16];
   tail[0] = 0;

   unsigned int parts[4];
   int c = sscanf(address_c.c_str(), "%3u.%3u.%3u.%3u%s", &parts[0], &parts[1], &parts[2], &parts[3], tail);

   if (c != 4 || tail[0])
       return false;

   for (int i = 0; i < 4; i++)
       if (parts[i] > 255)
           return false;

   return true;
}

#ifdef VISIMPL_USE_ZEROEQ
//----------------------------------------------------------------------------
visimpl::ZeroEQConfig& visimpl::ZeroEQConfig::instance()
{
  static ZeroEQConfig eqConfig;

  return eqConfig;
}

//----------------------------------------------------------------------------
void visimpl::ZeroEQConfig::connect(const std::string &s)
{
  if(m_subscriber || m_publisher)
  {
    const auto message = std::string("ZeroEQ Already connected. ") + __FILE__ + ":" + std::to_string(__LINE__);
    throw std::runtime_error(message);
  }

  if(s.compare(zeroeq::NULL_SESSION) == 0)
  {
    const auto message = std::string("Invalid NULL_SESSION connection, missing host and port. ") + __FILE__ + ":" + std::to_string(__LINE__);
    throw std::runtime_error(message);
  }

  m_session = s.empty() ? zeroeq::DEFAULT_SESSION : s;

  try
  {
    m_subscriber = std::make_shared<zeroeq::Subscriber>(m_session);
    m_publisher = std::make_shared<zeroeq::Publisher>(m_session);
    m_host = m_publisher->getURI().getHost();
    m_port = m_publisher->getURI().getPort();
  }
  catch(const std::exception &e)
  {
    std::cerr << "Exception in ZeroEQConfig when connecting to session " << m_session << std::endl;
    std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
    disconnect();
    throw;
  }

  print();
}

//----------------------------------------------------------------------------
void visimpl::ZeroEQConfig::connect(const std::string &h, const uint16_t p, bool invert)
{
  if(m_subscriber || m_publisher)
  {
    const auto message = std::string("ZeroEQ Already connected. ") + __FILE__ + ":" + std::to_string(__LINE__);
    throw std::runtime_error(message);
  }

  m_host = h;
  m_port = p;
  m_session = zeroeq::NULL_SESSION;

  if(!isValidIPAddress(m_host))
  {
    const auto message = std::string("Invalid host address for NULL_SESSION. ") + __FILE__ + ":" + std::to_string(__LINE__);
    throw std::runtime_error(message);
  }

  if(m_port < 1024)
  {
    const auto message = std::string("Invalid port for NULL_SESSION. ") + __FILE__ + ":" + std::to_string(__LINE__);
    throw std::runtime_error(message);
  }

  zeroeq::URI uri_subscriber;
  uri_subscriber.setHost(m_host);
  uri_subscriber.setPort(m_port + (invert ? 1:0));

  zeroeq::URI uri_publisher;
  uri_publisher.setHost(m_host);
  uri_publisher.setPort(m_port + (!invert ? 1:0));

  try
  {
    m_publisher = std::make_shared<zeroeq::Publisher>(uri_publisher, m_session);
    m_subscriber = std::make_shared<zeroeq::Subscriber>(uri_subscriber);
  }
  catch(const std::exception &e)
  {
    std::cerr << "Exception in ZeroEQConfig when connecting to session " << m_session << std::endl;
    std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
    disconnect();
    throw;
  }

  print();
}

//----------------------------------------------------------------------------
void visimpl::ZeroEQConfig::print() const
{
  zeroeq::URI uri;

  std::cout << "ZeroEQConfig -> ";
  if(!m_publisher || !m_subscriber)
  {
    std::cout << "not connected." << std::endl;
  }
  else
  {
    std::string session = m_session == zeroeq::NULL_SESSION ? "NULL_SESSION" : m_session;
    session = m_session == zeroeq::DEFAULT_SESSION ? "DEFAULT_SESSION" : session;

    std::cout << "publisher uri: " << m_host << ":" << m_port << " - subscriber to session: " << session << " - "
              << "receiver loop: " << (m_run ? "":"not ") << "running." << std::endl;
  }
}

//----------------------------------------------------------------------------
void visimpl::ZeroEQConfig::disconnect()
{
  m_publisher = nullptr;
  m_subscriber = nullptr;
  m_session.clear();
  m_host.clear();
  m_port = 0;
  stopReceiveLoop();
}

//----------------------------------------------------------------------------
visimpl::ZeroEQConfig::~ZeroEQConfig()
{
  disconnect();
  std::cout << "ZeroEQConfig -> disconnected." << std::endl;
}

//----------------------------------------------------------------------------
void visimpl::ZeroEQConfig::connectNullSession()
{
  if(m_subscriber || m_publisher)
  {
    const auto message = std::string("ZeroEQ Already connected. ") + __FILE__ + ":" + std::to_string(__LINE__);
    throw std::runtime_error(message);
  }

  m_session = zeroeq::NULL_SESSION;
  m_host = "127.0.0.1";

  for(const int port: PORTS)
  {
    try
    {
      m_port = port;

      zeroeq::URI uri;
      uri.setHost(m_host);
      uri.setPort(m_port);

      std::cout << "ZeroEQ: try publisher in port " << port << std::endl;
      m_publisher = std::make_shared<zeroeq::Publisher>(uri, m_session);
      break;
    }
    catch(...)
    {
      // nothing to do, unable to create a publisher in the
      // used port, use next port until we run out of ports.
    }
  }

  if(!m_publisher)
  {
    std::cerr << "Exception in ZeroEQConfig when connecting to session " << m_session << std::endl;
    std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
    disconnect();
    const auto message = std::string("ZeroEQ unable to publish in any of the known ports. ") + __FILE__ + ":" + std::to_string(__LINE__);
    throw std::runtime_error(message);
  }

  zeroeq::URIs uris;

  std::string subscriberPorts;
  for(const int port: PORTS)
  {
    if(port == m_publisher->getURI().getPort())
      continue;

    zeroeq::URI uri;
    uri.setHost(m_host);
    uri.setPort(port);

    uris.push_back(uri);

    subscriberPorts += subscriberPorts.empty() ? "":",";
    subscriberPorts += std::to_string(port);
  }

  try
  {
    std::cout << "ZeroEQ: try to subscribe to ports " << subscriberPorts << std::endl;
    m_subscriber = std::make_shared<zeroeq::Subscriber>(uris);
  }
  catch(std::exception &e)
  {
    std::cerr << "Exception in ZeroEQConfig when connecting to session " << m_session << std::endl;
    std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
    disconnect();
    throw;
  }

  print();
}

//----------------------------------------------------------------------------
void visimpl::ZeroEQConfig::startReceiveLoop()
{
  if(!m_thread && m_subscriber)
  {
    std::cout << "ZeroEQConfig STARTS TREAD" << std::endl;
    m_run = true;
    m_thread = std::make_shared<std::thread>([this]( )
      {
        while ( m_run )
        {
          try
          {
            if(m_subscriber)
            {
              m_subscriber->receive( 10000 );
            }
          }
          // exception in zmq_poll
          catch(const std::exception &e)
          {
            std::cerr << "ZeroEQConfig STOPS LISTENING TREAD because exception: " << e.what() << std::endl;
            m_run = false;
          }
          catch(...)
          {
            std::cerr << "ZeroEQConfig STOPS LISTENING TREAD because unknown exception." << std::endl;
            m_run = false;
          }
        }
      });
  }
}

//----------------------------------------------------------------------------
void visimpl::ZeroEQConfig::stopReceiveLoop()
{
  if(m_run)
  {
    m_run = false;
    if(m_thread) m_thread->join();

  }
  m_thread = nullptr;
}

#endif
