/*
 * @file	SubsetManager.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#ifndef SUBSETIMPORTER_H_
#define SUBSETIMPORTER_H_

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <simil/simil.h>

namespace visimpl
{

  class SubsetImporter : public QWidget
  {

    Q_OBJECT

  public:

    SubsetImporter( QWidget* parent_ = nullptr );
    ~SubsetImporter( );

    void init( void );

    void reload( const simil::SubsetEventManager* );
    void clear( void );

    const std::vector< std::string > selectedSubsets( void ) const;

  signals:

    void clickedAccept( void );
    void clickedClose( void );

  protected slots:

    void closeDialog( void );

  protected:

    const simil::SubsetEventManager* _subsetEventManager;

    enum TSubsetLine { sl_container = 0, sl_layout, sl_checkbox, sl_label };
    typedef std::tuple< QWidget*, QGridLayout*, QCheckBox*, QLabel* > tSubsetLine;

    QPushButton* _buttonAccept;
    QPushButton* _buttonCancel;

    QVBoxLayout* _layoutSubsets;

    std::map< std::string, tSubsetLine > _subsets;

  };


}



#endif /* SUBSETIMPORTER_H_ */
