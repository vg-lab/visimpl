/*
 * ConnectRESTDialog.cpp
 *
 *  Created on: May 5, 2022
 *      Author: felix
 */

// Sumrice
#include <sumrice/ConnectRESTDialog.h>

// Qt
#include <QComboBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QDialogButtonBox>

const QStringList PROTOCOLS = { "NEST", "ARBOR" };

//-----------------------------------------------------------------------------
ConnectRESTDialog::ConnectRESTDialog(QWidget *p, Qt::WindowFlags f)
: QDialog(p,f)
{
  auto layout = new QVBoxLayout();
  setLayout(layout);

  Connection connection;

  m_protocol = new QComboBox();
  m_protocol->addItems(PROTOCOLS);
  m_protocol->setCurrentIndex(connection.protocol.compare("NEST", Qt::CaseInsensitive) == 0 ? 0 : 1);

  // @felix Enable when ARBOR plugin has been tested.
  m_protocol->setEnabled(false);

  m_url = new QLineEdit();
  m_url->setText(connection.url);

  m_port = new QLineEdit();
  m_port->setText(QString::number(connection.port));

  auto glo = new QGridLayout();
  glo->addWidget(new QLabel("Protocol: "), 0, 0);
  glo->addWidget(m_protocol, 0, 1);
  glo->addWidget(new QLabel("IP address: "), 1, 0);
  glo->addWidget(m_url, 1, 1);
  glo->addWidget(new QLabel("Port: "), 2, 0);
  glo->addWidget(m_port, 2, 1);

  layout->addLayout(glo,1);

  m_options = new RESTConfigurationWidget(this);

  auto group = new QGroupBox("REST Options");
  group->setLayout(new QVBoxLayout());
  group->layout()->addWidget(m_options);

  layout->addWidget(group, 1);

  auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  layout->addWidget(buttons,0);

  setSizeGripEnabled(false);
  setModal(true);
  setFixedWidth(440);
}

//-----------------------------------------------------------------------------
void ConnectRESTDialog::setRESTConnection(const Connection &r)
{
  m_protocol->setCurrentIndex(r.protocol == "NEST" ? 0 : 1);
  m_url->setText(r.url);
  m_port->setText(QString::number(r.port));
}

//-----------------------------------------------------------------------------
ConnectRESTDialog::Connection ConnectRESTDialog::getRESTConnection() const
{
  Connection result;
  result.protocol = m_protocol->currentIndex() == 0 ? "NEST" : "ARBOR";
  result.url = m_url->text();
  result.port = m_port->text().toInt();

  return result;
}

//-----------------------------------------------------------------------------
void ConnectRESTDialog::setRESTOptions(const RESTConfigurationWidget::Options &o)
{
  m_options->setOptions(o);
}

//-----------------------------------------------------------------------------
RESTConfigurationWidget::Options ConnectRESTDialog::getRESTOptions() const
{
  return m_options->getOptions();
}
