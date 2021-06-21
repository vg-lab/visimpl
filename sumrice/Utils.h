/*
 * Utils.h
 *
 *  Created on: Jun 18, 2021
 *      Author: felix
 */

#ifndef SUMRICE_UTILS_H_
#define SUMRICE_UTILS_H_

#include <string>

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
      void connect(const std::string &h, const uint32_t p, bool invert = false);

      /** \brief Prints the current configuration.
       *
       */
      void print() const;

      /** \brief Returns the publisher connection port or 0 if not connected.
       *
       */
      uint32_t port() const
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

      ZeroEQConfig(ZeroEQConfig const&)   = delete;
      void operator=(ZeroEQConfig const&) = delete;
    private:
      /** \brief ZeroEQConfig empty class constructor.
       *
       */
      ZeroEQConfig(): m_host{}, m_port{0}, m_session(zeroeq::DEFAULT_SESSION), m_subscriber{nullptr}, m_publisher{nullptr} {};
      ~ZeroEQConfig();

    private:
      std::string                         m_host;       /** null session host address. */
      uint32_t                            m_port;       /** null session adddres port. */
      std::string                         m_session;    /** session id.                */
      std::shared_ptr<zeroeq::Subscriber> m_subscriber; /** ZeroEQ subscriber.         */
      std::shared_ptr<zeroeq::Publisher>  m_publisher;  /** ZeroEQ publisher.          */
  };
#endif
}

#endif /* SUMRICE_UTILS_H_ */
