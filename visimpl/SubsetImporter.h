/*
 * Copyright (c) 2015-2020 VG-Lab/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
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

#ifndef SUBSETIMPORTER_H_
#define SUBSETIMPORTER_H_

// Qt
#include <QDialog>
#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

// SimIL
#include <simil/simil.h>

class QListWidget;

namespace visimpl
{
  class SubsetImporter : public QDialog
  {
    Q_OBJECT

  public:
    SubsetImporter(QWidget* parent_ = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~SubsetImporter( ) {};

    void init( void );

    void reload( const simil::SubsetEventManager* );
    void clear( void );

    const std::vector< std::string > selectedSubsets( void ) const;

  protected:
    const simil::SubsetEventManager* _subsetEventManager;

    enum TSubsetLine { sl_container = 0, sl_layout, sl_checkbox, sl_label };
    typedef std::tuple< QWidget*, QGridLayout*, QCheckBox*, QLabel* > tSubsetLine;

    QPushButton* _buttonAccept;
    QPushButton* _buttonCancel;

    QListWidget* _listSubsets;

    std::map< std::string, tSubsetLine > _subsets;
  };
}

#endif /* SUBSETIMPORTER_H_ */
