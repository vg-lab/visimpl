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

// ViSimpl
#include <qabstractitemmodel.h>
#include <qabstractitemview.h>
#include <qalgorithms.h>
#include <qboxlayout.h>
#include <qchar.h>
#include <qdir.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qflags.h>
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qicon.h>
#include <qiodevice.h>
#include <qitemselectionmodel.h>
#include <qkeysequence.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qshortcut.h>
#include <qstandarditemmodel.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <qtextstream.h>
#include <qvariant.h>
#include <sumrice/types.h>
#include <visimpl/SelectionManagerWidget.h>

namespace visimpl
{
  SelectionManagerWidget::SelectionManagerWidget( QWidget* parent_ )
  : QWidget( parent_ )
  , _listViewAvailable( nullptr )
  , _listViewSelected( nullptr )
  , _modelAvailable( nullptr )
  , _modelSelected( nullptr )
  , _buttonAddToSelection( nullptr )
  , _buttonRemoveFromSelection( nullptr )
  {
    init( );

    setWindowTitle(tr("Selection Manager"));
    setWindowIcon(QIcon(":/visimpl.png"));
  }

  void SelectionManagerWidget::init( void )
  {
    auto layoutTop = new QVBoxLayout( );
    this->setLayout( layoutTop );

    _tabWidget = new QTabWidget( );
    QWidget* containerFoot = new QWidget( );
    auto layoutFoot = new QGridLayout( );
    containerFoot->setLayout( layoutFoot );

    layoutTop->addWidget( _tabWidget );
    layoutTop->addWidget( containerFoot );

    _initTabSelection( );
    _initTabExport( );

    // Foot
    QPushButton* buttonCancel = new QPushButton( "Cancel" );
    buttonCancel->setMaximumWidth( 100 );
    QPushButton* buttonAccept = new QPushButton( "Accept" );
    buttonAccept->setMaximumWidth( 100 );

    layoutFoot->addWidget( buttonCancel, 0, 0, 1, 1 );
    layoutFoot->addWidget( buttonAccept, 0, 2, 1, 1 );

    connect( buttonCancel, SIGNAL( clicked( void )),
             this, SLOT( _buttonCancelClicked( void )));
    connect( buttonAccept, SIGNAL( clicked( void )),
             this, SLOT( _buttonAcceptClicked( void )));

    new QShortcut( QKeySequence( Qt::Key_Escape ), this, SLOT( _buttonCancelClicked( )));
  }

  void SelectionManagerWidget::_initTabSelection( void )
  {
    auto containerSelection = new QWidget( );

    auto layoutSelection = new QGridLayout( );
    containerSelection->setLayout( layoutSelection );

    _labelAvailable = new QLabel( "Available GIDs: 0" );
    _labelSelection = new QLabel( "Selected GIDs: 0" );

    layoutSelection->addWidget( _labelAvailable, 0, 0, 1, 1 );
    layoutSelection->addWidget( _labelSelection, 0, 2, 1, 1 );

    _listViewAvailable = new QListView( );
    _listViewAvailable->setSelectionMode( QAbstractItemView::ExtendedSelection );
    _listViewAvailable->setUniformItemSizes( true );

    _listViewSelected = new QListView( );
    _listViewSelected->setSelectionMode( QAbstractItemView::ExtendedSelection );
    _listViewSelected->setUniformItemSizes( true );

    _modelAvailable = new QStandardItemModel( );
    _modelSelected = new QStandardItemModel( );

    _listViewAvailable->setModel( _modelAvailable );
    _listViewSelected->setModel( _modelSelected );

    _buttonAddToSelection = new QPushButton(QIcon(":/icons/right.svg"), "");
    _buttonAddToSelection->setToolTip( tr( "Add to selected GIDs" ));

    _buttonRemoveFromSelection = new QPushButton(QIcon(":/icons/left.svg"), "");
    _buttonRemoveFromSelection->setToolTip( tr( "Remove from selected GIDs" ));

    layoutSelection->addWidget( _listViewAvailable, 1, 0, 5, 1 );
    layoutSelection->addWidget( _buttonAddToSelection, 2, 1, 1, 1 );
    layoutSelection->addWidget( _buttonRemoveFromSelection, 4, 1, 1, 1 );
    layoutSelection->addWidget( _listViewSelected, 1, 2, 5, 1 );
    layoutSelection->addWidget( new QLabel("Selected:"), 6,0,1,1);

    QRegExp rx("[0-9 \\-\\,]+");
    auto validator = new QRegExpValidator(rx, this);

    _rangeEdit = new QLineEdit();
    _rangeEdit->setPlaceholderText("Nothing selected");
    _rangeEdit->setValidator(validator);

    auto rangeLayout = new QHBoxLayout();
    rangeLayout->addWidget(new QLabel("Selected:"), 0);
    rangeLayout->addWidget(_rangeEdit, 1);

    layoutSelection->addLayout(rangeLayout, 6, 0, 1, 4);

    connect(_rangeEdit, SIGNAL(editingFinished()),
            this,       SLOT(_updateRangeEdit()));

    connect(_listViewAvailable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection&)),
            this,                                 SLOT(_updateRangeEdit()));

    connect( _buttonAddToSelection, SIGNAL( clicked( void )),
             this, SLOT( _addToSelected( void )));

    connect( _buttonRemoveFromSelection, SIGNAL( clicked( void )),
             this, SLOT( _removeFromSelected( void )));

    _tabWidget->addTab( containerSelection, "Selection" );
  }

  void SelectionManagerWidget::_initTabExport( void )
  {
    auto containerExport = new QWidget( );
    auto layoutExport = new QGridLayout( );
    containerExport->setLayout( layoutExport );

    _pathExportDefault = QDir::currentPath();
    _lineEditFilePath = new QLineEdit( _pathExportDefault.append( "/output.txt"));

    _lineEditPrefix = new QLineEdit( );
    _lineEditSuffix = new QLineEdit( );
    _lineEditSeparator = new QLineEdit( );

    _radioNewLine = new QRadioButton( "New line" );
    _radioSpace = new QRadioButton( "Space" );
    _radioTab = new QRadioButton( "Tab" );
    _radioOther = new QRadioButton( "Other:" );

    _buttonBrowse = new QPushButton( "Browse..." );
    _buttonSave = new QPushButton( "Save" );

    auto groupBoxPrefix = new QGroupBox( "Prefix/Suffix" );
    auto layoutPrefix = new QGridLayout( );
    groupBoxPrefix->setLayout( layoutPrefix );
    layoutPrefix->addWidget( new QLabel( "Prefix:" ), 0, 0, 1, 1 );
    layoutPrefix->addWidget( _lineEditPrefix, 0, 1, 1, 1 );
    layoutPrefix->addWidget( new QLabel( "Suffix:" ), 1, 0, 1, 1 );
    layoutPrefix->addWidget( _lineEditSuffix, 1, 1, 1, 1 );

    auto groupBoxSeparator = new QGroupBox( "Separator" );
    auto layoutSeparator = new QGridLayout( );
    layoutSeparator->addWidget( _radioNewLine, 0, 0, 1, 2 );
    layoutSeparator->addWidget( _radioSpace, 1, 0, 1, 2 );
    layoutSeparator->addWidget( _radioTab, 2, 0, 1, 2 );
    layoutSeparator->addWidget( _radioOther, 3, 0, 1, 1 );
    layoutSeparator->addWidget( _lineEditSeparator, 3, 1, 1, 1 );

    _radioNewLine->setChecked( true );

    groupBoxSeparator->setLayout( layoutSeparator );

    layoutExport->addWidget( new QLabel( "File path:"), 0, 0, 1, 1 );
    layoutExport->addWidget( _lineEditFilePath, 0, 1, 1, 4 );
    layoutExport->addWidget( _buttonBrowse, 0, 5, 1, 1 );
    layoutExport->addWidget( groupBoxPrefix, 1, 0, 4, 2 );
    layoutExport->addWidget( groupBoxSeparator, 1, 2, 4, 3 );
    layoutExport->addWidget( _buttonSave, 4, 5, 1, 1 );

    connect( _buttonBrowse, SIGNAL( clicked( void )),
             this, SLOT( _buttonBrowseClicked( void )));

    connect( _buttonSave, SIGNAL( clicked( void )),
             this, SLOT( _buttonSaveClicked( void )));

    _tabWidget->addTab( containerExport, "Export" );
  }

  void SelectionManagerWidget::setGIDs( const TGIDSet& all_,
                                        const TGIDUSet& selected_ )
  {
    if(all_.size() == _gidsAll.size()) return;

    _gidsAll.clear( );
    _gidsAll.insert( all_.begin( ), all_.end( ));

    _fillLists( );

    clearSelection();
    setSelected( selected_ );
  }

  const TGIDUSet& SelectionManagerWidget::selected( void ) const
  {
    return _gidsSelected;
  }

  void SelectionManagerWidget::clearSelection( void )
  {
    _gidsSelected.clear( );
    _gidsAvailable = _gidsAll;

    _reloadLists( );
  }

  void SelectionManagerWidget::setSelected( const TGIDUSet& selected_ )
  {
    if( selected_ == _gidsSelected && !_gidsAvailable.empty( ))
      return;

    _gidsSelected = selected_;

    _gidsAvailable.clear( );
    for( auto gid : _gidsAll )
    {
      if( _gidsSelected.find( gid ) == _gidsSelected.end( ))
        _gidsAvailable.insert( gid );
    }

    _reloadLists( );
  }

  void SelectionManagerWidget::_fillLists( void )
  {
    _modelAvailable->clear( );
    _modelSelected->clear( );

    unsigned int index = 0;
    GIDVec gids( _gidsAll.size( ));
    std::copy( _gidsAll.begin( ), _gidsAll.end( ), gids.begin( ));
    std::sort( gids.begin( ), gids.end( ));

    QList<QStandardItem *> available, selected;
    for( auto gid : gids )
    {
      const auto text = QString::number(gid);
      auto idx = new QStandardItem(text);
      idx->setData(gid);

      available << idx;
      selected << new QStandardItem(text);

      _gidIndex.insert( std::make_pair( gid, index ));

      ++index;
    }

    _modelAvailable->insertColumn(0, available );
    _modelSelected->insertColumn(0, selected );
  }

  void SelectionManagerWidget::_reloadLists( void )
  {
    for( auto gid : _gidsAll )
    {
      bool stateSelected = ( _gidsSelected.find( gid ) != _gidsSelected.end( ));

      auto gidIndex = _gidIndex.find( gid );
      assert( gidIndex != _gidIndex.end( ));

      unsigned int row = gidIndex->second;

      _listViewAvailable->setRowHidden( row, stateSelected );
      _listViewSelected->setRowHidden( row, !stateSelected );
    }

    _updateListsLabelNumbers( );
  }

  void SelectionManagerWidget::_updateListsLabelNumbers( void )
  {
    _labelAvailable->setText( QString( "Available GIDs: ") + QString::number( _gidsAvailable.size( )));
    _labelSelection->setText( QString( "Selected GIDs: ") + QString::number( _gidsSelected.size( )));

    _buttonRemoveFromSelection->setEnabled(!_gidsSelected.empty());
  }

  void SelectionManagerWidget::_addToSelected( void )
  {
    const auto selectedIndices =
        _listViewAvailable->selectionModel( )->selectedIndexes( );

    if( selectedIndices.size( ) <= 0 )
      return;

    for( auto index : selectedIndices )
    {
      if( index.row( ) < 0 )
        continue;

      const auto item = _modelAvailable->itemFromIndex( index );

      bool ok;
      unsigned int gid = item->data( Qt::DisplayRole ).toUInt( &ok );

      _gidsAvailable.erase( gid );
      _gidsSelected.insert( gid );

      const auto gidIndex = _gidIndex.find( gid );
      assert( gidIndex != _gidIndex.end( ));

      _listViewAvailable->setRowHidden( gidIndex->second, true );
      _listViewSelected->setRowHidden( gidIndex->second, false );
    }

    _listViewAvailable->selectionModel( )->clearSelection( );
    _rangeEdit->clear();

    _updateListsLabelNumbers( );
  }

  void SelectionManagerWidget::_removeFromSelected( void )
  {
    auto selectedIndices =
          _listViewSelected->selectionModel( )->selectedIndexes( );

    for( auto index : selectedIndices )
    {
      if( index.row( ) < 0 )
        continue;

      auto item = _modelSelected->itemFromIndex( index );
      unsigned int gid = item->data( Qt::DisplayRole ).toUInt( );

      _gidsSelected.erase( gid );
      _gidsAvailable.insert( gid );

      auto gidIndex = _gidIndex.find( gid );
      assert( gidIndex != _gidIndex.end( ));

      _listViewAvailable->setRowHidden( gidIndex->second, false );
      _listViewSelected->setRowHidden( gidIndex->second, true );
    }

    _listViewSelected->selectionModel( )->clearSelection( );

    _updateListsLabelNumbers( );
  }

  void SelectionManagerWidget::_updateRangeEdit()
  {
    auto lEdit = qobject_cast<QLineEdit*>(sender());
    if(lEdit)
    {
      const auto parts = _rangeEdit->text().split(',');
      const auto model = _listViewAvailable->model();
      QItemSelection selection;

      auto processPart = [&model, &selection, this](const QString &s)
      {
        if(s.isEmpty()) return true;
        const auto trim = s.trimmed();
        bool ok = false;

        if(trim.contains("-"))
        {
          const auto values = trim.split("-");
          if(values.size() > 2) return false;
          const auto min = values.first().trimmed().toULongLong(&ok);
          if(!ok) return false;
          const auto max = values.last().trimmed().toULongLong(&ok);
          if(!ok || max < min) return false;
          const auto minIt = _gidIndex.find(min);
          const auto maxIt = _gidIndex.find(max);
          if(minIt == _gidIndex.end() || maxIt == _gidIndex.end()) return false;

          const auto range = QItemSelectionRange(model->index((*minIt).second, 0), model->index((*maxIt).second, 0));
          selection.append(range);
          return true;
        }

        // not a range
        const auto value = trim.toULongLong(&ok);
        if(!ok) return false;
        const auto it = _gidIndex.find(value);
        if(it == _gidIndex.end()) return false;
        const auto item = model->index((*it).second, 0);
        selection.append(QItemSelectionRange(item, item));
        return true;
      };

      auto it = std::find_if_not(parts.cbegin(), parts.cend(), processPart);
      if(it != parts.cend())
      {
        lEdit->setStyleSheet("QLineEdit { background: rgb(255, 160, 160); selection-background-color: rgb(0, 200, 200); }");
      }
      else
      {
        lEdit->setStyleSheet("");
      }

      if(!selection.isEmpty())
      {
        auto sModel = _listViewAvailable->selectionModel();
        sModel->blockSignals(true);
        sModel->select(selection, QItemSelectionModel::ClearAndSelect);
        sModel->blockSignals(false);
        _listViewAvailable->scrollTo(selection.last().bottomRight(), QAbstractItemView::EnsureVisible);
        _listViewAvailable->repaint();
      }
    }
    else
    {
      auto sModel = qobject_cast<QItemSelectionModel*>(sender());
      if(sModel)
      {
        const auto selection = sModel->selection();
        std::vector<Range> ranges;

        auto processRange = [&ranges](const QItemSelectionRange &r)
        {
          bool ok = false;
          const auto min = r.topLeft();
          if(!min.isValid()) return false;
          const auto minData = min.data(Qt::UserRole + 1);
          if(!minData.isValid()) return false;
          const auto minValue = minData.toULongLong(&ok);
          if(!ok) return false;

          const auto max = r.bottomRight();
          if(!max.isValid()) return false;
          if(min != max)
          {
            const auto maxData = max.data(Qt::UserRole + 1);
            if(!maxData.isValid()) return false;
            const auto maxValue = maxData.toULongLong(&ok);
            if(!ok) return false;

            ranges.emplace_back(minValue, maxValue);
            return true;
          }

          ranges.emplace_back(minValue, minValue);
          return true;
        };
        auto it = std::find_if_not(selection.cbegin(), selection.cend(), processRange);
        if(it != selection.cend() || ranges.empty()) return;

        ranges = mergeRanges(ranges);

        _rangeEdit->setStyleSheet("");
        _rangeEdit->blockSignals(true);
        _rangeEdit->setText(printRanges(ranges));
        _rangeEdit->blockSignals(false);
        _rangeEdit->repaint();
      }
      else
      {
        std::cerr << "Unable to indentify sender! " << __FILE__ << "," << __LINE__ << std::endl;
      }
    }
  }

  void SelectionManagerWidget::_saveToFile( const QString& filePath,
                                            const QString& separator,
                                            const QString& prefix,
                                            const QString& suffix )
  {
    if(prefix.contains(separator) || suffix.contains(separator))
    {
      QMessageBox msgBox( this );
      msgBox.setWindowTitle("Selection save");
      msgBox.setText( "The prefix and the suffix cannot contain the separator character." );
      msgBox.setStandardButtons( QMessageBox::Ok );
      msgBox.setWindowIcon(QIcon(":/visimpl.png"));
      msgBox.exec( );

      return;
    }

    QFile file;
    if( file.exists( filePath ))
    {
      QMessageBox msgBox( this );
      msgBox.setWindowTitle("Selection save");
      msgBox.setText( "The selected file already exists." );
      msgBox.setWindowIcon(QIcon(":/visimpl.png"));
      msgBox.setInformativeText( "Do you want to overwrite?" );
      msgBox.setStandardButtons( QMessageBox::Save | QMessageBox::Cancel );
      msgBox.setDefaultButton( QMessageBox::Save );
      int ret = msgBox.exec( );

      if((( QMessageBox::StandardButton ) ret ) == QMessageBox::Cancel )
        return;
    }

    file.setFileName( filePath );
    file.open( QFile::WriteOnly | QFile::Truncate );
    QTextStream outStream( &file );

    GIDVec gids( _gidsSelected.size( ));
    std::copy( _gidsSelected.begin( ), _gidsSelected.end( ), gids.begin( ));
    std::sort( gids.begin( ), gids.end( ));

    for( auto &gid : gids )
    {
      outStream << prefix << gid << suffix << separator;
    }

    file.close( );

    QMessageBox::information( this, QString( "Save successful" ), QString( "Selection saved to file." ));
  }

  void SelectionManagerWidget::_buttonAcceptClicked( void )
  {
    this->close( );

    emit selectionChanged( );
  }

  void SelectionManagerWidget::_buttonCancelClicked( void )
  {
    this->close( );
  }

  void SelectionManagerWidget::_buttonBrowseClicked( void )
  {
    QString path = QFileDialog::getSaveFileName(
         this, tr( "Save to file" ), _lineEditFilePath->text( ),
         tr( "Text  files (.txt);;All files (*)" ),
         nullptr, QFileDialog::DontUseNativeDialog );

    if( !path.isEmpty( ))
      _lineEditFilePath->setText( path );
  }

  void SelectionManagerWidget::_buttonSaveClicked( void )
  {
    QString separator;
    if( _radioNewLine->isChecked( ))
      separator = "\n";
    else if( _radioSpace->isChecked( ))
      separator = " ";
    else if( _radioTab->isChecked( ))
      separator = "\t";
    else
      separator = _lineEditSeparator->text( );

    _saveToFile( _lineEditFilePath->text( ), separator,
                 _lineEditPrefix->text( ), _lineEditSuffix->text( ));
  }

  SelectionManagerWidget::Ranges SelectionManagerWidget::mergeRanges(
      SelectionManagerWidget::Ranges &r)
  {
    Ranges result;

    auto sortRanges = [](const Range &lhs, const Range &rhs)
    {
      // ranges dont overlap
      return lhs.min < rhs.min;
    };
    std::sort(r.begin(), r.end(), sortRanges);

    // join sorted ranges
    for(auto it = r.cbegin(); it != r.cend(); ++it)
    {
      if(result.empty())
      {
        result.emplace_back(*it);
        continue;
      }

      if(result.back().canMerge(*it))
      {
        result.back() = result.back() + (*it);
      }
      else
      {
        result.emplace_back(*it);
      }
    }

    return result;
  }

  QString SelectionManagerWidget::printRanges(const SelectionManagerWidget::Ranges &r)
  {
    QStringList texts;
    auto printRange = [&texts](const Range &item){ texts << item.print(); };
    std::for_each(r.cbegin(), r.cend(), printRange);

    return texts.join(',');
  }

}
