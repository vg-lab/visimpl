/*
 * LoadingDialog.h
 *
 *  Created on: Sep 24, 2021
 *      Author: felix
 */

#ifndef SUMRICE_LOADINGDIALOG_H_
#define SUMRICE_LOADINGDIALOG_H_

// Sumrice
#include <sumrice/api.h>

// Qt
#include <QDialog>

class QLabel;
class QProgressBar;

/** \class LoadingDialog
 * \brief Dialog that shows data loading progress.
 *
 */
class SUMRICE_API LoadingDialog
: public QDialog
{
    Q_OBJECT
  public:
    /** \brief LoadingDialog class constructor.
     *
     */
    explicit LoadingDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    /** \brief LoadingDialog class virtual destructor.
     *
     */
    virtual ~LoadingDialog()
    {}

  public slots:
    /** \brief Updates progress value.
     * \param[in] value Value in [0,100].
     *
     */
    void setProgress(int value);

    /** \brief Updates network value.
     * \param[in] value Network values read.
     *
     */
    void setNetwork(unsigned int value);

    /** \brief Updates spikes value.
     * \param[in] value Spikes value read.
     *
     */
    void setSpikesValue(unsigned int value);

  private:
    /** \brief Helper method to initialize the gui elements.
     *
     */
    void initializeGUI();

    QProgressBar *m_progressBar;  /** dialog progress bar.    */
    QLabel       *m_networkLabel; /** network ids read label. */
    QLabel       *m_spikesLabel;  /** spikes ids read label.  */
};

#endif /* SUMRICE_LOADINGDIALOG_H_ */
