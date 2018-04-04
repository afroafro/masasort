#ifndef MASASORTMAINWINDOW_H
#define MASASORTMAINWINDOW_H

#include <QApplication>
#include <QtCharts>
//#include <WidgetIntanChart.h>
//#include <QWidgetSpikes.h>

using namespace QtCharts;

class QWidgetSpikes;
class WidgetIntanChart;

class masasortMainWindow : public QMainWindow {
    Q_OBJECT
public:
    masasortMainWindow(QWidget *parent=Q_NULLPTR, Qt::WindowFlags flags=Qt::WindowFlags());
    void openFile(QString="", QString="");
    bool getFileOpened();
    QVector<int>& getElectrodeArray();
    void setElectrodeArray(QVector<int>);
    QString getFileName();
    QString getFilePathName();
    QString getFileDir();
    void saveDigin0(QString);
    int getnChannel();
    QVector<QVector<double>>& getDatay();
    QVector<double>& getDatat();
    void changeElectrodeArray();

protected:
    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    WidgetIntanChart* widgetchart0;
    bool myConfirmDialog(QString);
    QWidget* widget;
    QGridLayout *mainLayout;

    // variables
    QString fileName;
    QString filePath;
    QString fileDir;
    QVector<qint32> t_amplifier_q;
    QVector<quint32> t_amplifier_qu;
    QVector<double> t_amplifier_db;
    QVector<QVector<quint16>> amplifier_data;
    QVector<QVector<QString>> nativeChannelNameraw;
    QVector<QVector<QString>> customChannelNameraw;
    QVector<QVector<QString>> nativeChannelName;
    QVector<QVector<QString>> customChannelName;
    QVector<QVector<quint16>> board_dig_in_data;
    int nChannel;

    QVector<QVector<double>> amplifier_data_db;
    QVector<QVector<int>> eventpnts;
    QVector<QVector<double>> eventamps;
    QVector<QVector<double>> eventpeakvals;
    double boardSampleRate;
    QVector<int> electrodearray;
    QVector<int> NNxConnectorarray;
    int Nelectrodearray;
    QVector<QVector<int>> electrodearray_raw;
    QVector<int> num_electrodearray_raw;
    QVector<QString> str_electrodearray_raw;
    int num_elec;
    QVector<QVector<QVector<double>>> events;

    bool fileopened;
    bool appbusy;

    QString threstype;
    double thresval;
    double paramspikeprepeak; // [ms]
    double paramspikepostpeak; // [ms]
    double parambaseprepeak; // [ms]
    double parambasepostpeak; // [ms]
    double spikeprepeak; // [pnt]
    double spikepostpeak; // [pnt]
    double baseprepeak; // [pnt]
    double basepostpeak; // [pnt]
    bool hasChname;
    double paramspikemergethres; // [ms]
    double spikemergethres; // [pnt]
    double paramrasterresolution; // [ms]
    double rasterresolution; // [s]
    int nChartButton;

    quint32 DATA_FILE_MAGIC_NUMBER;
    qint16 DATA_FILE_MAIN_VERSION_NUMBER;
    qint16 DATA_FILE_SECONDARY_VERSION_NUMBER;
    qint16 dspEnabled;
    double actualDspCutoffFreq;
    double actualLowerBandwidth;
    double actualUpperBandwidth;
    double desiredDspCutoffFreq;
    double desiredLowerBandwidth;
    double desiredUpperBandwidth;
    qint16 notchFilterComboBoxCurrentIndex;
    double desiredImpedanceFreq;
    double actualImpedanceFreq;
    QString note1LineEdit;
    QString note2LineEdit;
    QString note3LineEdit;
    //quint16 strlen;
    qint16 numTempSensors; // if !(saveTemp), then 0 (ver.1.1 addition)
    qint16 evalBoardMode; // (ver.1.3 addition)
    qint16 numSignalGroups;

    QString signalGroupName;
    QString strout;
    QString signalGroupPrefix;
    qint16 signalGroupEnabled;
    qint16 signalGroupNumChannels;
    qint16 signalGroupNumAmpChannels;

    qint16 nativeOrder;
    qint16 customOrder;
    qint16 signalType;
    qint16 channelEnabled;
    qint16 chipChannel;
    qint16 boardStream;
    qint16 voltageTriggerMode;
    qint16 digitalTriggerMode;
    qint16 voltageThreshold;
    qint16 digitalTriggerChannel;
    qint16 digitalEdgePolarity;
    qint16 analogTriggerChannel;
    qint16 analogEdgePolarity;
    double electrodeImpedanceMagnitude;
    double electrodeImpedancePhase;
    int filebytesize;
    bool openmasamode;

    int num_amplifier_channels;
    int num_aux_input_channels;
    int num_supply_voltage_channels;
    int num_temp_sensor_channels;
    int num_board_adc_channels;
    int num_board_dig_in_channels;
    int num_board_dig_out_channels;

    void saveVector(QVector<quint16>, QString="");
    void showConsecutiveFiles();

public slots:
    void pushFile(QAction*);
    void pushDisplay(QAction*);
};

#endif // MASASORTMAINWINDOW_H
