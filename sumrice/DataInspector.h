/*
 * Copyright (c) 2015-2020 VG-Lab/URJC.
 *
 * Authors: Aaron Sujar <aaron.sujar@urjc.es>
 *
 * This file is part of SimIL <https://github.com/gmrvvis/SimIL>
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

#ifndef DATAINSPECTOR_H
#define DATAINSPECTOR_H

// Qt
#include <QGroupBox>
#include <QTimer>

// Simil
#include <simil/simil.h>

#include <sumrice/api.h>

class QWidget;
class QLabel;

class SUMRICE_API DataInspector 
: public QGroupBox
{
  Q_OBJECT
public:
    DataInspector(const QString &title, QWidget *parent = nullptr);
    virtual ~DataInspector() {};

    void addWidget(QWidget *widget, int row, int column,
                   int rowSpan, int columnSpan,
                   Qt::Alignment alignment = Qt::Alignment());

    /** \brief Sets the simulation player to check for updates.
     * \param[in] simPlayer_ Simulation player raw pointer.
     *
     */
    void setSimPlayer(simil::SimulationPlayer * simPlayer_);

    /** \brief Enables/disables the player update checks.
     * \param[in] value True to enable false otherwise.
     *
     */
    void setCheckUpdates(const bool value);

    /** \brief Check for player updates interval, default 5000 milliseconds.
     *
     */
    void setCheckTimer(const int ms);

signals:
  void simDataChanged( void );

protected slots:
  void updateInfo( void );

protected:
    virtual void paintEvent(QPaintEvent *e);

    unsigned int _gidsize;
    unsigned int _spikesize;
    QLabel * _labelGIDs;
    QLabel *_labelSpikes;
    QLabel *_labelStartTime;
    QLabel *_labelEndTime;
    simil::SimulationPlayer * _simPlayer;
    bool m_check;
    QTimer m_timer;
};

#endif // DATAINSPECTOR_H
