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

#ifndef SUMRICE_LOADERTHREAD_H_
#define SUMRICE_LOADERTHREAD_H_

// Sumrice
#include <sumrice/api.h>

// Simil
#include <simil/types.h>
#include <simil/api.h>
#ifdef SIMIL_WITH_REST_API
#include <simil/loaders/LoaderRestData.h>
#endif

// Qt
#include <QThread>
#include <QTimer>

// C++
#include <string>

namespace simil
{
  class Network;
  class SimulationData;
}


/** \class LoaderThread
 * \brief Loads the data in a separated thread.
 *
 */
class SUMRICE_API LoaderThread
: public QThread
{
    Q_OBJECT
  public:
    /** \brief LoaderThread class constructor.
     *
     */
    LoaderThread();

    /** \brief LoaderThread virtual destructor.
     *
     */
    virtual ~LoaderThread();

    /** \brief Set the information for the data to load. If data
     * is REST use setRESTConfiguration().
     * \param[in] type Data origin type.
     * \param[in] arg1 Load argument 1.
     * \param[in] arg1 Load argument 1.
     * \param[in] arg1 Load argument 1.
     *
     */
    void setData(const simil::TDataType type,
                 const std::string &arg1,
                 const std::string &arg2);

    /** \brief Returns the loaded network data. Only valid after finished() signal.
     *
     */
    simil::Network *network() const;

    /** \brief Returns the simulation data. Only valid after finished() signal.
     *
     */
    simil::SimulationData* simulationData() const;

    /** \brief Returns an error string or emtpy if success.
     *
     */
    std::string errors() const;

    /** \brief Returns the data type.
     *
     */
    simil::TDataType type() const;

#ifdef SIMIL_WITH_REST_API
    /** \brief Sets the REST loader protocol configuration.
     * \param[in] o REST protocol configuration.
     *
     */
    void setRESTConfiguration(const simil::LoaderRestData::Configuration &o);

    /** \brief Returns the REST loader.
     *
     */
    simil::LoaderRestData *RESTLoader();
#endif

  protected:
    virtual void run();

  signals:
    void progress(int);
    void spikes(unsigned int);
    void network(unsigned int);

  private:
    simil::TDataType       m_type;       /** data origin type.                    */
    std::string            m_arg1;       /** argument 1, meaning depends on type. */
    std::string            m_arg2;       /** argument 2, meaning depends on type. */
    simil::Network*        m_network;    /** loaded network.                      */
    simil::SimulationData* m_data;       /** loaded data.                         */
#ifdef SIMIL_WITH_REST_API
    using Loader = simil::LoaderRestData;

    Loader*                m_rest;       /** rest data importer.                  */
    Loader::Configuration  m_restConfig; /** rest connnection configuration.      */
#endif
    std::string            m_errors;     /** error messages or empty if success.  */
};

#endif /* SUMRICE_LOADERTHREAD_H_ */
