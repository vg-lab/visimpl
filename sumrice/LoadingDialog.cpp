/*
 * LoadingDialog.cpp
 *
 *  Created on: Sep 24, 2021
 *      Author: felix
 */

// Sumrice
#include <sumrice/LoadingDialog.h>

// QT
#include <QVBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include <QApplication>
#include <QMainWindow>
#include <QWindow>
#include <QDebug>

const QString NETWORK_STRING = QString("Network: %1");
const QString SPIKES_STRING  = QString("Spikes: %1");

LoadingDialog::LoadingDialog(QWidget *p, Qt::WindowFlags f)
: QDialog(p,f)
, m_progressBar{nullptr}
, m_networkLabel{nullptr}
, m_spikesLabel{nullptr}
{
  initializeGUI();

  adjustSize();
}

void LoadingDialog::setProgress(int value)
{
  const auto pValue = std::min(100, std::max(0, value));
  m_progressBar->setValue(pValue);
}

void LoadingDialog::setNetwork(unsigned int value)
{
  if(value > 0) m_networkLabel->setText(NETWORK_STRING.arg(value));
}

void LoadingDialog::setSpikesValue(unsigned int value)
{
  if(value > 0) m_spikesLabel->setText(SPIKES_STRING.arg(value));
}

void LoadingDialog::initializeGUI()
{
  auto layout = new QVBoxLayout();

  m_progressBar = new QProgressBar(this);
  m_progressBar->setValue(0);
  layout->addWidget(m_progressBar);

  m_networkLabel = new QLabel(NETWORK_STRING.arg(0), this);
  layout->addWidget(m_networkLabel);

  m_spikesLabel = new QLabel(SPIKES_STRING.arg(0), this);
  layout->addWidget(m_spikesLabel);

  setLayout(layout);

  setFixedWidth(300);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}
