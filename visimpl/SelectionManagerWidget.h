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

#ifndef SELECTIONMANAGERWIDGET_H_
#define SELECTIONMANAGERWIDGET_H_

// Qt
#include <QListView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLineEdit>
#include <QRadioButton>

// C++
#include <unordered_set>

// ViSimpl
#include "types.h"

namespace visimpl
{
  class SelectionManagerWidget
  : public QWidget
  {
    Q_OBJECT;
  public:

    enum TSeparator
    {
      TSEP_NEWLINE = 0,
      TSEP_SPACE,
      TSEP_TAB,
      TSEP_OTHER
    };

    SelectionManagerWidget( QWidget* parent = nullptr );
    virtual ~SelectionManagerWidget( void ) {};

    void init( void );

    void setGIDs( const TGIDSet& all_,
                  const TGIDUSet& selected_ = { });

    void setSelected( const TGIDUSet& selected_ );
    const TGIDUSet& selected( void ) const;

    void clearSelection( void );

  signals:
    void selectionChanged( void );

  protected slots:
    void _addToSelected( void );
    void _removeFromSelected( void );

    void _buttonBrowseClicked( void );
    void _buttonSaveClicked( void );

    void _buttonCancelClicked( void );
    void _buttonAcceptClicked( void );

  protected:
    void _initTabSelection( void );
    void _initTabExport( void );

    void _fillLists( void );
    void _reloadLists( void );

    void _updateListsLabelNumbers( void );

    void _saveToFile( const QString& filePath,
                      const QString& separator = "\n",
                      const QString& prefix = "",
                      const QString& suffix = "" );

    TGIDUSet _gidsAll;
    TGIDUSet _gidsSelected;
    TGIDUSet _gidsAvailable;

    QTabWidget* _tabWidget;

    // Selection tab
    QListView* _listViewAvailable;
    QListView* _listViewSelected;

    QStandardItemModel* _modelAvailable;
    QStandardItemModel* _modelSelected;

    QPushButton* _buttonAddToSelection;
    QPushButton* _buttonRemoveFromSelection;

    QLabel* _labelAvailable;
    QLabel* _labelSelection;

    TUIntUintMap _gidIndex;

    // Export tab
    QLineEdit* _lineEditFilePath;
    QLineEdit* _lineEditPrefix;
    QLineEdit* _lineEditSuffix;
    QLineEdit* _lineEditSeparator;

    QRadioButton* _radioNewLine;
    QRadioButton* _radioSpace;
    QRadioButton* _radioTab;
    QRadioButton* _radioOther;

    QPushButton* _buttonBrowse;
    QPushButton* _buttonSave;

    QString _pathExportDefault;
  };
}

#endif /* SELECTIONMANAGERWIDGET_H_ */
