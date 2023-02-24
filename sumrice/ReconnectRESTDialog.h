/*
 *  Copyright (c) 2015-2023 VG-Lab/URJC.
 *
 *  Authors: Gael Rial Costas  <gael.rial.costas@urjc.es>
 *
 *  This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
 *
 *  This library is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU Lesser General Public License version 3.0 as published
 *  by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

//
// Created by grial on 22/02/23.
//

#ifndef VISIMPL_RECONNECTRESTDIALOG_H
#define VISIMPL_RECONNECTRESTDIALOG_H

#include "ConnectRESTDialog.h"

#include <QDialog>
#include <QRadioButton>


#include <simil/loaders/LoaderRestData.h>

class ReconnectRESTDialog : public QDialog
{

public:

  enum class Selection
  {
    SPIKES ,
    SPIKES_AND_NETWORK ,
    NEW_CONNECTION
  };

private:

Q_OBJECT

  QRadioButton* _spikes;
  QRadioButton* _both;
  QRadioButton* _newConnection;

public:

  explicit ReconnectRESTDialog( QWidget* parent = nullptr ,
                                Qt::WindowFlags f = Qt::WindowFlags( ));

  Selection getSelection( ) const;

};


#endif //VISIMPL_RECONNECTRESTDIALOG_H
