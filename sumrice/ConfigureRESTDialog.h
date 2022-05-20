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

#ifndef SUMRICE_CONFIGURERESTDIALOG_H_
#define SUMRICE_CONFIGURERESTDIALOG_H_

#include <QDialog>
#include <QWidget>

class QSpinBox;

/** \class RESTConfigurationWidget
 * \brief Implements a widget with the REST configuration options.
 *
 */
class RESTConfigurationWidget
: public QWidget
{
    Q_OBJECT
  public:
    /** \brief RESTConfigurationWidget class constructor.
     * \param[in] parent Raw pointer of the QWidget parent of this one.
     * \param[in] f Qt widget flags.
     *
     */
    explicit RESTConfigurationWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    /** \brief RESTConfigurationWidget class virtual destructor.
     *
     */
    virtual ~RESTConfigurationWidget()
    {};

    /** \struct Options
     * \brief Contains the REST options.
     *
     */
    struct Options
    {
        unsigned int waitTime;   /** time to wait in ms after a successful call. */
        unsigned int failTime;   /** time to wait in ms after a failed call.     */
        unsigned int spikesSize; /** size of spikes to ask in a call.            */

        Options(): waitTime(5000), failTime(1000), spikesSize(1000) {};
    };

    /** \brief Sets the widget options values.
     * \param[in] o Options struct reference.
     *
     */
    void setOptions(const Options &o);

    /** \brief Returns the widget values as a Options struct.
     *
     */
    Options getOptions() const;

  private:
    QSpinBox *m_waitTime;   /** wait time value spinbox.   */
    QSpinBox *m_spikesSize; /** spikes size value spinbox. */
};

/** \class ConfigureRESTDialog
 * \brief Implements a dialog to configure REST connection options.
 *
 */
class ConfigureRESTDialog
: public QDialog
{
    Q_OBJECT
  public:
    /** \brief ConfigureRESTDialog class constructor.
     * \param[in] parent Raw pointer of the widget parent of this one.
     * \param[in] f Qt dialog flags.
     * \param[in] o REST configuration options initial values.
     *
     */
    explicit ConfigureRESTDialog(QWidget *parent = nullptr,
                                 Qt::WindowFlags f = Qt::WindowFlags(),
                                 const RESTConfigurationWidget::Options &o = RESTConfigurationWidget::Options());

    /** \brief ConfigureRESTDialog class virtual destructor.
     *
     */
    virtual ~ConfigureRESTDialog()
    {}

    /** \brief Returns the configured options.
     *
     */
    RESTConfigurationWidget::Options getRESTOptions() const;

    /** \brief Sets the options values.
     *
     */
    void setRESTOptions(const RESTConfigurationWidget::Options &o);

  private:
    RESTConfigurationWidget *m_options; /** options widget */
};

#endif /* SUMRICE_CONFIGURERESTDIALOG_H_ */
