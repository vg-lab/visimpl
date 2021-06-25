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
#include "DisplayManagerWidget.h"

// Qt
#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>

using namespace visimpl;

DisplayManagerWidget::DisplayManagerWidget( )
: _eventData( nullptr )
, _histData( nullptr )
, _eventsLayout( nullptr )
, _histogramsLayout( nullptr )
, _dirtyFlagEvents( true )
, _dirtyFlagHistograms( true )
{
}

void DisplayManagerWidget::init(  const std::vector< visimpl::EventWidget* >* eventData,
                                  const std::vector< visimpl::HistogramWidget* >* histData )
{
  setMinimumWidth( 500 );

  setWindowTitle( "Visibility manager" );
  setWindowIcon(QIcon(":/visimpl.png"));

  _eventData = eventData;
  _histData = histData;

  const QStringList eventHeaders = { "Name", "Show", "Delete" };
  const QStringList histoHeaders = { "Name", "Size", "Show", "Delete" };

  auto globalLayout = new QGridLayout( );

  // Events
  _eventsLayout = new QGridLayout( );
  _eventsLayout->setAlignment( Qt::AlignTop );

  auto eventScrollContainer = new QWidget( );
  eventScrollContainer->setLayout( _eventsLayout );

  auto eventScroll = new QScrollArea( );
  eventScroll->setWidget( eventScrollContainer );
  eventScroll->setWidgetResizable( true );
  eventScroll->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

  auto eventGroup = new QGroupBox( );
  eventGroup->setTitle( "Events" );
  eventGroup->setMinimumHeight( 300 );
  eventGroup->setMaximumHeight( 300 );
  eventGroup->setLayout( new QVBoxLayout( ));

  auto headerEventLayout = new QGridLayout( );
  headerEventLayout->addWidget( new QLabel( "Name" ), 0, 0, 1, 2);
  headerEventLayout->addWidget( new QLabel( "Visible" ), 0, 2, 1, 1);
  headerEventLayout->addWidget( new QLabel( "Delete" ), 0, 3, 1, 1);

  auto eventHeader = new QWidget( );
  eventHeader->setLayout( headerEventLayout );

  eventGroup->layout( )->addWidget( eventHeader );
  eventGroup->layout( )->addWidget( eventScroll );

  // Histograms
  _histogramsLayout = new QGridLayout( );
  _histogramsLayout->setAlignment( Qt::AlignTop );

  auto histoScrollContainer = new QWidget( );
  histoScrollContainer->setLayout( _histogramsLayout );

  auto histoScroll = new QScrollArea( );
  histoScroll->setWidget( histoScrollContainer );
  histoScroll->setWidgetResizable( true );
  histoScroll->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

  auto histGroup = new QGroupBox( );
  histGroup->setTitle( "Histograms" );
  histGroup->setMinimumHeight( 300 );
  histGroup->setMaximumHeight( 300 );
  histGroup->setLayout(  new QVBoxLayout( ) );

  auto headerHistoLayout = new QGridLayout( );
  headerHistoLayout->addWidget( new QLabel( "Name" ), 0, 0, 1, 2);
  headerHistoLayout->addWidget( new QLabel( "Size" ), 0, 2, 1, 1);
  headerHistoLayout->addWidget( new QLabel( "Visible" ), 0, 3, 1, 1);
  headerHistoLayout->addWidget( new QLabel( "Delete" ), 0, 4, 1, 1);

  auto histoHeader = new QWidget( );
  histoHeader->setLayout( headerHistoLayout );

  histGroup->layout( )->addWidget( histoHeader );
  histGroup->layout( )->addWidget( histoScroll );

  auto closeButton = new QPushButton( "Close", this );
  connect( closeButton, SIGNAL( clicked( )), this, SLOT( close( )));

  globalLayout->addWidget( eventGroup, 0, 0, 5, 5 );
  globalLayout->addWidget( histGroup, 5, 0, 5, 5 );
  globalLayout->addWidget( closeButton, 10, 2, 1, 1 );

  this->setLayout( globalLayout );
}

void DisplayManagerWidget::close( void )
{
  hide( );
}

void DisplayManagerWidget::dirtyEvents(void)
{
  _dirtyFlagEvents = true;
}

void DisplayManagerWidget::dirtyHistograms(void)
{
  _dirtyFlagHistograms = true;
}

void DisplayManagerWidget::clearWidgets(void)
{
  clearEventWidgets();

  clearHistogramWidgets();
}

void DisplayManagerWidget::clearEventWidgets(void)
{
  auto removeEventWidget = [this](TDisplayEventTuple &e)
  {
    auto container = std::get< TDM_E_CONTAINER >( e );
    _eventsLayout->removeWidget( container );

    delete container;
  };
  std::for_each(_events.begin(), _events.end(), removeEventWidget);

  QLayoutItem *item;
  while ((item = _eventsLayout->takeAt(0)) != nullptr)
  {
    delete item->widget();
    delete item;
  }

  _events.clear();
}

void DisplayManagerWidget::clearHistogramWidgets(void)
{
  auto removeHistogramWidget = [this](TDisplayHistogramTuple &e)
  {
    auto container = std::get< TDM_H_CONTAINER >( e );
    _histogramsLayout->removeWidget( container );

    delete container;
  };
  std::for_each(_histograms.begin(), _histograms.end(), removeHistogramWidget);

  QLayoutItem *item;
  while ((item = _histogramsLayout->takeAt(0)) != nullptr)
  {
    delete item->widget();
    delete item;
  }

  _histograms.clear();
}

void DisplayManagerWidget::refresh()
{
  if (_dirtyFlagEvents) refreshEvents();

  if (_dirtyFlagHistograms) refreshHistograms();
}

void DisplayManagerWidget::refreshEvents(void)
{
  clearEventWidgets();

  unsigned int row = 0;
  for (const auto &ev : *_eventData)
  {
    QWidget *container = new QWidget();
    container->setMaximumHeight(50);
    // Fill name
    QGridLayout *contLayout = new QGridLayout();
    container->setLayout(contLayout);

    QLabel *nameLabel = new QLabel(tr(ev->name().c_str()), container);
    QPushButton *hideButton = new QPushButton(container);
    hideButton->setIcon(QIcon(QPixmap(":icons/show.svg")));
    hideButton->setCheckable(true);
    hideButton->setChecked(true);
    hideButton->setWhatsThis("Click to show/hide the row in main view.");

    QPushButton *deleteButton = new QPushButton(container);
    deleteButton->setIcon(QIcon(QPixmap(":icons/trash.svg")));

    contLayout->addWidget(nameLabel, row, 0, 1, 2);
    contLayout->addWidget(hideButton, row, 2, 1, 1);
    contLayout->addWidget(deleteButton, row, 3, 1, 1);

    _eventsLayout->addWidget(container);

    if (row < _eventData->size() - 1)
    {
      QFrame *line = new QFrame(container);
      line->setFrameShape(QFrame::HLine);
      line->setFrameShadow(QFrame::Sunken);

      _eventsLayout->addWidget(line);
    }

    connect(hideButton, SIGNAL(clicked( )),
            this,       SLOT(hideEventClicked( )));

    connect(deleteButton, SIGNAL(clicked( )),
            this,         SLOT(deleteEventClicked( )));

    _events.push_back(std::make_tuple(container, nameLabel, hideButton, deleteButton));
    ++row;
  }

  _dirtyFlagEvents = false;
}

void DisplayManagerWidget::refreshHistograms(void)
{
  clearHistogramWidgets();

  unsigned int row = 0;
  for (const auto &hist : *_histData)
  {
    QWidget *container = new QWidget();
    container->setMaximumHeight(50);

    // Fill name
    QGridLayout *contLayout = new QGridLayout();
    container->setLayout(contLayout);

    QLabel *nameLabel = new QLabel(tr(hist->name().c_str()), container);

    QLabel *numberLabel = new QLabel(QString::number(hist->gidsSize()), container);

    QPushButton *hideButton = new QPushButton(container);
    hideButton->setIcon(QIcon(QPixmap(":icons/show.svg")));
    hideButton->setCheckable(true);
    hideButton->setChecked(true);
    hideButton->setWhatsThis("Click to show/hide the row in main view.");

    QPushButton *deleteButton = new QPushButton(container);
    deleteButton->setIcon(QIcon(QPixmap(":icons/trash.svg")));

    contLayout->addWidget(nameLabel, row, 0, 1, 2);
    contLayout->addWidget(numberLabel, row, 2, 1, 1);
    contLayout->addWidget(hideButton, row, 3, 1, 1);
    contLayout->addWidget(deleteButton, row, 4, 1, 1);

    if (row == 0) deleteButton->setEnabled(false);

    _histogramsLayout->addWidget(container);

    if (row < _histData->size() - 1)
    {
      QFrame *line = new QFrame(container);
      line->setFrameShape(QFrame::HLine);
      line->setFrameShadow(QFrame::Sunken);

      _histogramsLayout->addWidget(line);
    }

    connect(hideButton, SIGNAL(clicked( )),
            this,       SLOT(hideHistoClicked( )));

    connect(deleteButton, SIGNAL(clicked( )),
            this,         SLOT(deleteHistoClicked( )));

    _histograms.push_back(std::make_tuple(container, nameLabel, numberLabel, hideButton, deleteButton));
    ++row;
  }

  _dirtyFlagHistograms = false;
}

void DisplayManagerWidget::hideEventClicked()
{
  auto author = qobject_cast<QWidget*>(sender());
  if (!author) return;

  unsigned int counter = 0;
  for (auto t : _events)
  {
    auto container = std::get<TDM_E_CONTAINER>(t);

    if (author->parent() == container)
    {
      auto button = std::get<TDM_E_SHOW>(t);

      bool hidden = button->isChecked();

      button->setIcon(QIcon(hidden ? ":icons/show.svg" : ":icons/hide.svg"));

      emit(eventVisibilityChanged(counter, hidden));

      break;
    }

    ++counter;
  }
}

void DisplayManagerWidget::deleteEventClicked()
{
  auto author = qobject_cast<QWidget*>(sender());
  if (!author) return;

  unsigned int counter = 0;
  for (auto t : _events)
  {
    auto container = std::get<TDM_E_CONTAINER>(t);

    if (author->parent() == container)
    {

      emit(removeEvent(counter));
      _dirtyFlagEvents = true;

      refreshEvents();

      break;
    }

    ++counter;
  }
}

void DisplayManagerWidget::hideHistoClicked()
{
  auto author = qobject_cast<QWidget*>(sender());
  if (!author) return;

  unsigned int counter = 0;
  for (auto t : _histograms)
  {
    auto container = std::get<TDM_H_CONTAINER>(t);

    if (author->parent() == container)
    {
      auto button = std::get<TDM_H_SHOW>(t);

      bool hidden = button->isChecked();

      button->setIcon(QIcon(hidden ? ":icons/show.svg" : ":icons/hide.svg"));

      emit(subsetVisibilityChanged(counter, hidden));

      break;
    }

    ++counter;
  }
}

void DisplayManagerWidget::deleteHistoClicked()
{
  auto author = qobject_cast<QWidget*>(sender());
  if (!author) return;

  unsigned int counter = 0;
  for (auto t : _histograms)
  {
    auto container = std::get<TDM_H_CONTAINER>(t);

    if (author->parent() == container)
    {

      emit(removeHistogram(counter));
      _dirtyFlagHistograms = true;

      refreshHistograms();

      break;
    }

    ++counter;
  }
}



