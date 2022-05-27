/*
 * Copyright (c) 2015-2022 VG-Lab/URJC.
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

#ifndef SUMRICE_CLOSEDATADIALOG_H_
#define SUMRICE_CLOSEDATADIALOG_H_

// Qt
#include <QDialog>

class QRadioButton;
class QShowEvent;

/** \class CloseDataDialog
 * \brief Implements the dialog to let the user
 *        choose closing options for REST datasets.
 *
 */
class CloseDataDialog
: public QDialog
{
    Q_OBJECT
  public:
    /** \brief CloseDataDialog class constructor.
     * \param[in] parent Raw pointer of the widget parent of this one.
     * \param[in] flags Qt dialog flags.
     *
     */
    explicit CloseDataDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    /** \brief CloseDataDialog class virtual destructor.
     *
     */
    virtual ~CloseDataDialog()
    {};

    /** \brief Returns true if the user selected to keep previous network data and
     * false otherwise.
     *
     */
    bool keepNetwork() const;

  protected:
    virtual void showEvent(QShowEvent *event) override;

  private:
    QRadioButton *m_keep; /** Radio button for 'keep network and settings' option. */
};

#endif /* SUMRICE_CLOSEDATADIALOG_H_ */
