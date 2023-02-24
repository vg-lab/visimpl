//
// Created by grial on 22/02/23.
//

#include "ReconnectRESTDialog.h"

#include <QVBoxLayout>
#include <QSplitter>
#include <QDialogButtonBox>

ReconnectRESTDialog::ReconnectRESTDialog( QWidget* parent , Qt::WindowFlags f )
  : QDialog( parent , f )
{

  _spikes = new QRadioButton( "Reload spikes" );
  _both = new QRadioButton( "Reload network and spikes" );
  _newConnection = new QRadioButton( "Create a new connection" );

  _spikes->setChecked( true );

  auto* layout = new QVBoxLayout( );

  layout->addWidget( _spikes );
  layout->addWidget( _both );
  layout->addWidget( _newConnection );

  auto buttons = new QDialogButtonBox(
    QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  connect( buttons , &QDialogButtonBox::accepted , this , &QDialog::accept );
  connect( buttons , &QDialogButtonBox::rejected , this , &QDialog::reject );
  layout->addWidget( buttons , 0 );

  setLayout( layout );
  setSizeGripEnabled( false );
  setModal( true );
}

ReconnectRESTDialog::Selection ReconnectRESTDialog::getSelection( ) const
{
  if ( _spikes->isChecked( )) return Selection::SPIKES;
  if ( _both->isChecked( )) return Selection::SPIKES_AND_NETWORK;
  return Selection::NEW_CONNECTION;
}