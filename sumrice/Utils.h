/*
 * Utils.h
 *
 *  Created on: Jun 18, 2021
 *      Author: felix
 */

#ifndef SUMRICE_UTILS_H_
#define SUMRICE_UTILS_H_

#include <string>
#include <memory>
#include <thread>

#ifdef VISIMPL_USE_ZEROEQ
#include <zeroeq/zeroeq.h>
#endif

namespace visimpl
{
  bool isValidIPAddress(const std::string &address);

#ifdef VISIMPL_USE_ZEROEQ
  class ZeroEQConfig
  {
    public:
      /** \brief Singleton instance method.
       *
       */
      static ZeroEQConfig &instance();

      /** \brief ZeroEQConfig session constructor.
       * \param[in] s Session id.
       *
       */
      void connect(const std::string &s = std::string());

      /** \brief ZeroEQConfig null session constructor.
       * \param[in] h Host address.
       * \param[in] p Address port.
       * \param[in] invert false to use port for subscriber, true to use it with publisher.
       *
       */
      void connect(const std::string &h, const uint16_t p, bool invert = false);

      /** \brief Uses a publisher in the range of known ports and subscribes to all except used publisher.
       *
       */
      void connectNullSession();

      /** \brief Prints the current configuration.
       *
       */
      void print() const;

      /** \brief Returns the publisher connection port or 0 if not connected.
       *
       */
      uint16_t port() const
      { return m_port; }

      /** \brief Returns the publisher host or empty if not connected.
       *
       */
      const std::string &host() const
      { return m_host; }

      /** \brief Returns the session id or empty if not connected.
       *
       */
      const std::string &session() const
      { return m_session; }

      /** \brief Returns the ZeroEQ subscriber or nullptr if not connected.
       *
       */
      std::shared_ptr<zeroeq::Subscriber> subscriber() const
      { return m_subscriber; }

      /** \brief Returns the ZeroEQ publisher or nullptr if not connected.
       *
       */
      std::shared_ptr<zeroeq::Publisher> publisher() const
      { return m_publisher; }

      /** \brief Returns true if connected and false otherwise.
       *
       */
      bool isConnected() const
      { return (m_subscriber != nullptr) && (m_publisher != nullptr); };

      /** \brief Disconnects from ZeroEQ if connected and clears the variables.
       *
       */
      void disconnect();

      /** \brief Starts the event receive loop.
       *
       */
      void startReceiveLoop();

      ZeroEQConfig(ZeroEQConfig const&)   = delete;
      void operator=(ZeroEQConfig const&) = delete;

    private:
      /** \brief Ports used for null session without parameters, the first free will be used
       * for publisher and the rest will be subscribed.
       *
       */
      std::vector<uint16_t> PORTS{50000,51000,52000,53000,54000};

      /** \brief ZeroEQConfig empty class constructor.
       *
       */
      ZeroEQConfig()
      : m_host{}, m_port{0}, m_session(zeroeq::DEFAULT_SESSION), m_subscriber{nullptr}, m_publisher{nullptr}, m_thread{nullptr}, m_run{false} {};

      /** \brief ZeroEQConfig destructor.
       *
       */
      ~ZeroEQConfig();

    private:
      std::string                         m_host;       /** null session host address.               */
      uint16_t                            m_port;       /** null session adddres port.               */
      std::string                         m_session;    /** session id.                              */
      std::shared_ptr<zeroeq::Subscriber> m_subscriber; /** ZeroEQ subscriber.                       */
      std::shared_ptr<zeroeq::Publisher>  m_publisher;  /** ZeroEQ publisher.                        */
      std::shared_ptr<std::thread>        m_thread;     /** receive loop thread. std::make_unique... */
      bool                                m_run;        /** true if thread running, false to stop.   */
  };
#endif
}

#endif /* SUMRICE_UTILS_H_ */
