#include <QApplication>
#include <QtCharts>
#include <WidgetIntanChart.h>
#include <QWidgetSpikes.h>
#include <QWidgetDisplay.h>
#include <masasortMainWindow.h>

using namespace std;
using namespace QtCharts;

masasortMainWindow::masasortMainWindow(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags){

    // parameters (these may not be working; look at WidgetIntanChart.cpp)
    fileopened = false;
    appbusy = false;
    threstype = "val";
    thresval = -100;
    paramspikeprepeak = 0.3; // [ms]
    paramspikepostpeak = 0.6; // [ms]
    parambaseprepeak = 1; // [ms]
    parambasepostpeak = 1.5; // [ms]
    hasChname = false;


    paramspikemergethres = 0.1; // [ms]
    paramrasterresolution = 1.0; // [ms]
    nChartButton = 8;
    openmasamode = false;
    /*/////////////////////////////////////*/

    // *** Create widget
    widget = new QWidget(this);

    Nelectrodearray = 4;
    electrodearray_raw.resize(Nelectrodearray);
    str_electrodearray_raw.resize(Nelectrodearray);
    num_electrodearray_raw.resize(Nelectrodearray);

    electrodearray_raw[0] = {13,14,15,16,12,11,10,9,8,7,6,5,1,2,3,4}; // default order of intan Ch
    electrodearray_raw[1] = {9,8,10,7,11,6,12,5,13,4,14,3,15,2,16,1}; // 1x16: #NNx pin in electrode shown in MAPPING from NeuroNexus mail // 1x16 (A015,A016,A014,A017,A013,A018,A012,A019,A008,A023,A009,A022,A010,A021,A011,A020)
    electrodearray_raw[2] = {4,2,7,5,3,1,8,6,12,10,15,13,11,9,16,14}; // tetrode: #NNx pin in electrode shown in MAPPING from NeuroNexus mail ** probably obsolete
    electrodearray_raw[3] = {17,16,10,23,18,9,24,15,8,25,19,7,26,14,6,27,20,5,28,13,4,29,21,3,30,12,2,31,22,1,32,11}; // tetrode: #NNx pin in electrode shown in MAPPING from NeuroNexus mail
    str_electrodearray_raw[0] = "default";
    str_electrodearray_raw[1] = "1x16";
    str_electrodearray_raw[2] = "2x8 tetrode";
    str_electrodearray_raw[3] = "A1x32-poly3";
    num_electrodearray_raw[0] = 16;
    num_electrodearray_raw[1] = 16;
    num_electrodearray_raw[2] = 16;
    num_electrodearray_raw[3] = 32;
    //electrodearray_raw[3] = {2,3,7,5,1,6,8,4,12,10,14,15,13,9,11,16}; // new tetrode #NNx pin in electrode shown in MAPPING from NeuroNexus mail

    mainLayout = new QGridLayout;
    QMenuBar *menuBar;
    menuBar = new QMenuBar;
    QMenu* fileMenu;
    QMenu* displayMenu;
    fileMenu = new QMenu("&File", widget);
    displayMenu = new QMenu("&Display", widget);
    menuBar->addMenu(fileMenu);
    menuBar->addMenu(displayMenu);
    int nfileaction = 10;
    QAction* fileAction[nfileaction];
    fileAction[0] = new QAction("&Open");
    fileAction[1] = new QAction("Open2");
    fileAction[2] = new QAction("&Save Data");
    fileAction[3] = new QAction("Save TutorData");
    fileAction[4] = new QAction("Save spike time");
    fileAction[5] = new QAction("Load Data");
    fileAction[6] = new QAction("Load TutorData");
    fileAction[7] = new QAction("Auto sort");
    fileAction[8] = new QAction("Auto extraction");
    fileAction[9] = new QAction("Delete TutorData");
    for (int ii=0; ii<nfileaction; ++ii){
        fileAction[ii]->setData(ii);
        fileMenu->addAction(fileAction[ii]);
    }
    int ndisplayaction = 4;
    QAction* displayAction[nfileaction];
    displayAction[0] = new QAction("Reorder Channels");
    displayAction[1] = new QAction("Show consecutive files");
    displayAction[2] = new QAction("Show variables");
    displayAction[3] = new QAction("Show sorted spikes");
    for (int ii=0; ii<ndisplayaction; ++ii){
        displayAction[ii]->setData(ii);
        displayMenu->addAction(displayAction[ii]);
    }
    mainLayout->setMenuBar(menuBar);
    connect(fileMenu, SIGNAL(triggered(QAction*)), this, SLOT(pushFile(QAction*)));
    connect(displayMenu, SIGNAL(triggered(QAction*)), this, SLOT(pushDisplay(QAction*)));
    widget->setLayout(mainLayout);
    this->setCentralWidget(widget);

    widgetchart0 = new WidgetIntanChart(this);


    // *** choose 16/32Ch
    // *** It might be fine with just using 32Ch, because null channels will be removed in OpenFile(), but you have to specify electrodes!
    if (myConfirmDialog("Open with 16Ch? (if not 32Ch)")){
        nChannel = 16;
        num_elec = 2;
        NNxConnectorarray = {13,14,15,16,12,11,10,9,8,7,6,5,1,2,3,4}; // RHD2132 16Ch vs CM16: #Ch of intan chip in each NNx connector pin (first NNx pin is 13th Ch in intan (A020 in 16Ch because it starts from A008)
    } else {
        nChannel = 32;
        num_elec = 3;
        NNxConnectorarray = {17,18,32,31,29,27,25,23,21,22,24,26,28,30,20,19,14,13,3,5,7,9,11,12,10,8,6,4,2,1,15,16}; // RHD2132 vs CM32: #Ch of intan chip in each NNx connector pin
    }
    // Change the order for subsequent calculation
    if (hasChname){
        electrodearray = electrodearray_raw[0];
    } else {
        electrodearray = electrodearray_raw[num_elec];
    }
    for (int ii=0;ii<electrodearray.length(); ++ii){
        for (int jj=0; jj<NNxConnectorarray.length(); ++jj){
            if (electrodearray[ii] == NNxConnectorarray[jj]){
                electrodearray[ii] = jj;
                break;
            }
        }
    }

    //QWidget* widgetchart = new QWidget(&window);
    //QGridLayout *chartLayout = new QGridLayout;

    //QWidget* widget2 = new QWidget(&window2);

    QWidget* widgetchartbuttons = new QWidget(this);

    // chart buttons
    QGridLayout *chartButtonLayout = new QGridLayout;
    QPushButton *chartButtons[nChartButton];
    chartButtons[0] = new QPushButton("top");
    QObject::connect(chartButtons[0], SIGNAL(released()), widgetchart0, SLOT(releasedChartButtonTop()));
    chartButtons[1] = new QPushButton("up");
    QObject::connect(chartButtons[1], SIGNAL(released()), widgetchart0, SLOT(releasedChartButtonUp()));
    chartButtons[2] = new QPushButton("down");
    QObject::connect(chartButtons[2], SIGNAL(released()), widgetchart0, SLOT(releasedChartButtonDown()));
    chartButtons[3] = new QPushButton("bottom");
    QObject::connect(chartButtons[3], SIGNAL(released()), widgetchart0, SLOT(releasedChartButtonBottom()));
    chartButtons[4] = new QPushButton("4");
    QObject::connect(chartButtons[4], SIGNAL(released()), widgetchart0, SLOT(releasedChartButton4()));
    chartButtons[5] = new QPushButton("8");
    QObject::connect(chartButtons[5], SIGNAL(released()), widgetchart0, SLOT(releasedChartButton8()));
    chartButtons[6] = new QPushButton("12");
    QObject::connect(chartButtons[6], SIGNAL(released()), widgetchart0, SLOT(releasedChartButton12()));
    chartButtons[7] = new QPushButton("16");
    QObject::connect(chartButtons[7], SIGNAL(released()), widgetchart0, SLOT(releasedChartButton16()));
    for (int ii=0; ii<nChartButton; ++ii){
        chartButtonLayout->addWidget(chartButtons[ii], ii, 0);
    }

    widgetchartbuttons->setLayout(chartButtonLayout);

    mainLayout->addWidget(widgetchartbuttons, 0, 0);
    mainLayout->addWidget(widgetchart0, 0, 1);

    // *** initialize widget
    widgetchart0->initParams();

}

void masasortMainWindow::closeEvent(QCloseEvent *event){
    if (!myConfirmDialog("Detected close event:\nAre you sure to close the window?")){
        event->ignore();
        return;
    } else {
        event->accept();
    }
}

int masasortMainWindow::getnChannel(){
    return nChannel;
}


void masasortMainWindow::pushFile(QAction *action){
    int value = action->data().toInt();
    if (value == 0){
        openmasamode = false;
        openFile();
    } else if (value == 1){
        openmasamode = true;
        openFile();
    } else if (value == 2){
        if (!fileName.isEmpty()){
            widgetchart0->saveData();
        }
    } else if (value == 3){
        if (!fileName.isEmpty()){
            widgetchart0->saveTutorData();
        }
    } else if (value == 4){
        if (!fileName.isEmpty()){
            widgetchart0->saveSpikeTime();
        }
    } else if (value == 5){
        widgetchart0->loadData();
    } else if (value == 6){
        widgetchart0->loadTutorData(false);
    } else if (value == 7){
        widgetchart0->runAutoTraining();
    } else if (value == 8){
        widgetchart0->runAutoExtraction();
    } else if (value == 9){
        widgetchart0->deleteTutorData();
    }
}

void masasortMainWindow::pushDisplay(QAction *action){
    int value = action->data().toInt();
    if (value == 0){
        //widgetchart0->changeElectrode();
        changeElectrodeArray();
    } else if (value == 1){
        showConsecutiveFiles();
    } else if (value == 2){ // show parameters
        double samprate = 1 / ((t_amplifier_db[t_amplifier_db.length()-1] - t_amplifier_db[0]) / (t_amplifier_db.length() - 1));
        QString outputstr = "boardSampleRate: " + QString::number(boardSampleRate);
        outputstr = outputstr + "\r\n" + "Real sample rate: " + QString::number(boardSampleRate);
        outputstr = outputstr + "\r\n" + "Electrode: " + str_electrodearray_raw[num_elec];
        QMessageBox::information(this, "Info", outputstr);
    } else if (value == 3){ // show sorted spikes
        widgetchart0->saveSpikes();
    }
}

QString masasortMainWindow::getFileName(){
    return fileName;
}

QString masasortMainWindow::getFilePathName(){
    return filePath;
}

void masasortMainWindow::showConsecutiveFiles(){

    QList<QUrl> fileurls = QFileDialog::getOpenFileUrls(this, "Open", QUrl(), "Select files to show (*.rhd);;All Files (*)");
    if (fileurls.isEmpty()){
        return;
    } else {
        QVector<QString> dialoglabel;
        QVector<QString> dialogvalue;
        QVector<QString> returnvalue;
        QString openchlist;
        // other potential params: stopband?
        dialoglabel.append("open ch (single ch num/digin0)");
        dialogvalue.append("digin0");
        returnvalue =  widgetchart0->myDialog("Open file name:", dialoglabel, dialogvalue);
        if (returnvalue.length() == 0){
            return;
        }
        openchlist = returnvalue[0];

        QMainWindow* showConsecutiveWindow = new QMainWindow(this);
        QWidgetDisplay* widgetDisplay = new QWidgetDisplay(showConsecutiveWindow);
        showConsecutiveWindow->setCentralWidget(widgetDisplay);
        showConsecutiveWindow->show();

        // show a wait bar
        QMainWindow* dialog = new QMainWindow(this);
        QWidget* dialogwidget = new QWidget(dialog);
        QGridLayout* dialogbox = new QGridLayout();
        QLabel* dialogstr0 = new QLabel(dialogwidget);
        dialogbox->addWidget(dialogstr0);
        dialogstr0->setText("Opening " + openchlist + " ...");
        dialogwidget->setLayout(dialogbox);
        dialogbox->setContentsMargins(10,10,10,10);
        dialog->setCentralWidget(dialogwidget);
        dialog->show();

        for (int ii=0; ii<fileurls.length(); ++ii){
            QString fileName = fileurls[ii].fileName();
            QString filePath = fileurls[ii].path();
            filePath.remove(0,1);

            dialogstr0->setText("Opening ... ( " + QString::number(ii+1) + " / " + QString::number(fileurls.length()) + " )\n" + fileName);
            qApp->processEvents(); // this is necessary for the GUI thread to repaint

            //QMessageBox::information(this, "!info", "ok");
            QStringList dirlists = filePath.split("/");
            QString saveDir = "";
            for (int jj=0; jj<dirlists.length()-1; ++jj){
                saveDir = saveDir + dirlists[jj] + "/";
            }
            QString saveName = fileName;
            saveName.chop(4);

            openFile(fileName, filePath);
            // widgetDisplay->setData(amplifier_data_db[openchlist.toInt()-1], t_amplifier_db);
        }
        dialog->close();
    }

}

QVector<QVector<double>>& masasortMainWindow::getDatay(){
    return amplifier_data_db;
}

QVector<double>& masasortMainWindow::getDatat(){
    return t_amplifier_db;
}

void masasortMainWindow::openFile(QString rhdfilename, QString rhdpathname){
    //QMessageBox::information(this, "!!", "pressed");
    if (appbusy){
        return;
    }

    QString fileNametmp;
    QString filePathtmp;

    if (!rhdfilename.isEmpty() && !rhdpathname.isEmpty()){
        fileNametmp = rhdfilename;
        filePathtmp = rhdpathname;
    } else {
        QUrl fileurl = QFileDialog::getOpenFileUrl(this, "Open", QUrl(filePath), "Recorded file (*.rhd);;All Files (*)");
        fileNametmp = fileurl.fileName();
        filePathtmp = fileurl.path();
        filePathtmp.remove(0,1);
    }
    //QMessageBox::information(this, "Unable to open file", fileName);
    //QMessageBox::information(this, "Unable to open file", filePath);

    if (fileNametmp.isEmpty()){
        return;
    } else {
        appbusy = true;
        this->setCursor(Qt::WaitCursor);
        QFile file(filePathtmp);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, "Unable to open file", file.errorString());
            return;
        }

        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_4_8); // should I use pointer? in -> setVersion ?
        in.setByteOrder(QDataStream::LittleEndian);
        in.setFloatingPointPrecision(QDataStream::SinglePrecision);

        filebytesize = file.size();

        in >> DATA_FILE_MAGIC_NUMBER;
        in >> DATA_FILE_MAIN_VERSION_NUMBER;
        in >> DATA_FILE_SECONDARY_VERSION_NUMBER;
        in >> boardSampleRate;
        in >> dspEnabled;
        in >> actualDspCutoffFreq;
        in >> actualLowerBandwidth;
        in >> actualUpperBandwidth;
        in >> desiredDspCutoffFreq;
        in >> desiredLowerBandwidth;
        in >> desiredUpperBandwidth;
        in >> notchFilterComboBoxCurrentIndex;
        in >> desiredImpedanceFreq;
        in >> actualImpedanceFreq;
        in >> note1LineEdit;
        in >> note2LineEdit;
        in >> note3LineEdit;
        if ((DATA_FILE_MAIN_VERSION_NUMBER == 1 && DATA_FILE_SECONDARY_VERSION_NUMBER >=1) || (DATA_FILE_MAIN_VERSION_NUMBER > 1)){
            in >> numTempSensors;
        }
        if ((DATA_FILE_MAIN_VERSION_NUMBER == 1 && DATA_FILE_SECONDARY_VERSION_NUMBER >=3) || (DATA_FILE_MAIN_VERSION_NUMBER > 1)){
            in >> evalBoardMode;
        }
        in >> numSignalGroups;

        spikeprepeak = round(paramspikeprepeak/1000*boardSampleRate); // pnt
        spikepostpeak = round(paramspikepostpeak/1000*boardSampleRate)-1; // pnt
        baseprepeak = round(parambaseprepeak/1000*boardSampleRate); // pnt
        basepostpeak = round(parambasepostpeak/1000*boardSampleRate)-1; // pnt
        spikemergethres = round(paramspikemergethres/1000*boardSampleRate); // pnt
        rasterresolution = paramrasterresolution/1000; // s

        int amplifier_index = 0;
        int aux_input_index = 0;
        int supply_voltage_index = 0;
        int board_adc_index = 0;
        int board_dig_in_index = 0;
        int board_dig_out_index = 0;
        nativeChannelNameraw.clear();
        customChannelNameraw.clear();
        nativeChannelName.clear();
        customChannelName.clear();
        nativeChannelNameraw.resize(numSignalGroups);
        customChannelNameraw.resize(numSignalGroups);
        nativeChannelName.resize(numSignalGroups);
        customChannelName.resize(numSignalGroups);
        // For now this program can read only 1 SignalGroup (one intan chip connected with a blue cable)
        for (int ii = 0; ii < numSignalGroups; ++ii) {
            in >> signalGroupName;
            if (ii == 1){
                strout = signalGroupName;
            }
            in >> signalGroupPrefix;
            in >> signalGroupEnabled;
            in >> signalGroupNumChannels;
            in >> signalGroupNumAmpChannels;
            nativeChannelNameraw[ii].resize(signalGroupNumChannels);
            customChannelNameraw[ii].resize(signalGroupNumChannels);
            for (int jj = 0; jj < signalGroupNumChannels; ++jj) {
                in >> nativeChannelNameraw[ii][jj];
                in >> customChannelNameraw[ii][jj];
                in >> nativeOrder;
                in >> customOrder;
                in >> signalType;
                in >> channelEnabled;
                in >> chipChannel;
                in >> boardStream;
                in >> voltageTriggerMode;
                if (openmasamode){
                    in >> digitalTriggerMode;
                }
                in >> voltageThreshold;
                in >> digitalTriggerChannel;
                in >> digitalEdgePolarity;
                if (openmasamode){
                    in >> analogTriggerChannel;
                    in >> analogEdgePolarity;
                }
                in >> electrodeImpedanceMagnitude;
                in >> electrodeImpedancePhase;
                if (electrodeImpedanceMagnitude != 0 && electrodeImpedanceMagnitude < 0.1){
                    file.close();
                    QMessageBox::information(this, "!Exception", "Seems not match any data format.");
                    return;
                }
                if (channelEnabled){
                    nativeChannelName[ii].append(nativeChannelNameraw[ii][jj]);
                    customChannelName[ii].append(customChannelNameraw[ii][jj]);
                    switch (signalType){
                        case 0:
                            amplifier_index = amplifier_index + 1;
                            break;
                        case 1:
                            aux_input_index = aux_input_index + 1;
                            break;
                        case 2:
                            supply_voltage_index = supply_voltage_index + 1;
                            break;
                        case 3:
                            board_adc_index = board_adc_index + 1;
                            break;
                    case 4:
                            board_dig_in_index = board_dig_in_index + 1;
                            break;
                        case 5:
                            board_dig_out_index = board_dig_out_index + 1;
                            break;
                        default:
                            break;
                            //error(sprintf('Unknown signal type: %d', signal_type));
                    }
                }
            }
        }
        num_amplifier_channels = amplifier_index;
        num_aux_input_channels = aux_input_index;
        num_supply_voltage_channels = supply_voltage_index;
        num_temp_sensor_channels = numTempSensors;
        num_board_adc_channels = board_adc_index;
        num_board_dig_in_channels = board_dig_in_index;
        num_board_dig_out_channels = board_dig_out_index;

        int bytes_per_block;
        // Each data block contains 60 amplifier samples.
        bytes_per_block = 60 * 4;  // timestamp data
        bytes_per_block = bytes_per_block + 60 * 2 * (num_amplifier_channels);
        // Auxiliary inputs are sampled 4x slower than amplifiers
        bytes_per_block = bytes_per_block + 15 * 2 * (num_aux_input_channels);
        // Supply voltage is sampled 60x slower than amplifiers
        bytes_per_block = bytes_per_block + 1 * 2 * (num_supply_voltage_channels);
        // Board analog inputs are sampled at same rate as amplifiers
        bytes_per_block = bytes_per_block + 60 * 2 * (num_board_adc_channels);
        // Board digital inputs are sampled at same rate as amplifiers
        if (board_dig_in_index > 0){
            bytes_per_block = bytes_per_block + 60 * 2;
        }
        // Board digital outputs are sampled at same rate as amplifiers
        if (board_dig_out_index > 0){
            bytes_per_block = bytes_per_block + 60 * 2;
        }
        // Temp sensor is sampled 60x slower than amplifiers
        if (numTempSensors > 0){
            bytes_per_block = bytes_per_block + 1 * 2 * numTempSensors;
        }
        // How many data blocks remain in this file?
        int data_present = 0;
        int bytes_remaining = filebytesize - in.device()->pos();
        if (bytes_remaining > 0){
            data_present = 1;
        }
        int num_data_blocks = bytes_remaining / bytes_per_block;
        //if (bytes_remaining % bytes_per_block != 0){
        //    file.close();
        //    QMessageBox::information(this, "!Exception", "Did not match any data format.");
        //    return;
       // }
        //int record_time = 60 * num_data_blocks / sample_rate;

        int num_amplifier_samples = 60 * num_data_blocks;
        int num_aux_input_samples = 15 * num_data_blocks;
        int num_supply_voltage_samples = 1 * num_data_blocks;
        int num_board_adc_samples = 60 * num_data_blocks;
        int num_board_dig_in_samples = 60 * num_data_blocks;
        int num_board_dig_out_samples = 60 * num_data_blocks;

        events.resize(num_amplifier_channels);
        eventamps.resize(num_amplifier_channels);
        eventpnts.resize(num_amplifier_channels);
        eventpeakvals.resize(num_amplifier_channels);
        t_amplifier_q.resize(num_amplifier_samples);
        t_amplifier_qu.resize(num_amplifier_samples);
        t_amplifier_db.resize(num_amplifier_samples);
        amplifier_data.resize(num_amplifier_channels);
        for (int ii = 0; ii < num_amplifier_channels; ++ii){
          amplifier_data[ii].resize(num_amplifier_samples);
        }
        amplifier_data_db.resize(num_amplifier_channels);
        for (int ii = 0; ii < num_amplifier_channels; ++ii){
          amplifier_data_db[ii].resize(num_amplifier_samples);
        }
        vector<vector<quint16>> aux_input_data;
        aux_input_data.resize(num_aux_input_channels);
        for (int ii = 0; ii < num_aux_input_channels; ++ii){
          aux_input_data[ii].resize(num_aux_input_samples);
        }
        vector<vector<quint16>> supply_voltage_data;
        supply_voltage_data.resize(num_supply_voltage_channels);
        for (int ii = 0; ii < num_supply_voltage_channels; ++ii){
          supply_voltage_data[ii].resize(num_supply_voltage_samples);
        }
        vector<vector<qint16>> temp_sensor_data;
        temp_sensor_data.resize(num_temp_sensor_channels);
        for (int ii = 0; ii < num_temp_sensor_channels; ++ii){
          temp_sensor_data[ii].resize(num_supply_voltage_samples);
        }
        vector<vector<quint16>> board_adc_data;
        board_adc_data.resize(num_board_adc_channels);
        for (int ii = 0; ii < num_board_adc_channels; ++ii){
          board_adc_data[ii].resize(num_board_adc_samples);
        }
        board_dig_in_data.resize(num_board_dig_in_channels);
        for (int ii = 0; ii < num_board_dig_in_channels; ++ii){
          board_dig_in_data[ii].resize(num_board_dig_in_samples);
        }
        //vector<quint16> board_dig_in_raw;
        //board_dig_in_raw.resize(num_board_dig_in_samples);
        vector<vector<quint16>> board_dig_out_data;
        board_dig_out_data.resize(num_board_dig_out_channels);
        for (int ii = 0; ii < num_board_dig_out_channels; ++ii){
          board_dig_out_data[ii].resize(num_board_dig_out_samples);
        }
        vector<quint16> board_dig_out_raw;
        board_dig_out_raw.resize(num_board_dig_out_samples);

        //QMessageBox::information(this, "opened file", QString::number(filebytesize));
        //QMessageBox::information(this, "opened file", QString::number(bytes_remaining));
        if (data_present){
            // Pre-allocate memory for data.
            /*if ((DATA_FILE_MAIN_VERSION_NUMBER == 1 && DATA_FILE_SECONDARY_VERSION_NUMBER >= 2) || (DATA_FILE_MAIN_VERSION_NUMBER > 1)){
                t_amplifier_q = (quint32) t_amplifier_q;
            } else {
                delete t_amplifier_q;
            }*/

            // Read sampled data from file.
            amplifier_index = 0;
            aux_input_index = 0;
            supply_voltage_index = 0;
            board_adc_index = 0;
            board_dig_in_index = 0;
            board_dig_out_index = 0;

            int print_increment = 10;
            int percent_done = print_increment;


            for (int ii = 0; ii < num_data_blocks; ++ii) {
                // In version 1.2, we moved from saving timestamps as unsigned
                // integeters to signed integers to accomidate negative (adjusted)
                // timestamps for pretrigger data.
                if ((DATA_FILE_MAIN_VERSION_NUMBER == 1 && DATA_FILE_SECONDARY_VERSION_NUMBER >= 2) || (DATA_FILE_MAIN_VERSION_NUMBER > 1)){
                    for (int jj = 0; jj < 60; ++jj){
                        in >> t_amplifier_q[amplifier_index+jj];
                        t_amplifier_db[amplifier_index+jj] = (double)t_amplifier_q[amplifier_index+jj]/(double)boardSampleRate;
                    }
                } else {
                    for (int jj = 0; jj < 60; ++jj){
                        in >> t_amplifier_qu[amplifier_index+jj];
                        t_amplifier_db[amplifier_index+jj] = (double)t_amplifier_qu[amplifier_index+jj]/(double)boardSampleRate;
                    }
                }
                if (num_amplifier_channels > 0){
                    for (int kk = 0; kk < num_amplifier_channels; ++kk){
                        for (int jj = 0; jj < 60; ++jj){
                            in >> amplifier_data[kk][amplifier_index+jj];
                            amplifier_data_db[kk][amplifier_index+jj] = 0.195 * ((double)amplifier_data[kk][amplifier_index+jj] - 32768);
                        }
                    }
                }
                if (num_aux_input_channels > 0){
                    for (int kk = 0; kk < num_aux_input_channels; ++kk){
                        for (int jj = 0; jj < 15; ++jj){
                            in >> aux_input_data[kk][aux_input_index+jj];
                        }
                    }
                }
                if (num_supply_voltage_channels > 0){
                    for (int kk = 0; kk < num_supply_voltage_channels; ++kk){
                        for (int jj = 0; jj < 1; ++jj){
                            in >> supply_voltage_data[kk][supply_voltage_index+jj];
                        }
                    }
                }
                if (num_temp_sensor_channels > 0){
                    for (int kk = 0; kk < num_temp_sensor_channels; ++kk){
                        for (int jj = 0; jj < 1; ++jj){
                            in >> temp_sensor_data[kk][supply_voltage_index+jj];
                        }
                    }
                }
                if (num_board_adc_channels > 0){
                    for (int kk = 0; kk < num_board_adc_channels; ++kk){
                        for (int jj = 0; jj < 60; ++jj){
                            in >> board_adc_data[kk][board_adc_index+jj];
                        }
                    }
                }
                if (num_board_dig_in_channels > 0){
                    for (int kk = 0; kk < num_board_dig_in_channels; ++kk){
                        for (int jj = 0; jj < 60; ++jj){
                            in >> board_dig_in_data[kk][board_dig_in_index+jj];
                        }
                    }
                }
                if (num_board_dig_out_channels > 0){
                    for (int jj = 0; jj < 60; ++jj){
                        in >> board_dig_out_raw[board_dig_out_index+jj];
                    }
                }

                amplifier_index = amplifier_index + 60;
                aux_input_index = aux_input_index + 15;
                supply_voltage_index = supply_voltage_index + 1;
                board_adc_index = board_adc_index + 60;
                board_dig_in_index = board_dig_in_index + 60;
                board_dig_out_index = board_dig_out_index + 60;

                //fraction_done = 100 * (ii / num_data_blocks);
                //if (fraction_done >= percent_done){
                    //fprintf(1, '%d%% done...\n', percent_done);
                    //percent_done = percent_done + print_increment;
                //}
            }

            // Make sure we have read exactly the right amount of data.
            //bytes_remaining = filesize - ftell(fid);
            //if (bytes_remaining ~= 0){
                //%error('Error: End of file not reached.');
            //}


        }
        file.close();

        if (DATA_FILE_MAGIC_NUMBER == 0xc6912702){
            /*
            QMessageBox::information(&window, "!!", QString::number(filebytesize));
            QMessageBox::information(&window, "!!", QString::number(bytes_remaining));
            //QMessageBox::information(&window, "!!", strout);
            QMessageBox::information(&window, "!!", QString::number(bytes_per_block));
            QMessageBox::information(&window, "!!", QString::number(num_data_blocks));
            QMessageBox::information(&window, "!!", QString::number(num_amplifier_samples));
            */
        } else {
            QMessageBox::information(this, "output", "File does not look like normal rhd data.");
        }
    }

    fileName = fileNametmp;
    filePath = filePathtmp;
    QStringList dirnames = filePathtmp.split("/");
    fileDir = "";
    for (int ii=0; ii<dirnames.length()-1; ++ii){
        fileDir = fileDir + dirnames[ii] + "/";
    }

    fileopened = true;
    //QMessageBox::information(this, "!!", QString::number(amplifier_data_db.length()));
    QString arrayname;
    QVector<int> tmpelectrodearray = electrodearray;
    if (amplifier_data_db.length() < nChannel){
        // *** user disabled some channels, so the program has to delete the data.
        // QMessageBox::information(this, "Notification", "There are only " + QString::number(amplifier_data_db.length()) + " channels. Remove channels.");
        for (int ii=tmpelectrodearray.length()-1; ii>=0; --ii){
            bool flagfound = false;
            for (int jj=0; jj<nativeChannelName[0].length(); ++jj){
                if (NNxConnectorarray[tmpelectrodearray[ii]]-1 < 10){
                    arrayname = "A-00" + QString::number(NNxConnectorarray[tmpelectrodearray[ii]]-1);
                } else {
                    arrayname = "A-0" + QString::number(NNxConnectorarray[tmpelectrodearray[ii]]-1);
                }
                if (nativeChannelName[0][jj].compare(arrayname) == 0){
                    //QMessageBox::information(this, "!!", arrayname + ";" + nativeChannelName[0][jj]);
                    flagfound = true;
                    break;
                }
            }
            if (!flagfound){
                //QMessageBox::information(this, "!!", arrayname + ";" + QString::number(ii));
                for (int jj=0; jj<electrodearray.length(); ++jj){
                    if (electrodearray[jj] > electrodearray[ii]){
                        electrodearray[jj] -= 1;
                    }
                }
                electrodearray.removeAt(ii);
            }
        }
    }


    //QMessageBox::information(this, "!!", QString::number(electrodearray.length()));

    widgetchart0->setData(amplifier_data_db.length(), boardSampleRate, nativeChannelName[0]);

    // additional data plot
    //widgetchart0->setData0(board_dig_in_data[0], t_amplifier_db);

    this->setWindowTitle("masasort: " + fileName);
    appbusy = false;
    this->setCursor(Qt::ArrowCursor);

}

QString masasortMainWindow::getFileDir(){
    return fileDir;
}

bool masasortMainWindow::getFileOpened(){
    return fileopened;
}


void masasortMainWindow::keyPressEvent(QKeyEvent *event){
    widgetchart0->keyPressReceiver(event);
    QWidget::keyPressEvent(event);
}

QVector<int>& masasortMainWindow::getElectrodeArray(){
    return electrodearray;
}

void masasortMainWindow::changeElectrodeArray(){
    QDialog dialog(this);
    dialog.setWindowTitle("Change electrode");
    QFormLayout form(&dialog);
    QVector<QRadioButton*> radiobutton;
    int nfield;
    nfield = str_electrodearray_raw.length();
    radiobutton.resize(nfield);
    for(int ii = 0; ii < nfield; ++ii) {
        //returnvalue[ii] = lineEdit[ii]->text();
        radiobutton[ii] = new QRadioButton(str_electrodearray_raw[ii], &dialog);
        if (num_elec == ii){
            radiobutton[ii]->setChecked(true);
        }
        form.addRow(QString::number(num_electrodearray_raw[ii]), radiobutton[ii]);
    }
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    int selectedElec;
    if (dialog.exec() != QDialog::Accepted) {
        return;
    } else {
        for(int ii = 0; ii < nfield; ++ii) {
            if (radiobutton[ii]->isChecked()){
                selectedElec = ii;
                break;
            }
        }
    }

    if (num_electrodearray_raw[selectedElec] != nChannel){
        QMessageBox::information(&dialog, "Error!!", "Different configuration from the current electrode, which has " + QString::number(nChannel) + " channels.");
        return;
    }

    if (num_electrodearray_raw[selectedElec] == 16){
        NNxConnectorarray = {13,14,15,16,12,11,10,9,8,7,6,5,1,2,3,4}; // RHD2132 16Ch vs CM16: #Ch of intan chip in each NNx connector pin (first NNx pin is 13th Ch in intan (A020 in 16Ch because it starts from A008)
    }
    if (num_electrodearray_raw[selectedElec] == 32){
        NNxConnectorarray = {17,18,32,31,29,27,25,23,21,22,24,26,28,30,20,19,14,13,3,5,7,9,11,12,10,8,6,4,2,1,15,16}; // RHD2132 vs CM32: #Ch of intan chip in each NNx connector pin
    }
    num_elec = selectedElec;
    electrodearray = electrodearray_raw[num_elec];
    for (int ii=0;ii<electrodearray.length(); ++ii){
        for (int jj=0; jj<NNxConnectorarray.length(); ++jj){
            if (electrodearray[ii] == NNxConnectorarray[jj]){
                electrodearray[ii] = jj;
                break;
            }
        }
    }
}

void masasortMainWindow::setElectrodeArray(QVector<int> elecarray){
    electrodearray = elecarray;
    //for (int ii = 0; ii < nChannel; ++ii){
    //    QMessageBox::information(this, "info", QString::number(electrodearray[ii]));
    //}
}


void masasortMainWindow::saveDigin0(QString chstr){

    QStringList savechlist = chstr.split(",");
    QString tmpstr;
    int tmpnum;
    for (int jj=0; jj< savechlist.length(); ++jj){
        tmpstr = savechlist.at(jj);
        if (tmpstr.compare("digin0") == 0){
            saveVector(board_dig_in_data[0], tmpstr);
        } else {
            tmpnum = tmpstr.toInt()-1;
            if (tmpnum >= num_amplifier_channels || tmpnum < 0){
                QMessageBox::information(this, "!Error", "illegal input " + tmpstr);
                return;
            }
            saveVector(amplifier_data[electrodearray[tmpnum]], tmpstr);
        }
    }
}

void masasortMainWindow::saveVector(QVector<quint16> data, QString strsuffix){
    //masasortMainWindow* parentwindow = (masasortMainWindow*)parentWidget();
    //QString rhdfilename = parentwindow->getFileName();
    QString rhdfilename = getFileName();
    rhdfilename.chop(4); // remove ".rhd"
    QString rhdpathname = getFilePathName();
    QStringList dirlists = rhdpathname.split("/");
    QString saveDir = "";
    for (int ii=0; ii<dirlists.length()-1; ++ii){
        saveDir = saveDir + dirlists[ii] + "/";
    }
    QString savename = saveDir + rhdfilename + "_" + strsuffix + ".txt";

//    QString savename = QFileDialog::getSaveFileName(this, "Save as:", rhdfilename + "_vector.txt", "ASCII (*.txt);;All Files (*)");

    //QMessageBox::information(this, "output", "ok.");

    rhdfilename= rhdfilename + ".rhd";
    if (savename.isEmpty()){
        return;
    } else {
        QFile file(savename);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, "Unable to save file", file.errorString());
            return;
        }

        // write parametes
        QTextStream out(&file);
        for (int ii=0; ii<data.length(); ++ii){
            out << data[ii] << "\r\n";
        }
        file.close();
    }
}

bool masasortMainWindow::myConfirmDialog(QString strquest){
    QDialog dialog(this);
    QFormLayout form(&dialog);
    form.addRow(new QLabel(strquest));
    QDialogButtonBox buttonBox(QDialogButtonBox::Yes | QDialogButtonBox::No, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    if (dialog.exec() != QDialog::Accepted) {
        return false;
    } else {
        return true;
    }
}
