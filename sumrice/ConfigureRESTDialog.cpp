/*
 * Copyright (c) 2022-2022 VG-Lab/URJC.
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

// Sumrice
#include <sumrice/ConfigureRESTDialog.h>

// Qt
#include <QSpinBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>

//-----------------------------------------------------------------------------
RESTConfigurationWidget::RESTConfigurationWidget(QWidget *p, Qt::WindowFlags f)
: QWidget(p,f)
{
  m_waitTime = new QSpinBox();
  m_waitTime->setMinimum(1);
  m_waitTime->setMaximum(10000);
  m_waitTime->setValue(5000);
  m_waitTime->setSuffix(" ms");

  m_spikesSize = new QSpinBox();
  m_spikesSize->setMinimum(100);
  m_spikesSize->setMaximum(10000);
  m_spikesSize->setValue(1000);
  m_spikesSize->setSuffix(" spikes");

  auto layout = new QGridLayout();
  layout->addWidget(new QLabel("Time between requests:"), 0, 0);
  layout->addWidget(m_waitTime, 0, 1);
  layout->addWidget(new QLabel("Request size:"), 2, 0);
  layout->addWidget(m_spikesSize, 2, 1);

  setLayout(layout);
  layout->setColumnStretch(0,0);
  layout->setColumnStretch(1,1);
}

//-----------------------------------------------------------------------------
void RESTConfigurationWidget::setOptions(const Options &o)
{
  m_waitTime->setValue(o.waitTime);
  m_spikesSize->setValue(o.spikesSize);
}

//-----------------------------------------------------------------------------
RESTConfigurationWidget::Options RESTConfigurationWidget::getOptions() const
{
  Options result;
  result.waitTime = m_waitTime->value();
  result.spikesSize = m_spikesSize->value();

  return result;
}

//-----------------------------------------------------------------------------
ConfigureRESTDialog::ConfigureRESTDialog(QWidget *p, Qt::WindowFlags f, const RESTConfigurationWidget::Options &o)
: QDialog(p,f)
{
  auto layout = new QVBoxLayout();
  setLayout(layout);

  m_options = new RESTConfigurationWidget(this);
  m_options->setOptions(o);

  auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  layout->addWidget(m_options, 1);
  layout->addWidget(buttons,0);

  setSizeGripEnabled(false);
  setModal(true);
  setFixedWidth(440);
}

//-----------------------------------------------------------------------------
RESTConfigurationWidget::Options ConfigureRESTDialog::getRESTOptions() const
{
  return m_options->getOptions();
}

//-----------------------------------------------------------------------------
void ConfigureRESTDialog::setRESTOptions(const RESTConfigurationWidget::Options &o)
{
  m_options->setOptions(o);
}
