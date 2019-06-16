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

#include <unordered_set>

#include "types.h"

namespace visimpl
{

  class SelectionManagerWidget : public QWidget
  {
    Q_OBJECT;

  public:

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

    void addToSelected( void );
    void removeFromSelected( void );

    void saveToFile( const std::string& filePath,
                     const std::string& separator = "\n",
                     const std::string& prefix = "",
                     const std::string& suffix = "" );

    void cancelClicked( void );
    void acceptClicked( void );


  protected:

    void _fillLists( void );
    void _reloadLists( void );

    QListView* _listViewAvailable;
    QListView* _listViewSelected;

    QStandardItemModel* _modelAvailable;
    QStandardItemModel* _modelSelected;

    QPushButton* _buttonAddToSelection;
    QPushButton* _buttonRemoveFromSelection;

    QLabel* _labelAvailable;
    QLabel* _labelSelection;

    TGIDUSet _gidsSelected;
    TGIDUSet _gidsAvailable;
    TGIDUSet _gidsAll;

    TUIntUintMap _gidIndex;

  };

}



#endif /* SELECTIONMANAGERWIDGET_H_ */
