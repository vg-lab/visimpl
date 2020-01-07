#ifndef DATAINSPECTOR_H
#define DATAINSPECTOR_H


#include <QWidget>
#include <QGroupBox>
#include <QLabel>

#include <simil/SimulationPlayer.h>

class DataInspector: public QGroupBox
{
    Q_OBJECT
public:
    DataInspector(const QString &title, QWidget *parent = nullptr);

    void addWidget(QWidget *widget, int row, int column,
                   int rowSpan, int columnSpan,
                   Qt::Alignment alignment = Qt::Alignment());

    void setSimPlayer(simil::SimulationPlayer * simPlayer_);

signals:

    void simDataChanged( void );

protected:

    virtual void paintEvent(QPaintEvent *event);


    unsigned int _gidsize;
    unsigned int _spikesize;
    QLabel * _labelGIDs;
    QLabel *_labelSpikes;
    QLabel *_labelStartTime;
    QLabel *_labelEndTime;
    simil::SimulationPlayer * _simPlayer;
};

#endif // DATAINSPECTOR_H
