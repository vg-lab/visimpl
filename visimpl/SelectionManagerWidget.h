/*
 * @file	SelectionManagerWidget.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#ifndef SELECTIONMANAGERWIDGET_H_
#define SELECTIONMANAGERWIDGET_H_

#include <QListView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLineEdit>
#include <QRadioButton>

#include <unordered_set>

#include "types.h"

namespace visimpl
{

  class SelectionManagerWidget : public QWidget
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

    SelectionManagerWidget( QWidget* parent = 0 );
    ~SelectionManagerWidget( void );

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
