/*
 * Copyright (c) 2022-2022 VG-Lab/URJC.
 *
 * Authors: Félix de las Pozas Álvarez <felix.delaspozas@urjc.es>
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

#ifndef SUMRICE_CONNECTRESTDIALOG_H_
#define SUMRICE_CONNECTRESTDIALOG_H_

// Sumrice
#include <sumrice/ConfigureRESTDialog.h>

// Qt
#include <QDialog>

class QComboBox;
class QLineEdit;

/** \class ConnectRESTDialog
 * \brief Implements a dialog for a REST connection configuration.
 *
 */
class ConnectRESTDialog
: public QDialog
{
    Q_OBJECT
  public:
    /** \brief ConnectRESTDialog class constructor.
     * \param[in] parent Raw pointer of the widget parent of this one.
     * \param[in] f Qt dialog flags.
     *
     */
    explicit ConnectRESTDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    /** \brief ConnectRESTDialog class virtual destructor.
     *
     */
    virtual ~ConnectRESTDialog()
    {};

    struct Connection
    {
        QString protocol;
        QString url;
        unsigned int port;

        Connection(): protocol("NEST"), url("localhost"), port(28080) {};
    };

    /** \brief Sets the connection values.
     * \param[in] r RESTConnection struct reference.
     *
     */
    void setRESTConnection(const Connection &r);

    /** \brief Returns the REST connection options.
     *
     */
    Connection getRESTConnection() const;

    /** \brief Sets the REST configuration values.
     * \param[in] o RESTConfigurationWidget options struct reference.
     *
     */
    void setRESTOptions(const RESTConfigurationWidget::Options &o);

    /** \brief Returns the REST configuration options values.
     *
     */
    RESTConfigurationWidget::Options getRESTOptions() const;

  private:
    QComboBox               *m_protocol; /** REST protocol, NEST or ARBOR.      */
    QLineEdit               *m_url;      /** url of server.                     */
    QLineEdit               *m_port;     /** port of server.                    */
    RESTConfigurationWidget *m_options;  /** REST configuration options widget. */
};

#endif /* SUMRICE_CONNECTRESTDIALOG_H_ */
