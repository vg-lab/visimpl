/*
 * Copyright (c) 2015-2022 VG-Lab/URJC.
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

// Project
#include <sumrice/CloseDataDialog.h>

// Qt
#include <QGroupBox>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QDialogButtonBox>

//-----------------------------------------------------------------------------
CloseDataDialog::CloseDataDialog(QWidget *p, Qt::WindowFlags f)
: QDialog(p,f)
{
  auto layout = new QVBoxLayout();

  auto groupBox = new QGroupBox(tr("Close dataset options:"), this);
  auto groupLayout = new QVBoxLayout(groupBox);
  groupBox->setLayout(groupLayout);

  m_keep = new QRadioButton(tr("Reset spike data and keep network data and settings."), groupBox);
  m_keep->setAutoExclusive(true);

  auto rButton = new QRadioButton(tr("Fully close dataset."), groupBox);
  rButton->setChecked(true);
  rButton->setAutoExclusive(true);

  groupLayout->addWidget(m_keep,0);
  groupLayout->addWidget(rButton,0);

  auto buttons = new QDialogButtonBox(this);
  buttons->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
  connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

  layout->addWidget(groupBox, 0);
  layout->addWidget(buttons, 0);

  setLayout(layout);

  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  setWindowTitle(tr("Close dataset"));
  setSizeGripEnabled(false);
  setModal(true);
}

//-----------------------------------------------------------------------------
bool CloseDataDialog::keepNetwork() const
{
  return m_keep->isChecked();
}

//-----------------------------------------------------------------------------
void CloseDataDialog::showEvent(QShowEvent *e)
{
  QDialog::showEvent(e);

  setFixedSize(size());
}
