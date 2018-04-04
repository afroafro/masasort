#include "WidgetIntanChart.h"
#include <QApplication>
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QIODevice>
#include <QMessageBox>
#include <QtCharts>
#include "QChartViewIntan.h"
#include <./../../libsvm-321/svm.h>
#include <./../../boost_1_62_0/boost/accumulators/statistics/median.hpp>  // if I put this below #include <>spuc.h> I got error...
#include <./../../spuc/generic/spuc.h>
#include <./../../spuc/generic/iir_2nd.h>
#include <./../../boost_1_62_0/boost/accumulators/accumulators.hpp>
#include <./../../boost_1_62_0/boost/accumulators/statistics/stats.hpp>
#include <./../../boost_1_62_0/boost/accumulators/statistics/mean.hpp>
#include <./../../boost_1_62_0/boost/accumulators/statistics/moment.hpp>
#include <masasortMainWindow.h>
#include <DSPFilters/include/DspFilters/Dsp.h>
#include <./../../spuce-master/spuce/filters/elliptic_iir.h>
#include <./../../spuce-master/spuce/filters/iir_coeff.h>
#include <time.h>

using namespace std;
using namespace QtCharts;
using namespace boost::accumulators;
using namespace SPUC;
using namespace spuce;
using namespace Dsp;

WidgetIntanChart::WidgetIntanChart(QWidget *parent) : QWidget(parent){
//*** This function organizes the layout of figures and buttons in the main window
//*** Check if indCh and nChannels are good because they may have to be changed to indCh + stCh or something

    setFocusPolicy(Qt::StrongFocus);

    parentwindow = (masasortMainWindow*)parentWidget();
    nOptionChart = 0;
    nChart = 4;
    stChart = 0;
    maxChart = 16;
    maxSpike = 12;
    nPoints = 600;
    detectCirclesize = 6;
    nadditionalfeatures = 0;
    hitii = -1;
    historyind = 0;
    appbusy = false;

}

void WidgetIntanChart::initParams(){
    // *** initialize for spike data
    //electrodearray.resize(nChannel);
    nChannel = parentwindow->getnChannel();
    datayfilt.resize(nChannel);
    eventpnts.resize(nChannel);
    eventamps.resize(nChannel);
    eventpeakvals.resize(nChannel);
    events.resize(nChannel);
    tutornum.resize(nChannel); // manually set spike IDs
    predictnum.resize(nChannel); // all spike IDs
    sspike.resize(nChannel);
    indsspike.resize(nChannel);
    strindsspike.resize(nChannel);
    spike_pntalign.resize(nChannel);
    // figure parameters
    axesYmax.resize(nChannel);
    axesYmin.resize(nChannel);
        // add tutor
        addTutor_path2.resize(nChannel);
        addTutor_indCh2.resize(nChannel);
        addTutor_indSpike2.resize(nChannel);
        addTutor_tutornum2.resize(nChannel);
        addTutor_features2.resize(nChannel);

    spikedetected.resize(nChannel);
    tutordetected.resize(nChannel);
    threstype.resize(nChannel);
    thresval.resize(nChannel);
    thresvalmax.resize(nChannel);
    spikeprepeak_init.resize(nChannel); // [ms]
    spikepostpeak_init.resize(nChannel); // [ms]
    baseprepeak_init.resize(nChannel); // [ms]
    basepostpeak_init.resize(nChannel); // [ms]
    spikemergethres_init.resize(nChannel); // [ms]
    rasterresolution_init.resize(nChannel); // [ms]    int spikeprepeak[16];

    highpass_freq.resize(nChannel);
    highpass_attn.resize(nChannel);
    highpass_ripple.resize(nChannel);
    lowpass_freq.resize(nChannel);
    lowpass_attn.resize(nChannel);
    lowpass_ripple.resize(nChannel);

    spike_lowpass_freq.resize(nChannel);
    spike_lowpass_attn.resize(nChannel);
    spike_lowpass_ripple.resize(nChannel);
    spike_realign_thres.resize(nChannel);
    spike_upsample_scale.resize(nChannel);

    trainnormval.resize(nChannel);
    param_C.resize(nChannel);
    param_gamma.resize(nChannel);

    stat_ampmean.resize(nChannel);
    stat_ampstd.resize(nChannel);
    stat_ampthresval.resize(nChannel);
    stat_ampthresvalmax.resize(nChannel);

    // Initialize parameters
    for (int ii=0; ii<nChannel; ++ii){
        spikedetected[ii] = 0;
        tutordetected[ii] = false;
        // parameters for detect spike
        threstype[ii] = "val";
        thresval[ii] = -50.0;
        thresvalmax[ii] = 0.0;
        spikeprepeak_init[ii] = 0.3; // [ms]
        spikepostpeak_init[ii] = 0.6; // [ms]
        baseprepeak_init[ii] = 1; // [ms]
        basepostpeak_init[ii] = 1.5; // [ms]
        spikemergethres_init[ii] = 0.1; // [ms]
        rasterresolution_init[ii] = 1.0; // [ms]
        // parameters for trainspike
        param_C[ii] = 400;
        param_gamma[ii] = 1;
        trainnormval[ii] = 1000;

        // parameters for filter
        highpass_freq[ii] = 400;
        highpass_attn[ii] = 50;
        highpass_ripple[ii] = 0.1;
        lowpass_freq[ii] = 4000;
        lowpass_attn[ii] = 50;
        lowpass_ripple[ii] = 0.1;

        spike_lowpass_freq[ii] = -1;
        spike_lowpass_attn[ii] = 50;
        spike_lowpass_ripple[ii] = 0.1;
        spike_realign_thres[ii] = 0.1;
        spike_upsample_scale[ii] = 2;

        axesYmin[ii] = 0;
        axesYmax[ii] = 0;
    }

    paramnames.clear();
    paramnames.append("electrodearray");
    paramnames.append("spikedetected");
    paramnames.append("threstype");
    paramnames.append("thresval");
    paramnames.append("stat_ampthresval");
    paramnames.append("thresvalmax");
    paramnames.append("stat_ampthresvalmax");
    paramnames.append("spikeprepeak_init");
    paramnames.append("spikepostpeak_init");
    paramnames.append("baseprepeak_init");
    paramnames.append("basepostpeak_init");
    paramnames.append("spikemergethres_init");
    paramnames.append("rasterresolution_init");
    paramnames.append("highpass_freq");
    paramnames.append("highpass_attn");
    paramnames.append("highpass_ripple");
    paramnames.append("lowpass_freq");
    paramnames.append("lowpass_attn");
    paramnames.append("lowpass_ripple");
    paramnames.append("spike_lowpass_freq");
    paramnames.append("spike_lowpass_attn");
    paramnames.append("spike_lowpass_ripple");
    paramnames.append("spike_realign_thres");
    paramnames.append("spike_upsample_scale");
    paramnames.append("trainnormval");
    paramnames.append("param_C");
    paramnames.append("param_gamma");
    paramnames.append("strindsspike");

    // *** Initialize the optional windows
    window2 = NULL;
    windowfeature = NULL;
    for (int ii=0; ii<nChannel; ++ii){
        strindsspike[ii] = "";
    }

    // *** Initialize figure parameters
    chartLayout = new QGridLayout;
    chartLayout->setRowMinimumHeight(50,20);
    chartLayout->setContentsMargins(0,0,0,0);
    chartLayout->setSpacing(0);

    linepen.setWidth(0.5);
    linepen.setColor(QColor(60,60,60,150));
    linepenspike.setWidth(0.5);
    linepenspike.setColor(QColor(160,160,160,40));
    linepencursor.setWidth(0.5);
    linepencursor.setColor(QColor(50,50,50,100));
    linepencurrent.setWidth(0.5);
    linepencurrent.setColor(QColor(0,0,0,255));

    linepenfilt.setWidth(0.5);
    linepenfilt.setColor(QColor(100,0,0,150));
    linepenthres.setWidth(0.5);
    linepenthres.setColor(QColor(70,150,0,100));
    detectcolor[0] = QColor(200,0,0,150);
    detectcolor[1] = QColor(0,100,255,150);
    detectcolor[2] = QColor(150,150,0,150);
    detectcolor[3] = QColor(0,200,0,150);
    detectcolor[4] = QColor(220,0,220,150);
    detectcolor[5] = QColor(50,120,0,150);
    detectcolor[6] = QColor(250,120,100,150);
    detectcolor[7] = QColor(50,0,100,150);
    detectcolor[8] = QColor(0,250,200,150);
    detectcolor[9] = QColor(220,80,150,150);
    detectcolor[10] = QColor(100,100,100,100);
    detectcolor[11] = QColor(0,0,0,255);
    for (int ii=0; ii<12; ++ii){
        linependetect[ii].setWidth(0.5);
        linependetect[ii].setColor(detectcolor[ii]);
    }

    linepentutor.resize(10);
    for (int ii=0; ii<10; ++ii){
        linepentutor[ii].setWidth(0.5);
    }
    linepentutor[0].setColor(QColor(200,0,0,100));
    linepentutor[1].setColor(QColor(0,100,255,100));
    linepentutor[2].setColor(QColor(150,150,0,100));
    linepentutor[3].setColor(QColor(0,200,0,100));
    linepentutor[4].setColor(QColor(220,0,220,100));
    linepentutor[5].setColor(QColor(50,120,0,100));
    linepentutor[6].setColor(QColor(250,120,100,100));
    linepentutor[7].setColor(QColor(50,0,100,100));
    linepentutor[8].setColor(QColor(0,250,200,100));
    linepentutor[9].setColor(QColor(220,80,150,100));
    linepennoise.setWidth(0.5); // not used
    linepennoise.setColor(QColor(0,0,0,20)); // not used

    // *** Initialize figures

    // this has to be figure number rather than nChannel
    for (int ii=0; ii<nOptionChart; ++ii){

    }


    for (int ii=0; ii<nChart; ++ii){
        chart.append(new QChartIntan());
        chart[ii]->legend()->hide();
        chart[ii]->setMargins(QMargins(0,0,0,0));
        chart[ii]->setMinimumHeight(20.0);
        chart[ii]->setAnimationOptions(QChart::SeriesAnimations);

        axesX.append(new QValueAxis);   //QObject::connect(chart[ii], SIGNAL(scaleChanged()), this, SLOT(xaxisChanged()));
        chart[ii]->addAxis(axesX[ii], Qt::AlignBottom);
        axesY.append(new QValueAxis);
        chart[ii]->addAxis(axesY[ii], Qt::AlignLeft);

        // Buttons
        buttons.append(new QWidget());
        buttonDetectSpike.append(new QPushButton("Detect" + QString::number(ii+1)));
        buttonShowFilt.append(new QPushButton("Show filter" + QString::number(ii+1)));
        buttonShowSpike.append(new QPushButton("Show spikes" + QString::number(ii+1)));
        buttonfiltpressed.append(false);

        buttons[ii]->setEnabled(false);
        buttonDetectSpike[ii]->setEnabled(false);
        buttonShowFilt[ii]->setEnabled(false);
        buttonShowSpike[ii]->setEnabled(false);

        // to define the button functions
        buttonsignalmapper.append(new QSignalMapper(buttons[ii]));
        QObject::connect(buttonDetectSpike[ii], SIGNAL(released()), buttonsignalmapper[ii*3], SLOT(map()));
        buttonsignalmapper.append(new QSignalMapper(buttons[ii]));
        QObject::connect(buttonShowFilt[ii], SIGNAL(released()), buttonsignalmapper[ii*3+1], SLOT(map()));
        buttonsignalmapper.append(new QSignalMapper(buttons[ii]));
        QObject::connect(buttonShowSpike[ii], SIGNAL(released()), buttonsignalmapper[ii*3+2], SLOT(map()));
        buttonsignalmapper[ii*3]->setMapping(buttonDetectSpike[ii], ii); // if clicked throw ii (Channel name)
        buttonsignalmapper[ii*3+1]->setMapping(buttonShowFilt[ii], ii);
        buttonsignalmapper[ii*3+2]->setMapping(buttonShowSpike[ii], ii);
        QObject::connect(buttonsignalmapper[ii*3], SIGNAL(mapped(int)), this, SLOT(buttonpushDetectSpike(int)));
        QObject::connect(buttonsignalmapper[ii*3+1], SIGNAL(mapped(int)), this, SLOT(buttonpushShowFilt(int)));
        QObject::connect(buttonsignalmapper[ii*3+2], SIGNAL(mapped(int)), this, SLOT(buttonpushShowSpike(int)));

        buttonLayout.append(new QVBoxLayout);
        buttonLayout[ii] -> addWidget(buttonDetectSpike[ii]);
        buttonLayout[ii] -> addWidget(buttonShowFilt[ii]);
        buttonLayout[ii] -> addWidget(buttonShowSpike[ii]);
        buttons[ii] -> setLayout(buttonLayout[ii]);

        series.append(new QLineSeries());
        series[ii]->setPen(linepen);   //series->setUseOpenGL(true);
        seriesfilt.append(new QLineSeries());
        seriesfilt[ii]->setPen(linepenfilt);
        seriesthres.append(new QLineSeries());
        seriesthres[ii]->setPen(linepenthres);
        chart[ii]->addSeries(series[ii]);
        chart[ii]->addSeries(seriesfilt[ii]);
        chart[ii]->addSeries(seriesthres[ii]);
        series[ii]->attachAxis(axesX[ii]);
        series[ii]->attachAxis(axesY[ii]);
        seriesfilt[ii]->attachAxis(axesX[ii]);
        seriesfilt[ii]->attachAxis(axesY[ii]);
        seriesthres[ii]->attachAxis(axesX[ii]);
        seriesthres[ii]->attachAxis(axesY[ii]);
        for (int jj=0; jj<maxSpike; ++jj){
            seriesdetect.append(new QScatterSeries());
            seriesdetect[ii*maxSpike+jj]->setPen(linependetect[jj]);
            seriesdetect[ii*maxSpike+jj]->setMarkerSize(detectCirclesize);
            seriesdetect[ii*maxSpike+jj]->setColor(QColor(0,0,0,0));
            //seriesdetect[ii*maxSpike+jj]->setColor(detectcolor[jj]);
            chart[ii]->addSeries(seriesdetect[ii*maxSpike+jj]);
            seriesdetect[ii*maxSpike+jj]->attachAxis(axesX[ii]);
            seriesdetect[ii*maxSpike+jj]->attachAxis(axesY[ii]);
        }
        chartview.append(new QChartViewIntan(chart[ii]));
        chartview[ii]->setRenderHint(QPainter::Antialiasing);
        chartview[ii]->setID(ii);
    }

    /* Additional graph
    chart0 = new QChartIntan();
    chart0->legend()->hide();
    chart0->setMargins(QMargins(0,0,0,0));
    chart0->setMinimumHeight(20.0);
    chart0->setAnimationOptions(QChart::SeriesAnimations);
    axesX0 = new QValueAxis;
    chart0->addAxis(axesX0, Qt::AlignBottom);
    axesY0 = new QValueAxis;
    chart0->addAxis(axesY0, Qt::AlignLeft);
    series0 = new QLineSeries();
    series0->setPen(linepen);
    chart0->addSeries(series0);
    series0->attachAxis(axesX0);
    series0->attachAxis(axesY0);
    chartview0 = new QChartViewIntan(chart0);
    chartview0->setRenderHint(QPainter::Antialiasing);
    chartLayout->addWidget(chartview0, 0, 1);
    */

    // display optional chart
    for (int ii=0; ii<nOptionChart; ++ii){
        //chartLayout->addWidget(chartview[ii+stChart], ii+1, 1);
    }
    // display default chart
    for (int ii=0; ii<nChart; ++ii){
        //QMessageBox::information(this, "!!", QString::number(nChart));
        chartLayout->addWidget(buttons[ii+stChart], ii+1, 0);
        chartLayout->addWidget(chartview[ii+stChart], ii+1, 1);
    }
    this->setLayout(chartLayout);
}

int WidgetIntanChart::getnChannel(){
    return nChannel;
}

QPen WidgetIntanChart::getLinepenspike(){
    return linepenspike;
}

QPen WidgetIntanChart::getLinepencurrent(){
    return linepencurrent;
}

QVector<QPen> WidgetIntanChart::getLinepentutor(){
    return linepentutor;
}
QPen WidgetIntanChart::getLinepennoise(){
    return linepennoise;
}
QPen WidgetIntanChart::getLinepencursor(){
    return linepencursor;
}

void WidgetIntanChart::setData(int query_nChannel, double query_samplerate, QVector<QString> query_channelname) {
    channelname = query_channelname;
    setData(query_nChannel, query_samplerate);
}

void WidgetIntanChart::setData(int query_nChannel, int query_nChart, int query_stChart, int query_nPoints, double query_samplerate) {
    // datay should be sorted based on electrodearray before entering this function
    nChart = query_nChart;
    stChart = query_stChart;
    nPoints = query_nPoints;
    setData(query_nChannel, query_samplerate);
}

void WidgetIntanChart::setData(int query_nChannel, double query_samplerate) {
    // datay should be sorted based on electrodearray before entering this function
    if (window2){
        window2->close();
    }
    if (windowfeature){
        windowfeature->close();
    }

    for (int ii=0; ii<nChart; ++ii){
        // Buttons
        for (int jj=0; jj<buttons.length(); ++jj){
            buttons[jj]->setEnabled(true);
            buttonDetectSpike[jj]->setEnabled(true);
            buttonShowFilt[jj]->setEnabled(true);
            buttonShowSpike[jj]->setEnabled(true);
        }
    }

    // *** initializing the buttons (Is this necessary?? these may be initialized in updateSeriesX)
    for (int ii=0; ii<nChart; ++ii){
        buttonfiltpressed[ii] = false;
        series[ii]->clear();
        seriesfilt[ii]->clear();
        seriesthres[ii]->clear();
        for (int jj=0; jj<maxSpike; ++jj){
            seriesdetect[ii*maxSpike+jj]->clear();
        }
    }
    nChannel = query_nChannel;
    QVector<QVector<double>>& datay = parentwindow->getDatay();
    QVector<double>& datat = parentwindow->getDatat();

    QVector<int>& electrodearray = parentwindow->getElectrodeArray();
    boardSampleRate = query_samplerate;
    datayfilt.clear();
    datayfilt.resize(nChannel);
    for (int ii=0; ii<nChannel; ++ii){
        datayfilt[ii].resize(datay[ii].length());
    }
    tutornum.clear();
    tutornum.resize(nChannel);
    predictnum.clear();
    predictnum.resize(nChannel);

    eventpnts.clear();
    eventpnts.resize(nChannel);
    eventamps.clear();
    eventamps.resize(nChannel);
    eventpeakvals.clear();
    eventpeakvals.resize(nChannel);
    events.clear();
    events.resize(nChannel);
    sspike.clear();
    sspike.resize(nChannel);
    indsspike.clear();
    indsspike.resize(nChannel);
    spike_pntalign.clear();
    spike_pntalign.resize(nChannel);

    // *** initialize figures
    for (int ii=0; ii<nChart; ++ii){
        axesX[ii]->setRange(datat[0],datat[datat.length()-1]);
        //QMessageBox::information(this, "!!", QString::number(electrodearray[ii+stChart]));
        //QMessageBox::information(this, "!!", QString::number(electrodearray[ii+stChart]) + ":" + channelname[electrodearray[ii+stChart]]);
        axesX[ii]->setTitleText('[' + QString::number(ii+1+stChart)  + ']' + channelname[electrodearray[ii+stChart]]);
    }
    updateSeriesX();    // show chart (stChart to stChart+nChart-1)
}


void WidgetIntanChart::changeChart(int query_nChart, int query_stChart, int query_nPoints){
    //** Add or delete nChart
    if (nChart > query_nChart){
        for (int ii=query_nChart; ii<nChart; ++ii){
            chartLayout->removeWidget(buttons[ii]);
            // !!! after removing them, if the window is closed, buttons may not be deleted
            buttons[ii]->setVisible(false);
            chartLayout->removeWidget(chartview[ii]);
            // !!! after removing them, if the window is closed, buttons may not be deleted
            chartview[ii]->setVisible(false);
        }
    } else if (nChart < query_nChart){
        for (int ii=nChart; ii<query_nChart; ++ii){
            chartLayout->addWidget(buttons[ii], ii-nChart, 0);
            chartLayout->addWidget(chartview[ii], ii-nChart, 1);
        }
    }

    QVector<int>& electrodearray = parentwindow->getElectrodeArray();
    //*** Assign values
    if (query_nChart < 1){
        nChart = 1;
    } else if (query_nChart > nChannel){
        nChart = nChannel;
    } else {
        nChart = query_nChart;
    }
    if (query_stChart < 0){
        stChart = 0;
    } else if (query_stChart > nChannel-nChart) {
        stChart = nChannel-nChart;
    } else {
        stChart = query_stChart;
    }
    nPoints = query_nPoints;

    for (int ii=0; ii<nChart; ++ii){
        axesX[ii]->setTitleText('[' + QString::number(ii+1+stChart)  + ']' + channelname[electrodearray[ii+stChart]]);
    }

    updateSeriesX();

    //*** Is this necessary?? maybe for updating the charts?
    /*
    chartLayout->setRowMinimumHeight(50,20);
    chartLayout->setContentsMargins(0,0,0,0);
    chartLayout->setSpacing(0);
    chartLayout->update();
    */
    //this->setLayout(chartLayout);
}

void WidgetIntanChart::releasedChartButtonTop(){
    changeChart(nChart, 0, nPoints);
}
void WidgetIntanChart::releasedChartButtonUp(){
    changeChart(nChart, stChart-1, nPoints);
}
void WidgetIntanChart::releasedChartButtonDown(){
    changeChart(nChart, stChart+1, nPoints);
}
void WidgetIntanChart::releasedChartButtonBottom(){
    changeChart(nChart, nChannel-nChart, nPoints);
}

void WidgetIntanChart::releasedChartButton4(){
    changeChart(4, stChart, nPoints);
}
void WidgetIntanChart::releasedChartButton8(){
    changeChart(8, stChart, nPoints);
}
void WidgetIntanChart::releasedChartButton12(){
    changeChart(12, stChart, nPoints);
}
void WidgetIntanChart::releasedChartButton16(){
    changeChart(16, stChart, nPoints);
}

void WidgetIntanChart::updateSeriesX(){
// *** draw plots (series) in the figures
    if (!parentwindow->getFileOpened()){
        return;
    }

    int stpnt;
    int enpnt;
    int datatlen;
    for (int ii=0; ii<nChart; ++ii){
        series[ii]->clear();
        seriesfilt[ii]->clear();
        seriesthres[ii]->clear();
        seriesdetect[ii]->clear();
    }

    QVector<double>& datat = parentwindow->getDatat();
    QVector<QVector<double>>& datay = parentwindow->getDatay();
    QVector<int>& electrodearray = parentwindow->getElectrodeArray();

    for (int ii=0; ii<nChart; ++ii){
    // for (int ii=stChart; ii<nChart+stChart; ++ii){
    // *** ii = #Ch

        qreal minaxisX = axesX[ii]->min();
        qreal maxaxisX = axesX[ii]->max();

        int minpnt = (int)((minaxisX - datat[0]) / (datat[1] - datat[0]));
        int maxpnt = (int)((maxaxisX - datat[0]) / (datat[1] - datat[0]));

        double minx;
        double miny;
        double maxx;
        double maxy;
        double minyfilt;
        double maxyfilt;
        double minxfilt;
        double maxxfilt;
        if (minpnt < 0){
            stpnt = 0;
        } else {
            stpnt = minpnt;
        }
        if (maxpnt + 1 > datat.length()){
            enpnt = datat.length();
        } else {
            enpnt = maxpnt + 1;
        }
        datatlen = enpnt - stpnt;

        //QMessageBox::information(this, "!!tlength", "len:" + QString::number(datatlen) + " range:" + QString::number(stpnt) + "-" + QString::number(enpnt));
        //QMessageBox::information(this, "!!tlength", "x:" + QString::number(datat[stpnt]) + "-" + QString::number(datay[electrodearray[ii+stChart]][stpnt]) + " y:" + QString::number(miny) + "-" + QString::number(maxy));
        //QMessageBox::information(this, "!!tlength", "Ch:" + QString::number(electrodearray[ii+stChart]));

        minx = datat[stpnt];


        miny = datay[electrodearray[ii+stChart]][stpnt];
        maxx = datat[stpnt];
        maxy = datay[electrodearray[ii+stChart]][stpnt];
        int allminy = miny;
        int allmaxy = maxy;

        if (datatlen <= nPoints){
            for (int jj = stpnt; jj < enpnt; ++jj){
                series[ii]->append(datat[jj], datay[electrodearray[ii+stChart]][jj]);
                //seriesfilt[ii]->append(datat[jj], datayfilt[electrodearray[ii]][jj]);
                if (allminy > datay[electrodearray[ii+stChart]][jj]){
                    allminy = (int)datay[electrodearray[ii+stChart]][jj];
                }
                if (allmaxy < datay[electrodearray[ii+stChart]][jj]){
                    allmaxy = (int)datay[electrodearray[ii+stChart]][jj];
                }
            }
            if (buttonfiltpressed[ii]){
                for (int jj = stpnt; jj < enpnt; ++jj){
                    seriesfilt[ii]->append(datat[jj], datayfilt[ii+stChart][jj]); // ??????????? datayfilt[electrodearray[ii+stChart]][jj] ???
                }
            }
        } else {
            for (int jj = stpnt; jj < enpnt; ++jj){
                if (jj%(datatlen/nPoints)==0){
                    if (minx < maxx){
                        series[ii]->append(minx, miny);
                        series[ii]->append(maxx, maxy);
                        //seriesfilt[ii]->append(minx, minyfilt);
                        //seriesfilt[ii]->append(maxx, maxyfilt);
                    } else {
                        series[ii]->append(maxx, maxy);
                        series[ii]->append(minx, miny);
                        //seriesfilt[ii]->append(maxx, maxyfilt);
                        //seriesfilt[ii]->append(minx, minyfilt);
                    }
                    if (allminy > miny){
                        allminy = (int)miny;
                    }
                    if (allmaxy < maxy){
                        allmaxy = (int)maxy;
                    }
                    miny = datay[electrodearray[ii+stChart]][jj];
                    maxy = datay[electrodearray[ii+stChart]][jj];
                    minx = datat[jj];
                    maxx = datat[jj];
                    //maxyfilt = datayfilt[electrodearray[ii]][jj];
                    //minyfilt = datayfilt[electrodearray[ii]][jj];
                } else {
                    if (maxy < datay[electrodearray[ii+stChart]][jj]){
                        maxy = datay[electrodearray[ii+stChart]][jj];
                        maxx = datat[jj];
                        //maxyfilt = datayfilt[electrodearray[ii]][jj];
                    }
                    if (miny > datay[electrodearray[ii+stChart]][jj]){
                        miny = datay[electrodearray[ii+stChart]][jj];
                        minx = datat[jj];
                        //minyfilt = datayfilt[electrodearray[ii]][jj];
                    }
                }
            }

            if (buttonfiltpressed[ii]){
                minxfilt = datat[stpnt];
                minyfilt = datayfilt[ii+stChart][stpnt];
                maxxfilt = datat[stpnt];
                maxyfilt = datayfilt[ii+stChart][stpnt];
                for (int jj = stpnt; jj < enpnt; ++jj){
                    if (jj%(datatlen/nPoints)==0){
                        if (minx < maxx){
                            seriesfilt[ii]->append(minxfilt, minyfilt);
                            seriesfilt[ii]->append(maxxfilt, maxyfilt);
                        } else {
                            seriesfilt[ii]->append(maxxfilt, maxyfilt);
                            seriesfilt[ii]->append(minxfilt, minyfilt);
                        }
                        minyfilt = datayfilt[ii+stChart][jj];
                        maxyfilt = datayfilt[ii+stChart][jj];
                        minxfilt = datat[jj];
                        maxxfilt = datat[jj];
                    } else {
                        if (maxyfilt < datayfilt[ii+stChart][jj]){
                            maxyfilt = datayfilt[ii+stChart][jj];
                            maxxfilt = datat[jj];
                        }
                        if (minyfilt > datayfilt[ii+stChart][jj]){
                            minyfilt = datayfilt[ii+stChart][jj];
                            minxfilt = datat[jj];
                        }
                    }
                }
            }
        }

        if (axesYmin[ii] == 0 && axesYmax[ii] == 0){
            axesY[ii]->setRange((allminy - allminy%100 - 100), (allmaxy - allmaxy%100 + 100));
        } else {
            axesY[ii]->setRange(axesYmin[ii], axesYmax[ii]);
        }

        drawDetect(ii+stChart);


        //axesX[ii]->setRange(mintick, maxtick);
        //axesX[ii]->setTickCount(ntick);

        //if (seriesthres[ii]->count() > 0){
        //    seriesthres[indCh]->clear();
        //    seriesthres[indCh]->append(axesX[indCh]->min(), thres);
        //    seriesthres[indCh]->append(axesX[indCh]->max(), thres);
        //    seriesdetect[indCh]->clear();
        //}
    }


    //QMessageBox::information(this, "!!tlength", QString::number(datat.length()));
    //QMessageBox::information(this, "!!stpnt", QString::number(stpnt));
    //QMessageBox::information(this, "!!enpnt", QString::number(enpnt));
}


void WidgetIntanChart::mousePressEvent(QMouseEvent *event){
    QWidget::mousePressEvent(event);

}

void WidgetIntanChart::mouseReleaseEvent(QMouseEvent *event){
    QWidget::mouseMoveEvent(event);
}


void WidgetIntanChart::setAxesX(qreal min, qreal max){
    QVector<double>& datat = parentwindow->getDatat();
    if (min == 0 && max == 0){
        min = datat[0];
        max = datat[datat.length() - 1];
    }
    if (min >= max){
        return;
    }
    for (int ii=0; ii<nChart; ++ii){
        axesX[ii]->setRange(min,max);
    }
    updateSeriesX();
}

void WidgetIntanChart::updateSeriesDetect(int indCh){
    QVector<double>& datat = parentwindow->getDatat();
    for (int ii=0; ii<nChart; ++ii){
        if (indCh == ii+stChart){
            seriesthres[ii]->clear();
            seriesthres[ii]->append(datat[0], stat_ampthresval[indCh]);
            seriesthres[ii]->append(datat[datat.length()-1], stat_ampthresval[indCh]);
            for (int jj=0; jj<maxSpike; ++jj){
                seriesdetect[ii*maxSpike+jj]->clear();
            }
            for (int jj=0; jj<eventpnts[indCh].length(); ++jj){
                seriesdetect[ii*maxSpike+10]->append(datat[eventpnts[indCh][jj]], eventpeakvals[indCh][jj]);
            }
            //QMessageBox::information(this, "!prehistoryind", "Ch:" + QString::number(indCh) + "; Nspikes:" + QString::number(eventpnts[indCh].length()));
        }
    }
}

void WidgetIntanChart::setAxesY(){
    QVector<QString> dialoglabel;
    QVector<QString> dialogvalue;
    QVector<QString> returnvalue;
    dialoglabel.append("axis num (0 for all)");
    dialoglabel.append("Y max");
    dialoglabel.append("Y min (reset if max&min == 0)");
    dialogvalue.append("0");
    dialogvalue.append(QString::number(axesYmax[0]));
    dialogvalue.append(QString::number(axesYmin[0]));
    returnvalue = myDialog("Y range:", dialoglabel, dialogvalue);
    if (returnvalue.length() == 0){
        return;
    }
    int axisnum = returnvalue[0].toInt();
    qreal ymax = returnvalue[1].toDouble();
    qreal ymin = returnvalue[2].toDouble();
    if (axisnum == 0){
        for (int ii=0; ii<nChart; ++ii){
            axesYmax[ii] = ymax;
            axesYmin[ii] = ymin;
        }
    } else if (axisnum >= 1 && axisnum <= 16) {
        axesYmax[axisnum-1] = ymax;
        axesYmin[axisnum-1] = ymin;
    }
    updateSeriesX();
}

void WidgetIntanChart::scaleAxesX(double scalemin, double scalemax){
    for (int ii=0; ii<nChart; ++ii){
        qreal min = axesX[ii]->min();
        qreal max = axesX[ii]->max();
        qreal len = max - min;
        axesX[ii]->setRange(min + len*scalemin, max + len*scalemax);
    }
    updateSeriesX();
}
void WidgetIntanChart::alignAxesXto(int indChart){
    for (int ii=0; ii<nChart; ++ii){
        if (ii != indChart){
            saveAxesXHistory(ii);
            break;
        }
    }
    qreal min = axesX[indChart]->min();
    qreal max = axesX[indChart]->max();
    for (int ii=0; ii<nChart; ++ii){
        axesX[ii]->setRange(min,max);
    }
    updateSeriesX();
}

void WidgetIntanChart::saveAxesXHistory(int indChart){
    //QMessageBox::information(this, "!prehistoryind", QString::number(historyind));
    if (historyind == 10){
        for (int ii=0; ii<9; ++ii){
            historymin[ii] = historymin[ii+1];
            historymax[ii] = historymax[ii+1];
        }
        historymin[9] = axesX[indChart]->min();
        historymax[9] = axesX[indChart]->max();
    } else {
        if (historyind == historymin.size()){
            historymin.append(axesX[indChart]->min());
            historymax.append(axesX[indChart]->max());
            historyind += 1;
        } else {
            historymin[historyind] = axesX[indChart]->min();
            historymax[historyind] = axesX[indChart]->max();
            historyind += 1;
        }
    }
    //QMessageBox::information(this, "!posthistoryind", QString::number(historyind));
}



void WidgetIntanChart::keyPressEvent(QKeyEvent *event){
    keyPressReceiver(event);
    QWidget::keyPressEvent(event);
}

void WidgetIntanChart::keyPressReceiver(QKeyEvent *event){
    switch (event->key()) {
        case Qt::Key_Home:
            saveAxesXHistory(0);
            setAxesX(0, 0);
            break;
        case Qt::Key_0:
            saveAxesXHistory(0);
            if (event->modifiers() == Qt::ShiftModifier){
                setAxesX(-0.01, 0.03);
            } else {
                setAxesX(-0.02, 0.06);
            }
            break;
        case Qt::Key_Up:
            saveAxesXHistory(0);
            if (event->modifiers() == Qt::ShiftModifier){
                scaleAxesX(3.5/8.0, -3.5/8.0);
            } else {
                scaleAxesX(1.0/4.0, -1.0/4.0);
            }
            break;
        case Qt::Key_Down:
            saveAxesXHistory(0);
            if (event->modifiers() == Qt::ShiftModifier){
                scaleAxesX(-3.5, 3.5);
            } else {
                scaleAxesX(-1.0/2.0, 1.0/2.0);
            }
            break;
        case Qt::Key_Left:
            saveAxesXHistory(0);
            if (event->modifiers() == Qt::ShiftModifier){
                scaleAxesX(-1.0/2.0, -1.0/2.0);
            } else {
                scaleAxesX(-1.0/8.0, -1.0/8.0);
            }
            break;
        case Qt::Key_Right:
            saveAxesXHistory(0);
            if (event->modifiers() == Qt::ShiftModifier){
                scaleAxesX(1.0/2.0, 1.0/2.0);
            } else {
                scaleAxesX(1.0/8.0, 1.0/8.0);
            }
            break;
        case Qt::Key_Z:
            if (event->modifiers() == Qt::ControlModifier){
                if (historyind >= 1){
                    //QMessageBox::information(this, "!prehistoryind", QString::number(historyind));
                    setAxesX(historymin[historyind-1], historymax[historyind-1]);
                    historyind -= 1;
                    //QMessageBox::information(this, "!posthistoryind", QString::number(historyind));
                }
            }
            break;
        case Qt::Key_Y:
            if (event->modifiers() == Qt::ControlModifier){
                if (historyind < historymin.size()-1){
                    //QMessageBox::information(this, "!prehistoryind", QString::number(historyind));
                    setAxesX(historymin[historyind+1], historymax[historyind+1]);
                    historyind += 1;
                    //QMessageBox::information(this, "!posthistoryind", QString::number(historyind));
                }
            }
            break;
        case Qt::Key_D:
            if (event->modifiers() == Qt::ControlModifier){
                setAxesY();
            }
        case Qt::Key_E:
            if (event->modifiers() == Qt::ControlModifier){
                parentwindow->changeElectrodeArray();
            }
            break;
    }
}

void WidgetIntanChart::buttonpushDetectSpike(int indChart) {
    // get User Inputs and detect spikes for the channel

    int indCh = indChart + stChart;

    QVector<QString> dialoglabel;
    QVector<QString> dialogvalue;
    QVector<QString> returnvalue;
    // other potential params: stopband?
    dialoglabel.append("Highpass cut-off freq [Hz] (skipped if<0)");
    dialoglabel.append("Highpass attenuation [dB]");
    dialoglabel.append("Highpass ripples [dB]");
    dialoglabel.append("Lowpass cut-off freq [Hz] (skipped if<0)");
    dialoglabel.append("Lowpass attenuation [dB]");
    dialoglabel.append("Lowpass ripples [dB]");
    dialogvalue.append(QString::number(highpass_freq[indCh]));
    dialogvalue.append(QString::number(highpass_attn[indCh]));
    dialogvalue.append(QString::number(highpass_ripple[indCh]));
    dialogvalue.append(QString::number(lowpass_freq[indCh]));
    dialogvalue.append(QString::number(lowpass_attn[indCh]));
    dialogvalue.append(QString::number(lowpass_ripple[indCh]));
    returnvalue = myDialog("Elliptic filter (2nd-order):", dialoglabel, dialogvalue);
    if (returnvalue.length() == 0){
        return;
    }
    highpass_freq[indCh] = returnvalue[0].toDouble();
    highpass_attn[indCh] = returnvalue[1].toDouble();
    highpass_ripple[indCh] = returnvalue[2].toDouble();
    lowpass_freq[indCh] = returnvalue[3].toDouble();
    lowpass_attn[indCh] = returnvalue[4].toDouble();
    lowpass_ripple[indCh] = returnvalue[5].toDouble();

    // dialog for detect params
    dialoglabel.clear();
    dialogvalue.clear();
    returnvalue.clear();
    dialoglabel.append("threstype");
    dialoglabel.append("thresval");
    dialoglabel.append("thresvalmax (skipped if 0)");
    dialoglabel.append("spikeprepeak_init");
    dialoglabel.append("spikepostpeak_init");
    dialoglabel.append("baseprepeak_init");
    dialoglabel.append("basepostpeak_init");
    dialoglabel.append("spikemergethres_init");
    dialoglabel.append("rasterresolution_init");
    dialogvalue.append(threstype[indCh]);
    dialogvalue.append(QString::number(thresval[indCh]));
    dialogvalue.append(QString::number(thresvalmax[indCh]));
    dialogvalue.append(QString::number(spikeprepeak_init[indCh]));
    dialogvalue.append(QString::number(spikepostpeak_init[indCh]));
    dialogvalue.append(QString::number(baseprepeak_init[indCh]));
    dialogvalue.append(QString::number(basepostpeak_init[indCh]));
    dialogvalue.append(QString::number(spikemergethres_init[indCh]));
    dialogvalue.append(QString::number(rasterresolution_init[indCh]));
    returnvalue = myDialog("Parameters:", dialoglabel, dialogvalue);
    if (returnvalue.length() == 0){
        return;
    }
    threstype[indCh] = returnvalue[0];
    thresval[indCh] = returnvalue[1].toDouble();
    thresvalmax[indCh] = returnvalue[2].toDouble();
    spikeprepeak_init[indCh] = returnvalue[3].toDouble();
    spikepostpeak_init[indCh] = returnvalue[4].toDouble();
    baseprepeak_init[indCh] = returnvalue[5].toDouble();
    basepostpeak_init[indCh] = returnvalue[6].toDouble();
    spikemergethres_init[indCh] = returnvalue[7].toDouble();
    rasterresolution_init[indCh] = returnvalue[8].toDouble();

    detectSpike(indCh, false);

    if (spikedetected[indCh] && eventpnts[indCh].length() > 0){
        dialoglabel.clear();
        dialogvalue.clear();
        returnvalue.clear();
        dialoglabel.append("upsampling rate [int](<=10, skipped if<=1)");
        dialoglabel.append("Lowpass cut-off freq [Hz] (skipped if<0)");
        dialoglabel.append("Lowpass attenuation [dB]");
        dialoglabel.append("Lowpass ripples [dB]");
        dialogvalue.append(QString::number(spike_upsample_scale[indCh]));
        if (spike_lowpass_freq[indCh] == -1){
            dialogvalue.append(QString::number(boardSampleRate/2));
        } else {
            dialogvalue.append(QString::number(spike_lowpass_freq[indCh]));
        }
        dialogvalue.append(QString::number(spike_lowpass_attn[indCh]));
        dialogvalue.append(QString::number(spike_lowpass_ripple[indCh]));
        returnvalue = myDialog(QString::number(eventpnts[indCh].length()) + " spikes were detected.", dialoglabel, dialogvalue);
        if (returnvalue.length() == 0){
            return;
        }
        spike_upsample_scale[indCh] = returnvalue[0].toInt();
        if (spike_upsample_scale[indCh] <= 1){
            spike_upsample_scale[indCh] = 1;
        } else if (spike_upsample_scale[indCh] > 10){
            spike_upsample_scale[indCh] = 10;
        }
        spike_lowpass_freq[indCh] = returnvalue[1].toDouble();
        spike_lowpass_attn[indCh] = returnvalue[2].toDouble();
        spike_lowpass_ripple[indCh] = returnvalue[3].toDouble();

        resampleSpike(indCh);
    }
}

QVector<QString> WidgetIntanChart::myDialog(QString dialogtitle, QVector<QString> dialoglabel, QVector<QString> dialogvalue){
    QVector<QString> returnvalue;
    if (dialoglabel.length() != dialogvalue.length()){
        return returnvalue;
    }
    int nfield = dialoglabel.length();

    QDialog dialog(this);
    dialog.setWindowTitle(dialogtitle);
    QFormLayout form(&dialog);
    //form.addRow(new QLabel("Parameters:"));
    QList<QLineEdit*> fields;
    QLineEdit* lineEdit[nfield];
    for(int ii = 0; ii < nfield; ++ii) {
        lineEdit[ii] = new QLineEdit(dialogvalue[ii], &dialog);
        form.addRow(dialoglabel[ii], lineEdit[ii]);
        fields << lineEdit[ii];
    }
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    if (dialog.exec() != QDialog::Accepted) {
        return returnvalue;
    } else {
        returnvalue.resize(nfield);
        for(int ii = 0; ii < nfield; ++ii) {
            returnvalue[ii] = lineEdit[ii]->text();
        }
        return returnvalue;
    }

}

void WidgetIntanChart::detectSpike(int indCh, bool loaded){
    spikeprepeak = round(spikeprepeak_init[indCh]/1000*boardSampleRate); // pnt
    spikepostpeak = round(spikepostpeak_init[indCh]/1000*boardSampleRate); // pnt
    baseprepeak = round(baseprepeak_init[indCh]/1000*boardSampleRate); // pnt
    basepostpeak = round(basepostpeak_init[indCh]/1000*boardSampleRate); // pnt
    spikemergethres = round(spikemergethres_init[indCh]/1000*boardSampleRate); // pnt
    rasterresolution = rasterresolution_init[indCh]/1000; // s
    spikerealignpnt = round(spike_realign_thres[indCh]/1000*boardSampleRate);

    long iir_coeff_order = 2;
    float highpassfreq = (float)highpass_freq[indCh]/(float)boardSampleRate; // cut-off freq (1=sampling rate)
    float lowpassfreq = (float)lowpass_freq[indCh]/(float)boardSampleRate; // cut-off freq (1=sampling rate)
    float highpassattn = highpass_attn[indCh]; // passband attenuation. dB?
    float lowpassattn = lowpass_attn[indCh]; // passband attenuation. dB?
    float highpassripple = highpass_ripple[indCh]; // ripple. dB?
    float lowpassripple = lowpass_ripple[indCh]; // ripple. dB?
    //highpass filter
    iir_coeff* filt = new iir_coeff(iir_coeff_order, filter_type::high);
    if (highpassfreq <= 0){
        elliptic_iir(*filt, 0.000000001, 0.000000001, 0.000000001);
    } else if (highpassfreq >= 0.5){
        elliptic_iir(*filt, 0.499999999, highpassripple, highpassattn);
    } else {
        elliptic_iir(*filt, highpassfreq, highpassripple, highpassattn);
    }
    iir_2nd<double> highpassfilt(filt->get_b(0), filt->get_b(1), filt->get_b(2), filt->get_a(1), filt->get_a(2));
    // lowpass filter
    filt = new iir_coeff(iir_coeff_order, filter_type::low);
    if (lowpassfreq <= 0){
        elliptic_iir(*filt, 2, 0.000000001, 0.000000001);
    } else if (lowpassfreq >= 0.5){
        elliptic_iir(*filt, 0.499999999, lowpassripple, lowpassattn);
    } else {
        elliptic_iir(*filt, lowpassfreq, lowpassripple, lowpassattn);
    }
    iir_2nd<double> lowpassfilt(filt->get_b(0), filt->get_b(1), filt->get_b(2), filt->get_a(1), filt->get_a(2));
        //QMessageBox::information(this, "!! ok order", QString::number(filt->getOrder()));
        //QMessageBox::information(this, "!! ok a0", QString::number(filt->get_a(0)));
        //QMessageBox::information(this, "!! ok a1", QString::number(filt->get_a(1)));
        //QMessageBox::information(this, "!! ok a2", QString::number(filt->get_a(2)));
        //QMessageBox::information(this, "!! ok b0", QString::number(filt->get_b(0)));
        //QMessageBox::information(this, "!! ok b1", QString::number(filt->get_b(1)));
        //QMessageBox::information(this, "!! ok b2", QString::number(filt->get_b(2)));

    /* some variables */
    QVector<int> pntcross;
    QVector<int> pntcrossonset;
    bool flagCrossed = false;
    accumulator_set<double, stats<tag::mean, tag::moment<2>> > accwave;

    spikedetected[indCh] = 0;
    eventpeakvals[indCh].clear();
    eventpnts[indCh].clear();
    events[indCh].clear();
    eventamps[indCh].clear();

    QVector<QVector<double>>& datay = parentwindow->getDatay();
    QVector<int>& electrodearray = parentwindow->getElectrodeArray();

    if (threstype[indCh].compare("SD") == 0){
        // smooth
        for (int ii=0; ii<datay[electrodearray[indCh]].length(); ++ii){
            datayfilt[indCh][ii] = highpassfilt.clock(lowpassfilt.clock(datay[electrodearray[indCh]][ii]));
            accwave(datayfilt[indCh][ii]);
        }

        // threshold
        stat_ampmean[indCh] = mean(accwave);
        stat_ampstd[indCh] = sqrt(moment<2>(accwave));
        stat_ampthresval[indCh] = stat_ampmean[indCh] + stat_ampstd[indCh] * thresval[indCh];
        stat_ampthresvalmax[indCh] = stat_ampmean[indCh] + stat_ampstd[indCh] * thresvalmax[indCh];
    } else if (threstype[indCh].compare("val") == 0){
        // smooth
        for (int ii=0; ii<datay[electrodearray[indCh]].length(); ++ii){
            datayfilt[indCh][ii] = highpassfilt.clock(lowpassfilt.clock(datay[electrodearray[indCh]][ii]));
            accwave(datayfilt[indCh][ii]);
        }
        // threshold
        stat_ampmean[indCh] = mean(accwave);
        stat_ampstd[indCh] = sqrt(moment<2>(accwave));
        stat_ampthresval[indCh] = thresval[indCh];
        stat_ampthresvalmax[indCh] = thresvalmax[indCh];
    } else if (threstype[indCh].compare("autoval") == 0){
        // smooth
        for (int ii=0; ii<datay[electrodearray[indCh]].length(); ++ii){
            datayfilt[indCh][ii] = highpassfilt.clock(lowpassfilt.clock(datay[electrodearray[indCh]][ii]));
            accwave(datayfilt[indCh][ii]);
        }
        stat_ampmean[indCh] = mean(accwave);
        stat_ampstd[indCh] = sqrt(moment<2>(accwave));
        //QMessageBox::information(this, "!! ERROR", QString::number(indCh) + ";" + QString::number(stat_ampthresval[indCh]));
        //QMessageBox::information(this, "!! ERROR", QString::number(indCh) + ";" + QString::number(stat_ampthresvalmax[indCh]));
    } else {
        QMessageBox::information(this, "!! ERROR", "Unknown option of threstype\n");
        return;
    }
    for (int ii=0; ii<datayfilt[indCh].length(); ++ii){
        if (thresval[indCh] > 0){
            if (datayfilt[indCh][ii] > stat_ampthresval[indCh]){
                pntcross.append(ii);
            }
        } else {
            if (datayfilt[indCh][ii] < stat_ampthresval[indCh]){
                pntcross.append(ii);
            }
        }
    }
    if (pntcross.length() > 50000){
        // QMessageBox::information(this, "!! Info", "Skipped spike detection because too many events (>50000) were detected.\n Lower the threshold if possible.");
        return;
    }
    for (int ii=1; ii<pntcross.length(); ++ii){
        if (pntcross[ii-1] + 1 == pntcross[ii]){
            if (!flagCrossed){
                pntcrossonset.append(pntcross[ii-1]);
            }
            flagCrossed = true;
        } else {
            flagCrossed = false;
        }
    }

    int indlast;
    double tmpspikepeak;
    int tmpspikepeakx;
    for (int ii=0; ii<pntcrossonset.length(); ++ii){
        if (ii == pntcrossonset.length()-1){
            if (pntcrossonset[ii] + spikepostpeak < datayfilt[indCh].length()-1){
                indlast = pntcrossonset[ii] + spikepostpeak;
            } else {
                indlast = datayfilt[indCh].length()-1;
            }
        } else {
            if (pntcrossonset[ii] + spikepostpeak < pntcrossonset[ii+1]-1){
                indlast = pntcrossonset[ii] + spikepostpeak;
            } else {
                indlast = pntcrossonset[ii+1]-1;
            }
        }
        // find a peak
        tmpspikepeak = datayfilt[indCh][pntcrossonset[ii]];
        tmpspikepeakx = pntcrossonset[ii];
        if (thresval[indCh] > 0){
            for (int jj=pntcrossonset[ii]; jj < indlast; ++jj){
                if (tmpspikepeak < datayfilt[indCh][jj]){
                    tmpspikepeak = datayfilt[indCh][jj];
                    tmpspikepeakx = jj;
                }
            }
            if (thresvalmax[indCh] != 0 && stat_ampthresvalmax[indCh] < tmpspikepeak){
                continue;
            }
        } else {
            for (int jj=pntcrossonset[ii]; jj < indlast; ++jj){
                if (tmpspikepeak > datayfilt[indCh][jj]){
                    tmpspikepeak = datayfilt[indCh][jj];
                    tmpspikepeakx = jj;
                }
            }
            if (thresvalmax[indCh] != 0 && stat_ampthresvalmax[indCh] > tmpspikepeak){
                continue;
            }
        }
        // calculate the peak amplitude
        double localbaseline;
        accumulator_set<double, stats<tag::median(with_p_square_quantile)>> accbaseline;
        QVector<double> spikewaveform;
        int lenspike = spikeprepeak+spikepostpeak+spikerealignpnt*2+1;
        spikewaveform.resize(lenspike);

        if ((tmpspikepeakx-baseprepeak >= 0 && tmpspikepeakx+basepostpeak < datayfilt[indCh].length()) && (tmpspikepeakx-spikerealignpnt-spikeprepeak >= 0 && tmpspikepeakx+spikerealignpnt+spikepostpeak < datayfilt[indCh].length())) {
            eventpeakvals[indCh].append(tmpspikepeak);
            eventpnts[indCh].append(tmpspikepeakx);
            // push spike waveforms
            for(int jj=0; jj < lenspike; ++jj){
                spikewaveform[jj] = datayfilt[indCh][tmpspikepeakx-spikeprepeak-spikerealignpnt+jj];
            }
            events[indCh].append(spikewaveform);
            // baseline calculation
            for(int jj=0; jj < baseprepeak+basepostpeak+1; ++jj){
                accbaseline(datayfilt[indCh][tmpspikepeakx-baseprepeak+jj]);
            }
            localbaseline = median(accbaseline);
            if (tmpspikepeak - localbaseline > 0){
                eventamps[indCh].append(tmpspikepeak - localbaseline);
            } else {
                eventamps[indCh].append(-tmpspikepeak + localbaseline);
            }
        }
    }

    // *** detectCh
    drawDetect(indCh);

    //*** write tutornum
    tutornum[indCh].resize(eventpnts[indCh].length());
    predictnum[indCh].resize(eventpnts[indCh].length());
    spike_pntalign[indCh].resize(eventpnts[indCh].length());
    for (int ii=0; ii<eventpnts[indCh].length(); ++ii){
        tutornum[indCh][ii] = -1.0; // initial number
        predictnum[indCh][ii] = -1.0; // initial number
        spike_pntalign[indCh][ii] = 0;
    }
    spikedetected[indCh] = 1;


}

void WidgetIntanChart::resampleSpike(int indCh){
    // filter parameters
    long iir_coeff_order = 2;
    float lowpassfreq = (float)spike_lowpass_freq[indCh]/(float)boardSampleRate/(float)spike_upsample_scale[indCh]; // cut-off freq (1=sampling rate)
    float lowpassattn = spike_lowpass_attn[indCh]; // passband attenuation. dB?
    float lowpassripple = spike_lowpass_ripple[indCh]; // ripple. dB?
    iir_coeff* filt = new iir_coeff(iir_coeff_order, filter_type::low);
    if (lowpassfreq <= 0){
        elliptic_iir(*filt, 2, 0.000000001, 0.000000001);
    } else if (lowpassfreq >= 0.5){
        elliptic_iir(*filt, 0.499999999, lowpassripple, lowpassattn);
    } else {
        elliptic_iir(*filt, lowpassfreq, lowpassripple, lowpassattn);
    }
    iir_2nd<double> lowpassfilt(filt->get_b(0), filt->get_b(1), filt->get_b(2), filt->get_a(1), filt->get_a(2));

    // generate new spikes
    QVector<double> spikewaveform;
    QVector<double> spikewaveformfilt;
    QVector<double> finalspikewaveform;
    int lenspike = (spikeprepeak+spikepostpeak+spikerealignpnt*2+1)*spike_upsample_scale[indCh]-1; // truncate the end
    int lenfinalspike = (spikeprepeak+spikepostpeak+1)*spike_upsample_scale[indCh]-1;

    int indevent;
    int lenevent = events[indCh].length();
    spikewaveform.resize(lenspike);
    spikewaveformfilt.resize(lenspike);
    finalspikewaveform.resize(lenfinalspike);
    for(int ii=0; ii < lenevent; ++ii){
        //QMessageBox::information(this, "!!ok", QString::number(ii));

        // upsample spikes
        for(int jj=0; jj < lenspike; ++jj){
            indevent = (int)floor((double)jj/(double)spike_upsample_scale[indCh]);
            if (jj%spike_upsample_scale[indCh] == 0){ // !!!!!!!! this may be a bit dangerous
                spikewaveform[jj] = events[indCh][ii][indevent];
            } else {
                spikewaveform[jj] = events[indCh][ii][indevent] + (events[indCh][ii][indevent+1] - events[indCh][ii][indevent])/spike_upsample_scale[indCh]*(jj%spike_upsample_scale[indCh]);
            }
        }

        //QMessageBox::information(this, "!! upsample ok", QString::number(ii));
        // filter spikes
        for (int jj=0; jj<lenspike; ++jj){
            spikewaveformfilt[jj] = lowpassfilt.clock(spikewaveform[jj]);
        }
        // find peak pnt
        double maxval;
        int pntmax;
        int stjj = spikeprepeak*spike_upsample_scale[indCh];
        int enjj = (spikerealignpnt+spikeprepeak+1+spikerealignpnt)*spike_upsample_scale[indCh]-1;
        for (int jj=stjj; jj<enjj; ++jj){
            if (jj == stjj){
                maxval = spikewaveformfilt[jj];
                pntmax = jj;
            } else {
                if (thresval[indCh] >= 0){
                    if (maxval < spikewaveformfilt[jj]){
                        maxval = spikewaveformfilt[jj];
                        pntmax = jj;
                    }
                } else {
                    if (maxval > spikewaveformfilt[jj]){
                        maxval = spikewaveformfilt[jj];
                        pntmax = jj;
                    }
                }
            }
        }
        //QMessageBox::information(this, "!!filter ok", QString::number(ii));

        // realign spikes
        spike_pntalign[indCh][ii] = pntmax - (spikeprepeak+spikerealignpnt)*spike_upsample_scale[indCh];
        if (spike_pntalign[indCh][ii] > spikerealignpnt*spike_upsample_scale[indCh]){ // !!! may not be necessary
            spike_pntalign[indCh][ii] = spikerealignpnt*spike_upsample_scale[indCh];
        } else if (spike_pntalign[indCh][ii] < -spikerealignpnt*spike_upsample_scale[indCh]){
            spike_pntalign[indCh][ii] = -spikerealignpnt*spike_upsample_scale[indCh];
        }
        for (int jj=0; jj<lenfinalspike; ++jj){
            finalspikewaveform[jj] = spikewaveformfilt[spikerealignpnt*spike_upsample_scale[indCh] + spike_pntalign[indCh][ii] + jj];
        }
        //QMessageBox::information(this, "!!realign ok", QString::number(ii));

        events[indCh][ii] = finalspikewaveform;
    }
}
void WidgetIntanChart::buttonpushShowFilt(int indChart) {
    int indCh = indChart + stChart;
    if (buttonfiltpressed[indCh]){
        buttonfiltpressed[indCh] = false;
    } else {
        buttonfiltpressed[indCh] = true;
    }
    updateSeriesX();
}

void WidgetIntanChart::addFeatures() {
    if (appbusy){
        return;
    }

    int maxnfeatures = 12;
    int indCh = window2->getSpikeCh();

    QVector<QString> dialoglabel;
    QVector<QString> dialogvalue;
    QVector<QString> returnvalue;
    // other potential params: stopband?
    dialoglabel.append("#channel separated by comma[,]");
    if (strindsspike[indCh].isEmpty()){
        dialogvalue.append("");
    } else {
        dialogvalue.append(strindsspike[indCh]);
    }
    returnvalue = myDialog("Set #Ch for training:", dialoglabel, dialogvalue);
    if (returnvalue.length() == 0){
        return;
    }

    // check validity of input data
    if (returnvalue[0].isEmpty()){
        strindsspike[indCh] = "";
        return;
    } else {
        QStringList featureChs = returnvalue[0].split(",");
        if (featureChs.length() > maxnfeatures){
            QMessageBox::information(this, "!Error", "maximum channel number is " + QString::number(maxnfeatures));
            return;
        }
        for (int ii=0; ii< featureChs.length(); ++ii){
            int tmpnum = featureChs.at(ii).toInt()-1;
            if (tmpnum == indCh){
                QMessageBox::information(this, "!Error", "Exclude the current channel " + QString::number(tmpnum+1));
                return;
            }
            if (tmpnum >= nChannel || tmpnum < 0){
                QMessageBox::information(this, "!Error", "illegal channel number " + QString::number(tmpnum+1));
                return;
            }
        }
    }

    // set values
    strindsspike[indCh] = returnvalue[0];

    appbusy = true;
    this->setCursor(Qt::WaitCursor);

    setFeatures(indCh);

    appbusy = false;
    this->setCursor(Qt::ArrowCursor);

    QMessageBox::information(this, "info", "Features were added.");

}

void WidgetIntanChart::showFeatures() {
    if (appbusy){
        return;
    }
    appbusy = true;
    this->setCursor(Qt::WaitCursor);
    int indCh = window2->getSpikeCh();
    if (!windowfeature){
        //windowfeature = new QWidgetFeature(window2, nadditionalfeatures);
        windowfeature = new QWidgetFeature(this, nadditionalfeatures); // changed parents from window2 to this
        windowfeature->setSpikes(indsspike[indCh], sspike[indCh], tutornum[indCh], predictnum[indCh]);
    } else {
        windowfeature->deleteLater();
        //delete(windowfeature);
        // windowfeature = new QWidgetFeature(window2, nadditionalfeatures);
        // !!! this is unstable, so it would be better not to delete and change nadditionalfeatures in the current window
        windowfeature = new QWidgetFeature(this, nadditionalfeatures); // changed parents from window2 to this
        windowfeature->setSpikes(indsspike[indCh], sspike[indCh], tutornum[indCh], predictnum[indCh]);
    }
    if (windowfeature->isHidden()){
        windowfeature->show();
    }
    appbusy = false;
    this->setCursor(Qt::ArrowCursor);
}

void WidgetIntanChart::setFeatures(int indCh) {
    if (strindsspike[indCh].isEmpty()){
        return;
    }
    QStringList featureChs = strindsspike[indCh].split(",");
    nadditionalfeatures = featureChs.length();
    int nspike = events[indCh].length();

    // initiation
    sspike[indCh].clear();
    sspike[indCh].resize(nadditionalfeatures);
    indsspike[indCh].clear();
    indsspike[indCh].resize(nadditionalfeatures);
    for (int ii=0; ii< nadditionalfeatures; ++ii){
        indsspike[indCh][ii] = featureChs[ii].toInt()-1;
    }
    //QMessageBox::information(this, "info", QString::number(nspike));
    //QMessageBox::information(this, "info", QString::number(spikeprepeak+spikepostpeak+1));

    //QMessageBox::information(this, "info", "Channels" + QString::number(indCh) + "Ch" + strindsspike[indCh]);
    //QMessageBox::information(this, "info", "str[0]" + featureChs[0]);
    //QMessageBox::information(this, "info", "Channels[0]" + QString::number(indsspike[indCh][0]));
    //QMessageBox::information(this, "info", "str[1]" + featureChs[1]);
    //QMessageBox::information(this, "info", "Channels[1]" + QString::number(indsspike[indCh][1]));
    //QMessageBox::information(this, "info", "str[2]" + featureChs[2]);
    //QMessageBox::information(this, "info", "Channels[2]" + QString::number(indsspike[indCh][2]));

    // filter parameters
    long iir_coeff_order = 2;
    float lowpassfreq = (float)spike_lowpass_freq[indCh]/(float)boardSampleRate/(float)spike_upsample_scale[indCh]; // cut-off freq (1=sampling rate)
    float lowpassattn = spike_lowpass_attn[indCh]; // passband attenuation. dB?
    float lowpassripple = spike_lowpass_ripple[indCh]; // ripple. dB?
    iir_coeff* filt = new iir_coeff(iir_coeff_order, filter_type::low);
    if (lowpassfreq <= 0){
        elliptic_iir(*filt, 2, 0.000000001, 0.000000001);
    } else if (lowpassfreq >= 0.5){
        elliptic_iir(*filt, 0.499999999, lowpassripple, lowpassattn);
    } else {
        elliptic_iir(*filt, lowpassfreq, lowpassripple, lowpassattn);
    }
    iir_2nd<double> lowpassfilt(filt->get_b(0), filt->get_b(1), filt->get_b(2), filt->get_a(1), filt->get_a(2));

    QVector<QVector<double>>& datay = parentwindow->getDatay();
    QVector<int>& electrodearray = parentwindow->getElectrodeArray();

    QVector<QVector<double>> datayfilttmp;
    datayfilttmp.resize(nChannel);

    //iir_2nd<double> lowpassfilt(0.387677266990254, 0.772525419967773, 0.387677266990254, 0.335579577532574, 0.230223981542265);
    //iir_2nd<double> highpassfilt(0.944958699245553, -1.88990296370930, 0.944958699245553, -1.90951197019535, 0.914076648999788);
    for (int ii=0; ii<nadditionalfeatures; ++ii){
        //if (!spikedetected[indsspike[indCh][ii]]){
            // filter the feature channels in the same way as indCh
            long iir_coeff_order = 2;
            float highpassfreq = (float)highpass_freq[indCh]/(float)boardSampleRate; // cut-off freq (1=sampling rate)
            float lowpassfreq = (float)lowpass_freq[indCh]/(float)boardSampleRate; // cut-off freq (1=sampling rate)
            float highpassattn = highpass_attn[indCh]; // passband attenuation. dB?
            float lowpassattn = lowpass_attn[indCh]; // passband attenuation. dB?
            float highpassripple = highpass_ripple[indCh]; // ripple. dB?
            float lowpassripple = lowpass_ripple[indCh]; // ripple. dB?
            iir_coeff* filt = new iir_coeff(iir_coeff_order, filter_type::high);
            //highpass filter
            if (highpassfreq <= 0){
                elliptic_iir(*filt, 0.000000001, 0.000000001, 0.000000001);
            } else if (highpassfreq >= 0.5){
                elliptic_iir(*filt, 0.499999999, highpassripple, highpassattn);
            } else {
                elliptic_iir(*filt, highpassfreq, highpassripple, highpassattn);
            }
            iir_2nd<double> highpassfilt(filt->get_b(0), filt->get_b(1), filt->get_b(2), filt->get_a(1), filt->get_a(2));
            // lowpass filter
            filt = new iir_coeff(iir_coeff_order, filter_type::low);
            if (lowpassfreq <= 0){
                elliptic_iir(*filt, 2, 0.000000001, 0.000000001);
            } else if (lowpassfreq >= 0.5){
                elliptic_iir(*filt, 0.499999999, lowpassripple, lowpassattn);
            } else {
                elliptic_iir(*filt, lowpassfreq, lowpassripple, lowpassattn);
            }
            iir_2nd<double> lowpassfilt(filt->get_b(0), filt->get_b(1), filt->get_b(2), filt->get_a(1), filt->get_a(2));

            //QMessageBox::information(this, "info", "num" + QString::number(ii) + "Ch" + QString::number(indsspike[indCh][ii]));
            //QMessageBox::information(this, "info", "num" + QString::number(ii) + "datay length" + QString::number(datay.length()));
            //QMessageBox::information(this, "info", "num" + QString::number(ii) + "elec length" + QString::number(electrodearray.length()));
            //QMessageBox::information(this, "info", "num" + QString::number(ii) + "elec[1]" + QString::number(electrodearray[1]));
            //QMessageBox::information(this, "info", "num" + QString::number(ii) + "elec[ind]" + QString::number(electrodearray[indsspike[indCh][ii]]));
            //QMessageBox::information(this, "info", "num" + QString::number(ii) + "length" + QString::number(datay[electrodearray[indsspike[indCh][ii]]].length()));

            datayfilttmp[indsspike[indCh][ii]].resize(datay[electrodearray[indsspike[indCh][ii]]].length());
            for (int jj=0; jj<datay[electrodearray[indsspike[indCh][ii]]].length(); ++jj){
                datayfilttmp[indsspike[indCh][ii]][jj] = highpassfilt.clock(lowpassfilt.clock(datay[electrodearray[indsspike[indCh][ii]]][jj]));
            }
        //}
    }

    QVector<double> spikewaveform;
    QVector<double> spikewaveformup;
    QVector<double> spikewaveformfilt;

    // copy the wave
    int indevent;
    int lenspike = spikeprepeak+spikepostpeak+spikerealignpnt*2+1;
    int lenspikeup = (spikeprepeak+spikepostpeak+spikerealignpnt*2+1)*spike_upsample_scale[indCh]-1; // truncate the end
    int lenfinalspike = (spikeprepeak+spikepostpeak+1)*spike_upsample_scale[indCh]-1;
    spikewaveform.resize(lenspike);
    spikewaveformup.resize(lenspikeup);
    spikewaveformfilt.resize(lenspikeup);
    for (int ii=0; ii<nadditionalfeatures; ++ii){
        sspike[indCh][ii].resize(nspike);
        for (int jj=0; jj<nspike; ++jj){
            for(int kk=0; kk < lenspike; ++kk){
                // copy the wave
                spikewaveform[kk] = datayfilttmp[indsspike[indCh][ii]][eventpnts[indCh][jj]-spikeprepeak-spikerealignpnt+kk];
            }
            for(int kk=0; kk < lenspikeup; ++kk){
                // upsample spikes
                indevent = (int)floor((double)kk/(double)spike_upsample_scale[indCh]);
                if (kk%spike_upsample_scale[indCh] == 0){ // !!!!!!!! this may be a bit dangerous
                    spikewaveformup[kk] = spikewaveform[indevent];
                } else {
                    spikewaveformup[kk] = spikewaveform[indevent] + (spikewaveform[indevent+1] - spikewaveform[indevent])/spike_upsample_scale[indCh]*(kk%spike_upsample_scale[indCh]);
                }
                // filter spikes
                spikewaveformfilt[kk] = lowpassfilt.clock(spikewaveformup[kk]);
            }
            // copy the wave
            sspike[indCh][ii][jj].resize(lenfinalspike);
            for (int kk=0; kk<lenfinalspike; ++kk){
                sspike[indCh][ii][jj][kk] = spikewaveformfilt[spikerealignpnt*spike_upsample_scale[indCh] + spike_pntalign[indCh][jj] + kk];
            }
        }
    }

}


void WidgetIntanChart::runAutoTraining() { // auto sort
    if (!myConfirmDialog("If you run this, current tutor data will be deleted.\n Is it OK to delete them?")){
        return;
    }

    deleteTutorData();

    bool analyzeCh[nChannel];
    for (int ii=0; ii<nChannel; ++ii){
        analyzeCh[ii] = false;
    }
    loadTutorData(true);

    // Check the lentutor and determine which Ch to be analyzed
    int lentutor = addTutor_indCh.length();
    for (int jj=0; jj<lentutor; ++jj){
        analyzeCh[addTutor_indCh[jj]] = true;
    }
    for (int ii=0; ii<nChannel; ii++){
        for (int jj=0; jj<tutornum[ii].length(); jj++){
            if (tutornum[ii][jj] >= 0){
                analyzeCh[ii] = true;
                lentutor += 1;
            }
        }
    }
    if (lentutor == 0){
        return;
    }

    QList<QUrl> fileurls = QFileDialog::getOpenFileUrls(this, "Open", QUrl(), "Recorded file (*.rhd);;All Files (*)");
    if (fileurls.isEmpty()){
        return;
    } else {
        QVector<QString> dialoglabel;
        QVector<QString> dialogvalue;
        QVector<QString> returnvalue;
        QString filenameprefix;
        // other potential params: stopband?
        dialoglabel.append("filename prefix");
        dialogvalue.append("");
        returnvalue = myDialog("Save file name:", dialoglabel, dialogvalue);
        if (returnvalue.length() == 0){
            return;
        }
        filenameprefix = returnvalue[0];

        // show a wait bar
        QMainWindow* dialog = new QMainWindow(this);
        QWidget* dialogwidget = new QWidget(dialog);
        QGridLayout* dialogbox = new QGridLayout();
        QLabel* dialogstr0 = new QLabel(dialogwidget);
        dialogbox->addWidget(dialogstr0);
        dialogstr0->setText("Sorting spikes...");
        dialogwidget->setLayout(dialogbox);
        dialogbox->setContentsMargins(10,10,10,10);
        dialog->setCentralWidget(dialogwidget);
        dialog->show();

        for (int ii=0; ii<fileurls.length(); ++ii){
            QString fileName = fileurls[ii].fileName();
            QString filePath = fileurls[ii].path();
            filePath.remove(0,1);

            dialogstr0->setText("Sorting spikes... ( " + QString::number(ii+1) + " / " + QString::number(fileurls.length()) + " )\n" + fileName);
            qApp->processEvents(); // this is necessary for the GUI thread to repaint

            //QMessageBox::information(this, "!info", "ok");
            QStringList dirlists = filePath.split("/");
            QString saveDir = "";
            for (int ii=0; ii<dirlists.length()-1; ++ii){
                saveDir = saveDir + dirlists[ii] + "/";
            }
            QString saveName = fileName;
            saveName.chop(4);

            parentwindow->openFile(fileName, filePath);

            for (int jj=0; jj<nChannel; ++jj){
                if (analyzeCh[jj]){
                    //QMessageBox::information(this, "!info", "num " + QString::number(jj));
                    //QMessageBox::information(this, "!info", "analyze " + QString::number(jj));
                    threstype[jj] = "autoval";

                    //if (!spikedetected[jj] && eventpnts[jj].length() == 0){
                        detectSpike(jj, true);
                    //}
                    if (spikedetected[jj] && eventpnts[jj].length() > 0){
                        //QMessageBox::information(this, "!info", "num " + QString::number(jj));
                        resampleSpike(jj);
                    }
                }
            }
            for (int jj=0; jj<nChannel; ++jj){
                if (analyzeCh[jj]) {
                    if (spikedetected[jj] && eventpnts[jj].length() > 0) {
                        //QMessageBox::information(this, "!info", "num " + QString::number(jj));
                        //QMessageBox::information(this, "!info", "num " + QString::number(jj));
                        setFeatures(jj);
                        //QMessageBox::information(this, "!features", QString::number(nadditionalfeatures));
                        //QMessageBox::information(this, "!eventlen", QString::number(events[jj][0].length()));
                        //QMessageBox::information(this, "!info", "num " + QString::number(jj));

                        // *** set auto trainval (this may should be explicitly declared)
                        //trainnormval[jj] = 0;

                        trainSpike(jj, false);
                        //QMessageBox::information(this, "!info", "savename " + saveName);
                    }
                }
            }
            saveSpikeTime(saveDir + saveName + filenameprefix + "_auto_tspike.txt");
            saveData(saveDir + saveName + filenameprefix + "_auto_data.txt");
        }
        //QMessageBox::information(this, "Done", QString::number(fileurls.length()) + " files were processed.");
        dialog->close();
    }

}


void WidgetIntanChart::runAutoExtraction() {

    QList<QUrl> fileurls = QFileDialog::getOpenFileUrls(this, "Open", QUrl(), "Recorded file (*.rhd);;All Files (*)");
    if (fileurls.isEmpty()){
        return;
    } else {
        QVector<QString> dialoglabel;
        QVector<QString> dialogvalue;
        QVector<QString> returnvalue;
        QString filenameprefix;
        QString savechstr;
        // other potential params: stopband?
        dialoglabel.append("save ch (single ch num/digin0)");
        dialogvalue.append("1,digin0");
        dialoglabel.append("filename prefix");
        dialogvalue.append("");
        returnvalue = myDialog("Save file name:", dialoglabel, dialogvalue);
        if (returnvalue.length() == 0){
            return;
        }
        savechstr = returnvalue[0];
        filenameprefix = returnvalue[1];

        QStringList savechlist = returnvalue[0].split(",");
        QString tmpstr;
        int tmpnum;
        for (int jj=0; jj< savechlist.length(); ++jj){
            tmpstr = savechlist.at(jj);
            if (tmpstr.compare("digin0") != 0){
                tmpnum = tmpstr.toInt()-1;
                if (tmpnum >= nChannel || tmpnum < 0){
                    QMessageBox::information(this, "!Error", "illegal input " + tmpstr);
                    return;
                }
            }
        }

        // show a wait bar
        QMainWindow* dialog = new QMainWindow(this);
        QWidget* dialogwidget = new QWidget(dialog);
        QGridLayout* dialogbox = new QGridLayout();
        QLabel* dialogstr0 = new QLabel(dialogwidget);
        dialogbox->addWidget(dialogstr0);
        dialogstr0->setText("Extracting " + savechstr + " ...");
        dialogwidget->setLayout(dialogbox);
        dialogbox->setContentsMargins(10,10,10,10);
        dialog->setCentralWidget(dialogwidget);
        dialog->show();

        for (int ii=0; ii<fileurls.length(); ++ii){
            QString fileName = fileurls[ii].fileName();
            QString filePath = fileurls[ii].path();
            filePath.remove(0,1);

            dialogstr0->setText("Extracting " + savechstr + " from files... ( " + QString::number(ii+1) + " / " + QString::number(fileurls.length()) + " )\n" + fileName);
            qApp->processEvents(); // this is necessary for the GUI thread to repaint

            //QMessageBox::information(this, "!info", "ok");
            QStringList dirlists = filePath.split("/");
            QString saveDir = "";

            for (int jj=0; jj<dirlists.length()-1; ++jj){
                saveDir = saveDir + dirlists[jj] + "/";
            }
            QString saveName = fileName;
            saveName.chop(4);

            parentwindow->openFile(fileName, filePath);
            parentwindow->saveDigin0(savechstr);
        }
        dialog->close();
    }

}

/*
void WidgetIntanChart::setHitii(int tmphitii, int tmpChart){
    hitii = tmphitii;
    int indChart = tmpChart;
    int indCh = indChart + stChart;
    //window2->drawSpikes();
    //QMessageBox::information(this, "hitii", QString::number(hitii));

    //QMessageBox::information(this, "hitii", "1");

    // After this takes a lot of time when spike number is large (can I replace modified lines rather than reprint all the lines)
    if (window2){
        window2->setHitii(hitii);
        indCh = window2->getSpikeCh();
    }
    //QMessageBox::information(this, "hitii", "2");
    if (indCh != -1){
        drawDetect(indCh);
    }
    //QMessageBox::information(this, "hitii", "3");
    if (windowfeature){
        //QMessageBox::information(this, "info", QString::number(hitii));
        windowfeature->setHitii(hitii);
        //windowfeature->drawSpikes();
    }
    if (window2){
        window2->setFocus();
    }
}
*/


void WidgetIntanChart::setHitiis(QVector<int> tmphitii, int tmpChart){
    hitiis = tmphitii;
    int indChart = tmpChart;
    int indCh = indChart + stChart;
    //window2->drawSpikes();
    //QMessageBox::information(this, "hitii", QString::number(hitii));

    //QMessageBox::information(this, "hitii", "1");

    // After this takes a lot of time when spike number is large (can I replace modified lines rather than reprint all the lines)
    if (window2){
        window2->setHitiis(hitiis);
        indCh = window2->getSpikeCh();
    }
    //QMessageBox::information(this, "hitii", "2");
    if (indCh != -1){
        drawDetect(indCh);
    }
    //QMessageBox::information(this, "hitii", "3");
    if (windowfeature){
        //QMessageBox::information(this, "info", QString::number(hitii));
        windowfeature->setHitiis(hitiis);
        //windowfeature->drawSpikes();
    }
    if (window2){
        window2->setFocus();
    }
}

void WidgetIntanChart::buttonpushShowSpike(int indChart) {
    if (appbusy){
        return;
    }
    int indCh = indChart + stChart;
    if (events[indCh].length() == 0){
        QMessageBox::information(this, "!!", "Detect spikes first");
        return;
    }

    appbusy = true;
    this->setCursor(Qt::WaitCursor);

    // *** Reuse of window2 may induce crash after long time
    if (window2){
        window2->deleteLater();
    }

    QVector<int>& electrodearray = parentwindow->getElectrodeArray();
    QVector<double> tspike;
    int lenspike = events[indCh][0].length();
    tspike.resize(lenspike);
    for (int ii=0; ii<lenspike; ++ii){
        tspike[ii] = -spikeprepeak_init[indCh] + (double)ii/(double)spike_upsample_scale[indCh]/boardSampleRate*1000.0;
    }
    //QMessageBox::information(this, "!!", QString::number(tspike[0]));
    //QMessageBox::information(this, "!!", QString::number(tspike[lenspike-1]));

    // *** Reuse of window2 may induce crash after long time, so this is a solution
    window2 = new QWidgetSpikes(this);
    window2->setWindowTitle("masasort: Ch" + QString::number(indCh+1) + "(" + channelname[electrodearray[indCh]] + ")");
    //QMessageBox::information(this, "events", QString::number(events[indCh].length()));
    //QMessageBox::information(this, "tutor", QString::number(tutornum[indCh].length()));
    //QMessageBox::information(this, "predict", QString::number(predictnum[indCh].length()));
    window2->setSpikes(events[indCh], tspike, indCh, tutornum[indCh], predictnum[indCh]);
    if (window2->isHidden()){
        window2->show();
    }
    /*
    if (!window2){
        window2 = new QWidgetSpikes(this);
        window2->setWindowTitle("masasort: Ch" + QString::number(indCh+1) + "(" + channelname[electrodearray[indCh]] + ")");
        //QMessageBox::information(this, "events", QString::number(events[indCh].length()));
        //QMessageBox::information(this, "tutor", QString::number(tutornum[indCh].length()));
        //QMessageBox::information(this, "predict", QString::number(predictnum[indCh].length()));
        window2->setSpikes(events[indCh], tspike, indCh, tutornum[indCh], predictnum[indCh]);
    } else {
        window2->setWindowTitle("masasort: Ch" + QString::number(indCh+1) + "(" + channelname[electrodearray[indCh]] + ")");
        window2->setSpikes(events[indCh], tspike, indCh, tutornum[indCh], predictnum[indCh]);
    }

    if (window2->isHidden()){
        window2->show();
    }
    */
    /*
    for(int ii=0; ii<10; ++ii){
        window2->tutorRadio[ii]->setEnabled(false);
    }
    for(int ii=0; ii<tutornum[indCh].length(); ++ii){
        if (tutornum[indCh][ii] >= 0){
            window2->tutorRadio[(int)tutornum[indCh][ii]]->setEnabled(true);
        } else if (predictnum[indCh][ii] >= 0){
            window2->tutorRadio[(int)predictnum[indCh][ii]]->setEnabled(true);
        }
    }
    */
    appbusy = false;
    this->setCursor(Qt::ArrowCursor);
}

void WidgetIntanChart::showCurrentSpike(){
    QVector<double>& datat = parentwindow->getDatat();
    int currentspikeind = window2->getCurrentSpike();
    if (currentspikeind != -1){
        int currentspikeCh = window2->getSpikeCh();
        //QMessageBox::information(this, "!!", QString::number(currentspikeind));
        double minx = datat[eventpnts[currentspikeCh][currentspikeind]]-0.01;
        double maxx = datat[eventpnts[currentspikeCh][currentspikeind]]+0.01;
        for (int ii=0; ii<nChart; ++ii){
            axesX[ii]->setRange(minx, maxx);
        }
        updateSeriesX();
    } else {
        QMessageBox::information(this, "!!", "Click a spike first");
    }
}

void WidgetIntanChart::removeTrainedSpike(){
    QVector<double> tmptutornum = window2->getTutornum();
    int spikeCh = window2->getSpikeCh();
    if (tmptutornum.length() != 0){
        tutornum[spikeCh] = tmptutornum;
    }

    //QMessageBox::information(this, "Error", "ok1.");
    for (int ii=0; ii<eventpnts[spikeCh].length(); ++ii){
        bool flagfound = false;
        for (int jj=0; jj<nChannel; ++jj){
            if (spikedetected[jj] && jj != spikeCh){
                for (int kk=0; kk<eventpnts[jj].length(); ++kk){
                    if(eventpnts[jj][kk] > eventpnts[spikeCh][ii] + spikemergethres){
                        break;
                    }
                    if (abs(eventpnts[jj][kk] - eventpnts[spikeCh][ii]) <= spikemergethres){
                        if (predictnum[jj][kk] >= 1 || tutornum[jj][kk] >= 1){
                            tutornum[spikeCh][ii] = 0;
                            flagfound = true;
                            break;
                        }
                    }
                }
            }
            if (flagfound){
                break;
            }
        }
    }
    //QMessageBox::information(this, "Error", "ok2.");

    window2->setTutornum(tutornum[spikeCh]);
    //QMessageBox::information(this, "Error", "ok3.");
    for(int ii=0; ii<tutornum[spikeCh].length(); ++ii){
        if (tutornum[spikeCh][ii] >= 0){
            window2->setTutorRadio((int)tutornum[spikeCh][ii], true);
            //QMessageBox::information(this, "info", QString::number(tmptutornum[ii]));
            //break;
        } else if (predictnum[spikeCh][ii] >= 0){
            window2->setTutorRadio((int)predictnum[spikeCh][ii], true);
        }
    }
    if (windowfeature){
        windowfeature->setTutornum(tutornum[spikeCh]);
    }
    drawDetect(spikeCh);
}

void WidgetIntanChart::registerSpike(){
    QVector<double> tmptutornum = window2->getTutornum();
    if (tmptutornum.length() == 0){
        QMessageBox::information(this, "Error", "No tutor data. Set tutor spikes first.");
        return;
    }
    int spikeCh = window2->getSpikeCh();
    tutornum[spikeCh] = tmptutornum;
    bool flagfound = false;
    for (int ii=0; ii<tmptutornum.length(); ++ii){
        if (tmptutornum[ii] >= 0){
            flagfound = true;
            break;
        }
    }
    if (!flagfound){// all -1
        for (int ii=0; ii<predictnum[spikeCh].length(); ++ii){
            predictnum[spikeCh][ii] = -1.0;
        }
    }

    updateSpikeID();

    QMessageBox::information(this, "info", "registered");

    if (windowfeature){
        windowfeature->setTutornum(tmptutornum);
    }
    drawDetect(spikeCh);
}

void WidgetIntanChart::updateSpikeID(){

    //*** count all IDs in tutornum, predictnum, and addTutor_tutornum
    QVector<QVector<int>> nTutor;
    QVector<QVector<int>> nAddTutor;
    QVector<QVector<int>> nPredict;
    nTutor.resize(nChannel);
    nAddTutor.resize(nChannel);
    nPredict.resize(nChannel);
    for (int ii=0; ii<nChannel; ++ii){
        nTutor[ii].resize(10);
        nAddTutor[ii].resize(10);
        nPredict[ii].resize(10);
        for (int jj=0; jj<10; ++jj){ // ID spike
            nTutor[ii][jj] = 0;
            nAddTutor[ii][jj] = 0;
            nPredict[ii][jj] = 0;
            for (int kk=0; kk<tutornum[ii].length(); ++kk){
                if (jj == tutornum[ii][kk]){ // spike found
                    nTutor[ii][jj] += 1;
                    nPredict[ii][jj] += 1;
                } else if (jj == predictnum[ii][kk]){
                    nPredict[ii][jj] += 1;
                }
            }
            for (int kk=0; kk<addTutor_tutornum.length(); ++kk){
                if (ii == addTutor_indCh[kk] && jj == addTutor_tutornum[kk]){ // spike found
                    nAddTutor[ii][jj] += 1;
                }
            }
        }
    }

    //*** update variables
    spikeID_indCh.clear();
    spikeID_indSpike.clear();
    spikeID_nTutor.clear();
    spikeID_nAddTutor.clear();
    spikeID_nPredict.clear();
    for (int ii=0; ii<nChannel; ++ii){
        for (int jj=1; jj<10; ++jj){
            if (nTutor[ii][jj] > 0 || nAddTutor[ii][jj] > 0 || nPredict[ii][jj] > 0 ){ //*** spike found
                spikeID_indCh.append(ii);
                spikeID_indSpike.append(jj);
                spikeID_nTutor.append(nTutor[ii][jj]);
                spikeID_nAddTutor.append(nAddTutor[ii][jj]);
                spikeID_nPredict.append(nPredict[ii][jj]);
            }
        }
    }

}


bool WidgetIntanChart::myConfirmDialog(QString strquest){
    QDialog dialog(this);
    QFormLayout form(&dialog);
    form.addRow(new QLabel(strquest));
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    if (dialog.exec() != QDialog::Accepted) {
        return false;
    } else {
        return true;
    }
}

void WidgetIntanChart::pushTrainSpike(){
    int spikeCh = window2->getSpikeCh();

    QDialog dialog(this);
    // Use a layout allowing to have a label next to each field
    QFormLayout form(&dialog);

    // Add some text above the fields
    form.addRow(new QLabel("The question ?"));

    // Add the lineEdits with their respective labels
    QList<QLineEdit*> fields;
    int nfield = 3;
    QLineEdit* lineEdit[nfield];
    QVector<QString> texts;
    texts.resize(nfield);
    texts[0] = "C";
    lineEdit[0] = new QLineEdit(QString::number(param_C[spikeCh]), &dialog);
    texts[1] = "gamma";
    lineEdit[1] = new QLineEdit(QString::number(param_gamma[spikeCh]), &dialog);
    texts[2] = "normalization value (if <=0, max)";
    lineEdit[2] = new QLineEdit(QString::number(trainnormval[spikeCh]), &dialog);

    for(int ii = 0; ii < nfield; ++ii) {
        QString label = texts[ii];
        form.addRow(label, lineEdit[ii]);
        fields << lineEdit[ii];
    }

    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Show the dialog as modal
    if (dialog.exec() != QDialog::Accepted) {
        // If the user didn't dismiss the dialog, do something with the fields
        return;
    } else {
        //foreach(QLineEdit *lineEdit, fields) {
        //    qDebug() << lineEdit->text();
        //}
        param_C[spikeCh] = lineEdit[0]->text().toDouble();
        param_gamma[spikeCh] = lineEdit[1]->text().toDouble();
        trainnormval[spikeCh] = lineEdit[2]->text().toDouble();
    }

    trainSpike(spikeCh, true);
}

void WidgetIntanChart::trainSpike(int spikeCh, bool showinfo){
    // get tutor index >=0 ignore if tutor =-1(initial value) (also train with tutor=0[noise])
    int nspike = tutornum[spikeCh].length();
    if (nspike == 0){
        return;
    }
    int numfeatures = 1 + nadditionalfeatures;
    int numfeaturepnts = events[spikeCh][0].length();

    QVector<int> indlabel;
    //QVector<double> output1;
    //QVector<double> output2;
    for (int ii=0; ii<nspike; ++ii){
        if (tutornum[spikeCh][ii] >= 0){
            indlabel.append(ii);
        }
    }
    int nlabel = indlabel.length();
    int naddtutor = 0;
    for (int ii=0; ii < addTutor_features.length(); ++ii){
        if (addTutor_indCh[ii] == spikeCh){
//        if (addTutor_indCh[ii] == spikeCh && addTutor_features[ii].length() == numfeaturepnts*(nadditionalfeatures+1)){
            naddtutor += 1;
        }
    }

    int naddtutorfeature = 0;
    if (naddtutor == 0){
        if (nlabel == 0){
            QMessageBox::information(this, "!!Error", "No tutor spike was found. Set tutors first." + QString::number(nspike) + "; " + QString::number(nlabel));
            return;
        }
    } else {
        naddtutorfeature = addTutor_features[0].length();
    }

    // *** Calculate normalization index (so that values become from -0.5 to 0.5[actual max is -1.0 to 1.0] )
    double value;
    if (trainnormval[spikeCh] <= 0){
        trainnormval[spikeCh] = 0;
        for (int ii=0; ii < nspike; ++ii){
            for (int jj=0; jj<numfeaturepnts; ++jj){
                value = events[spikeCh][ii][jj];
                if (trainnormval[spikeCh] < value){
                    trainnormval[spikeCh] = value * 2.0;
                } else if (-trainnormval[spikeCh] > value){
                    trainnormval[spikeCh] = -value * 2.0;
                }
            }
            for (int jj=0; jj<nadditionalfeatures; ++jj){
                for (int kk=0; kk < numfeaturepnts; ++kk){
                    value = sspike[spikeCh][jj][ii][kk];
                    if (trainnormval[spikeCh] < value){
                        trainnormval[spikeCh] = value * 2.0;
                    } else if (-trainnormval[spikeCh] > value){
                        trainnormval[spikeCh] = -value * 2.0;
                    }
                }
            }
        }
        for (int ii=0; ii < addTutor_features.length(); ++ii){
            if (addTutor_indCh[ii] == spikeCh){

//            if (addTutor_indCh[ii] == spikeCh && addTutor_features[ii].length() == numfeaturepnts*(nadditionalfeatures+1)){
                for (int jj=0; jj < addTutor_features[ii].length(); ++jj){
                    value = addTutor_features[ii][jj];
                    if (trainnormval[spikeCh] < value){
                        trainnormval[spikeCh] = value * 2.0;
                    } else if (-trainnormval[spikeCh] > value){
                        trainnormval[spikeCh] = -value * 2.0;
                    }
                }
            }
        }
        // *** maximum val = 10000
        if (trainnormval[spikeCh] > 10000){
            trainnormval[spikeCh] = 10000;
        }
    }

    //QMessageBox::information(this, "info", QString::number(nlabel*(numfeatures*numfeaturepnts+1)));

    struct svm_parameter param;
    struct svm_problem prob;
    svm_node** x = new svm_node *[nlabel + naddtutor];
    svm_node* x_space = new svm_node[nlabel*(numfeatures*numfeaturepnts+1) + naddtutor*(naddtutorfeature+1)];

    //QMessageBox::information(this, "nlabel", QString::number(nlabel));
    //QMessageBox::information(this, "nlabel feat", QString::number(numfeatures*numfeaturepnts+1));
    //QMessageBox::information(this, "naddtutor", QString::number(naddtutor));
    //QMessageBox::information(this, "naddtutor feat", QString::number(naddtutorfeature+1));
    //QMessageBox::information(this, "x_spacelen", QString::number(nlabel*(numfeatures*numfeaturepnts+1) + naddtutor*naddtutorfeature+1));
    //QMessageBox::information(this, "addtutor feature len", QString::number(addTutor_features.length()));

    int indpnt=0;
    int indxpnt=0;
    int indstpnt=0;
    double exvalue = 0;

    int indexlabel;
    for (int ii=0; ii < nlabel; ++ii){
        indexlabel = 1;
        indstpnt = indpnt;
        for (int jj=0; jj<numfeaturepnts; ++jj){
            value = events[spikeCh][indlabel[ii]][jj]/trainnormval[spikeCh];
            // one training vector (feature values for a labeled datum)
            if (value != 0){ // skip if there is no value ("sparse representation")
                x_space[indpnt].index = indexlabel; // should start from 1 and ascending order
                x_space[indpnt].value = value; // maybe value should be from -1.0 to 1.0?
                if (value > 1 || value < -1){
                    exvalue = value;
                }
                indpnt += 1;
            }
            indexlabel += 1;
        }
        if (nadditionalfeatures > 0){
            for (int jj=0; jj<nadditionalfeatures; ++jj){
                for (int kk=0; kk < numfeaturepnts; ++kk){
                    value = sspike[spikeCh][jj][indlabel[ii]][kk]/trainnormval[spikeCh];
                    // one training vector (feature values for a labeled datum)
                    if (value != 0){ // skip if there is no value ("sparse representation")
                        x_space[indpnt].index = indexlabel; // should start from 1 and ascending order
                        x_space[indpnt].value = value; // maybe value should be from -1.0 to 1.0?
                        if (value > 1 || value < -1){
                            exvalue = value;
                        }
                        indpnt += 1;
                    }
                    indexlabel += 1;
                }
            }
        }
        x_space[indpnt].index = -1;
        x[indxpnt] = &x_space[indstpnt];
        indpnt += 1;
        indxpnt += 1;
//      x[indlabel[ii]] = &x_space[indstpnt];
    }
    if (exvalue != 0){
        if (showinfo){
            QMessageBox::information(this, "!!Warning", "Normalization is not sufficience. Found value = " + QString::number(exvalue));
        }
    }


    // additional teachers
    // !!! this function gives some errors when the tutor file is weird (different nadditionaltutor for different channels?)
    for (int ii=0; ii < addTutor_features.length(); ++ii){
        indexlabel = 1;
        indstpnt = indpnt;
        if (addTutor_indCh[ii] == spikeCh){
           //QMessageBox::information(this, "target ", QString::number(ii));
           //QMessageBox::information(this, "tutor# ", QString::number(addTutor_features[ii].length()));
//        if (addTutor_indCh[ii] == spikeCh && addTutor_features[ii].length() == numfeaturepnts*(nadditionalfeatures+1)){
            for (int jj=0; jj < addTutor_features[ii].length(); ++jj){
                value = addTutor_features[ii][jj]/trainnormval[spikeCh];
                if (value != 0){ // skip if there is no value ("sparse representation")
                    // QMessageBox::information(this, "!!No", QString::number(jj) + "indpnt=" + QString::number(indpnt) + ":" + QString::number(value));
                    x_space[indpnt].index = indexlabel; // should start from 1 and ascending order
                    x_space[indpnt].value = value; // maybe value should be from -1.0 to 1.0?
                    //output1.append((double)indexlabel);
                    //output2.append(value);
                    if (value > 1 || value < -1){
                        exvalue = value;
                    }
                    indpnt += 1;
                }
                indexlabel += 1;
            }
        } else {
            continue;
        }
        x_space[indpnt].index = -1;
        x[indxpnt] = &x_space[indstpnt];
        indpnt += 1;
        indxpnt += 1;
    }
    if (exvalue != 0){
        if (showinfo){
            QMessageBox::information(this, "!!Warning", "Normalization is not sufficience. Found value = " + QString::number(exvalue));
        }
    }
    //saveVectorDouble("output1.txt", output1);
    //saveVectorDouble("output2.txt", output2);

    //QMessageBox::information(this, "ok", "ok");

    prob.l = nlabel + naddtutor;
    double* labels = new double [nlabel + naddtutor];
    int n[10];
    for (int ii=0; ii<10; ++ii){
        n[ii] = 0;
    }

    prob.y = labels;
    for (int ii=0; ii<nlabel; ++ii){
        prob.y[ii] = tutornum[spikeCh][indlabel[ii]];
        n[(int)tutornum[spikeCh][indlabel[ii]]]+=1;
    }
    prob.x = x;
    indexlabel = 0;
    //QMessageBox::information(this, "!!Warning", "addTutor_features[ii].length( = " + QString::number(addTutor_features[0].length()));
    //QMessageBox::information(this, "!!Warning", "naddfeatures = " + QString::number(nadditionalfeatures));
    //QMessageBox::information(this, "!!Warning", "featurepnts = " + QString::number(numfeaturepnts));
    for (int ii=0; ii < addTutor_features.length(); ++ii){
        //QMessageBox::information(this, "!!Warning", "indCh = " + QString::number(spikeCh) + "=" + QString::number(addTutor_indCh[ii]));
        //QMessageBox::information(this, "!!Warning", "numfeaturepnts = " + QString::number(addTutor_features[ii].length()) + "=" + QString::number(numfeaturepnts*(nadditionalfeatures+1)));
        if (addTutor_indCh[ii] == spikeCh && addTutor_features[ii].length() == numfeaturepnts*(nadditionalfeatures+1)){
            //QMessageBox::information(this, "!!Warning", "tutornum = " + QString::number(addTutor_tutornum[ii]));
            prob.y[nlabel+indexlabel] = addTutor_tutornum[ii];
            n[(int)addTutor_tutornum[ii]]+=1;
            indexlabel += 1;
        }
    }

    // /////// set svm type (cannot omit unnecessary parameters)
    param.svm_type = C_SVC; // default. other option:NU_SVC,ONE_CLASS,EPSILON_SVR,NU_SVR
    param.C = param_C[spikeCh]; // cost of constrains violation. default=1 (for C_SVC, EPSILON_SVR, and NU_SVR) larger then slow convergence
    param.nr_weight = 0; // number of weighted elements
    param.weight = NULL;  // default=1, then C = C*weight for C_SVC
    param.weight_label = NULL; // weighted label label
    param.eps = 0.001; //  recommended 0.001 (but 0.00001 for nu-SVC)
    param.probability = 0; // default: Don't use SVR model (which uses probability estimate for model calculation)
    param.nu = 0.5; // default. for NU_SVC, ONE_CLASS, and NU_SVR
    param.p = 0.1; // default. for EPSILON_SVR
    // /////// set kernel
    param.kernel_type = RBF; // default (radial basis function). other option:LINEAR,POLY,SIGMOID,PRECOMPUTED
    param.gamma = param_gamma[spikeCh]; // gamma in kernel function (default= 1/num_features)
    param.degree = 3; // default = 3. for POLY
    param.coef0 = 0; // default=0 (for POLY/SIGMOID)
    param.cache_size = 100; // [Mb]. default=100
    param.shrinking = 1; // default (shrinking is conducted)

    //const char* mychar = svm_check_parameter(&prob, &param);
    //if(mychar == NULL){
    //    delete mychar;
    //} else {
    //    QMessageBox::information(this, "!!error in svm parameters", QString(mychar));
    //    delete mychar;
    //    return;
    //}

    //QMessageBox::information(this, "info", "premodel");

    struct svm_model* model = svm_train(&prob, &param);

    //QMessageBox::information(this, "info", "postmodel");

    svm_node** xtest = new svm_node *[nspike];
    svm_node* x_spacetest = new svm_node[nspike*(numfeatures*numfeaturepnts+1)];
    //QMessageBox::information(this, "!!x", QString::number(sizeof(x)));
    //svm_node* x_space = new svm_node[nspike*numfeatures];
    indpnt=0;
    indstpnt=0;

    //QMessageBox::information(this, "info", "preloop");
    exvalue = 0;
    for (int ii=0; ii < nspike; ++ii){
        indexlabel = 1;
//        svmnodep = Malloc(svm_node,numfeatures);
        indstpnt = indpnt;
        for (int jj=0; jj < numfeaturepnts; ++jj){
            value = events[spikeCh][ii][jj]/trainnormval[spikeCh];
            // one training vector (feature values for a labeled datum)
            if (value != 0){ // skip if there is no value ("sparse representation")
                x_spacetest[indpnt].index = indexlabel; // should start from 1 and ascending order
                x_spacetest[indpnt].value = value; // maybe value should be from -1.0 to 1.0?
                if (value > 1 || value < -1){
                    exvalue = value;
                }
                indpnt += 1;
            }
            indexlabel += 1;
        }
        for (int jj=0; jj<nadditionalfeatures; ++jj){
            for (int kk=0; kk < numfeaturepnts; ++kk){
                value = sspike[spikeCh][jj][ii][kk]/trainnormval[spikeCh];
                // one training vector (feature values for a labeled datum)
                if (value != 0){ // skip if there is no value ("sparse representation")
                    x_spacetest[indpnt].index = indexlabel; // should start from 1 and ascending order
                    x_spacetest[indpnt].value = value; // maybe value should be from -1.0 to 1.0?
                    if (value > 1 || value < -1){
                        exvalue = value;
                    }
                    indpnt += 1;
                }
                indexlabel += 1;
            }
        }
        x_spacetest[indpnt].index = -1;
        xtest[ii] = &x_spacetest[indstpnt];
        indpnt += 1;
//      x[ii] = &x_space[indstpnt];
    }
    if (exvalue != 0){
        if (showinfo){
            QMessageBox::information(this, "!!Warning", "Normalization is not sufficience. Found value = " + QString::number(exvalue));
        }
    }
    //QMessageBox::information(this, "info", "postloop");

    predictnum[spikeCh].clear();
    predictnum[spikeCh].resize(nspike);
    int p[10];
    for (int ii=0; ii<10; ++ii){
        p[ii] = 0;
    }
    for (int ii=0; ii < nspike; ++ii){
        if (tutornum[spikeCh][ii] >= 0){ // if tutor is specified. just copy it to predict
            predictnum[spikeCh][ii] = tutornum[spikeCh][ii];
        } else {
            predictnum[spikeCh][ii] = svm_predict(model, xtest[ii]);
        }
        if (predictnum[spikeCh][ii] > -1){
            p[(int)predictnum[spikeCh][ii]]+=1;
        }
    }
    //QMessageBox::information(this, "info", "postpred");
    //QMessageBox::information(this, "!!Error", "train spike2.");

    if (window2){
        window2->setPredictnum(predictnum[spikeCh]);
        QString outputstr= "set " + QString::number(nlabel) + "/" + QString::number(nspike) + " spikes \r\n";
        for (int ii=0; ii < 10; ++ii){
            outputstr = outputstr + "set" + QString::number(ii) + ": " + QString::number(n[ii]) + "\r\n";
            outputstr = outputstr + "prd" + QString::number(ii) + ": " + QString::number(p[ii]) + "\r\n";
        }
        //QMessageBox::information(this, "Summary", QString::number(n1) + " spikes = 1 and " + QString::number(n0) + " spikes = 0 (" + QString::number(nlabel) + "/" + QString::number(nspike) + ");\r\n Results: " + QString::number(p1)+" spikes = 1 and " + QString::number(p0) + " spikes = 0");
        if (showinfo){
            QMessageBox::information(this, "Summary",  outputstr);
        }
    }
    if (windowfeature){
        windowfeature->setPredictnum(predictnum[spikeCh]);
    }

    // fix this

    delete(labels);
    delete(x);
    delete(x_space);
    delete(xtest);
    delete(x_spacetest);
    svm_destroy_param(&param);
    svm_free_and_destroy_model(&model);

    tutordetected[spikeCh] = true;

    //QMessageBox::information(this, "Summary", "postdestroy");
    //void svm_cross_validation(const struct svm_problem *prob, const struct svm_parameter *param, int nr_fold, double *target);

}

/*
void WidgetIntanChart::showTutorSpike(QChartViewSpikes qchartviewspikes){
    tutornum = qchartviewspikes.getTutornum();
}
*/

/*
void WidgetIntanChart::pushFile(QAction *action){
    int value = action->data().toInt();
    if (value == 0){//
        saveSpikes();
    }
}
*/

void WidgetIntanChart::saveData(QString fileName){
    //masasortMainWindow* parentwindow = (masasortMainWindow*)parentWidget();
    //QString rhdfilename = parentwindow->getFileName();
    QString rhdfilename = parentwindow->getFileName();
    QString rhdpathname = parentwindow->getFilePathName();
    rhdfilename.chop(4); // remove ".rhd"

    QVector<int>& electrodearray = parentwindow->getElectrodeArray();

    if (fileName.isEmpty()){
        fileName = QFileDialog::getSaveFileName(this, "Save as:", rhdpathname + "/" + rhdfilename + "_data.txt", "ASCII (*.txt);;All Files (*)");
    }
    rhdfilename= rhdfilename + ".rhd";
    if (fileName.isEmpty()){
        return;
    } else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, "Unable to save file", file.errorString());
            return;
        }

        //if (binary)
        //QDataStream out(&file);
        //out.setVersion(QDataStream::Qt_4_8); // should I use pointer? in -> setVersion ?
        //in.setByteOrder(QDataStream::LittleEndian);
        //in.setFloatingPointPrecision(QDataStream::SinglePrecision);

        int maxlength = 0;
        for (int ii=0; ii<nChannel; ++ii){
            if (maxlength < tutornum[ii].length()){
                maxlength = tutornum[ii].length();
            }
        }
        if (maxlength < nChannel){
            maxlength = nChannel;
        }
        //QMessageBox::information(this, "info", QString::number(maxlength));

        // write parametes
        QTextStream out(&file);
        out << "masasortdata" << "\r\n";
        out << rhdfilename << "\r\n";
        out << rhdpathname << "\r\n";
        if (tutorpaths.length() > 0){
            for (int ii=0; ii<tutorpaths.length(); ++ii){
                if (ii < tutorpaths.length()-1){
                    out << tutorpaths[ii] << "\t";
                } else {
                    out << tutorpaths[ii] << "\r\n";
                }
            }
        } else {
            out << "\r\n";
        }
        for (int ii=0; ii<paramnames.length(); ++ii){
            if (ii < paramnames.length()-1){
                out << paramnames[ii] << "\t";
            } else {
                out << paramnames[ii] << "\r\n";
            }
        }
        for (int ii=0; ii<nChannel; ++ii){
            out << electrodearray[ii] << "\t";
            out << spikedetected[ii] << "\t";
            out << threstype[ii] << "\t";
            out << thresval[ii] << "\t";
            out << stat_ampthresval[ii] << "\t";
            out << thresvalmax[ii] << "\t";
            out << stat_ampthresvalmax[ii] << "\t";
            out << spikeprepeak_init[ii] << "\t";
            out << spikepostpeak_init[ii] << "\t";
            out << baseprepeak_init[ii] << "\t";
            out << basepostpeak_init[ii] << "\t";
            out << spikemergethres_init[ii] << "\t";
            out << rasterresolution_init[ii] << "\t";
            out << highpass_freq[ii] << "\t";
            out << highpass_attn[ii] << "\t";
            out << highpass_ripple[ii] << "\t";
            out << lowpass_freq[ii] << "\t";
            out << lowpass_attn[ii] << "\t";
            out << lowpass_ripple[ii] << "\t";
            out << spike_lowpass_freq[ii] << "\t";
            out << spike_lowpass_attn[ii] << "\t";
            out << spike_lowpass_ripple[ii] << "\t";
            out << spike_realign_thres[ii] << "\t";
            out << spike_upsample_scale[ii] << "\t";
            out << trainnormval[ii] << "\t";
            out << param_C[ii] << "\t";
            out << param_gamma[ii] << "\t";
            out << strindsspike[ii] << "\r\n";
        }
        for (int ii=0; ii<maxlength; ++ii){
            for (int jj=0; jj<tutornum.length(); ++jj){
                if (ii < tutornum[jj].length()){
                    out << tutornum[jj][ii] << "\t";
                } else {
                    out << "\t";
                }
                if (jj == predictnum.length()-1){
                    if (ii < predictnum[jj].length()){
                        out << predictnum[jj][ii] << "\r\n";
                    } else {
                        out << "\r\n";
                    }
                } else {
                    if (ii < predictnum[jj].length()){
                        out << predictnum[jj][ii] << "\t";
                    } else {
                        out << "\t";
                    }
                }
            }
        }
        file.close();

        QStringList dirlists = fileName.split("/");
        dataDir = "";
        for (int ii=0; ii<dirlists.length()-1; ++ii){
            dataDir = dataDir + dirlists[ii] + "/";
        }
        dataFileName = dirlists[dirlists.length()-1];
        parentwindow->setWindowTitle("masasort: " + parentwindow->getFileName() + " > " + dataFileName);


    }
}

void WidgetIntanChart::loadData(){
    //masasortMainWindow* parentwindow = (masasortMainWindow*)parentWidget();
    //QString rhdfilename = parentwindow->getFileName();

    QString fileName = QFileDialog::getOpenFileName(this, "Load data:", "", "data file (*_data.txt);;ASCII (*.txt);;All Files (*)");
    if (fileName.isEmpty()){
        return;
    } else {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, "Unable to load file", file.errorString());
            return;
        }

        int filebytesize = file.size();

        QVector<QVector<double>> tmptutornum;
        QVector<QVector<double>> tmppredictnum;
        QVector<QString> tmptutorpaths;
        QVector<int> tmpelectrodearray;
        tmpelectrodearray.resize(nChannel);
        // !!! in the future, this might will be a problem

        int parammap[paramnames.length()];
        for (int ii=0; ii<paramnames.length(); ++ii){
            parammap[ii] = -1;
        }
        tmptutornum.resize(nChannel);
        tmppredictnum.resize(nChannel);
        QString line;
        QStringList strin;

        QTextStream in(&file);
        if (in.readLine() != "masasortdata"){
            file.close();
            QMessageBox::information(this, "Error", "Different format. Open data file.");
            return;
        }
        QString rhdfilename = in.readLine();
        QString rhdpathname = in.readLine();
        line = in.readLine();
        strin = line.split("\t"); // tutorpaths
        for (int ii=0; ii<strin.length(); ++ii){
            tmptutorpaths.append(strin[ii]);
        }
        line = in.readLine();
        strin = line.split("\t"); // parameternames in QStringList
        for (int ii=0; ii<strin.length(); ++ii){
            for (int jj=0; jj<paramnames.length(); ++jj){
                if (strin.at(ii).compare(paramnames[jj]) == 0){
                    parammap[jj] = ii;
                    continue;
                }
            }
        }
        for (int ii=0; ii<nChannel; ++ii){
            line = in.readLine();
            strin = line.split("\t");
            int indp = 0;
            if (parammap[indp] != -1){
                tmpelectrodearray[ii] = strin.at(parammap[indp]).toInt();
            }
            if (parammap[++indp] != -1){
                spikedetected[ii] = strin.at(parammap[indp]).toInt();
            }
            if (parammap[++indp] != -1){
                threstype[ii] = strin.at(parammap[indp]);
            }
            if (parammap[++indp] != -1){
                thresval[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                stat_ampthresval[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                thresvalmax[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                stat_ampthresvalmax[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                spikeprepeak_init[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                spikepostpeak_init[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                baseprepeak_init[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                basepostpeak_init[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                spikemergethres_init[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                rasterresolution_init[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                highpass_freq[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                highpass_attn[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                highpass_ripple[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                lowpass_freq[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                lowpass_attn[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                lowpass_ripple[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                spike_lowpass_freq[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                spike_lowpass_attn[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                spike_lowpass_ripple[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                spike_realign_thres[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                spike_upsample_scale[ii] = strin.at(parammap[indp]).toInt();
            }
            if (parammap[++indp] != -1){
                trainnormval[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                param_C[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                param_gamma[ii] = strin.at(parammap[indp]).toDouble();
            }
            if (parammap[++indp] != -1){
                strindsspike[ii] = strin.at(parammap[indp]);
            }
        }

        //QMessageBox::information(this, "Unable to load file", "ok2");
        while (!in.atEnd()){
            QString line = in.readLine();
            QStringList strin = line.split("\t");
            for (int ii=0; ii<nChannel; ++ii){
                if (!strin.at(ii*2).isEmpty()){
                    //QMessageBox::information(this, "info", QString::number(ii));
                    tmptutornum[ii].append(strin.at(ii*2).toDouble());
                }
                if (!strin.at(ii*2+1).isEmpty()){
                    //QMessageBox::information(this, "info", QString::number(ii));
                    tmppredictnum[ii].append(strin.at(ii*2+1).toDouble());
                }
            }
        }
        file.close();
        parentwindow->setElectrodeArray(tmpelectrodearray);

        // remove \" because the read QString somehow includes "" in the variable.
        for (int ii=0; ii<nChannel; ++ii){
            if (strindsspike[ii].left(1) == "\"" && strindsspike[ii].right(1) == "\""){
                strindsspike[ii] = strindsspike[ii].split("\"")[1];
            }
        }

        // Open .rhd file
        QStringList dirlists = fileName.split("/");
        dataFileName = dirlists[dirlists.length()-1];
        QString targetPath;
        bool filefound = false;
        dataDir = "";
        for (int ii=0; ii<dirlists.length()-1; ++ii){
            dataDir = dataDir + dirlists[ii] + "/";
            if (QFile::exists(dataDir + rhdfilename)){
                targetPath = dataDir + rhdfilename;
                filefound = true;
            }
        }
        if (filefound){
            parentwindow->openFile(rhdfilename, targetPath);
        } else {
            QUrl fileurl = QFileDialog::getOpenFileUrl(this, "Open", QUrl(rhdfilename), "Recorded file (*.rhd);;All Files (*)");
            QString tmpfilename = fileurl.fileName();
            if (tmpfilename.isEmpty()){
                return;
            }
            QString tmpfilepath = fileurl.path();
            tmpfilepath.remove(0,1);
            parentwindow->openFile(tmpfilename, tmpfilepath);
        }

        if (parentwindow->getFileOpened()){
            parentwindow->setWindowTitle("masasort: " + parentwindow->getFileName() + " < " + dataFileName);
            for (int ii=0; ii<nChannel; ++ii){
                if (spikedetected[ii]){
                    detectSpike(ii, true);
                    tutornum[ii] = tmptutornum[ii];
                    predictnum[ii] = tmppredictnum[ii];
                    resampleSpike(ii);
                }
            }
            for (int ii=0; ii<nChannel; ++ii){
                if (spikedetected[ii]){
                    setFeatures(ii);
                }
            }
        } else {
            QMessageBox::information(this, "Unable to load file", "invalid path");
            return;
        }

        //QMessageBox::information(this, "Unable to load file", QString::number(tmptutorpaths.length()));

        deleteTutorData();
        // Open tutor file
        if (!tmptutorpaths[0].isEmpty()){
            for (int ii=0; ii<tmptutorpaths.length(); ++ii){
                if (QFile::exists(tmptutorpaths[ii])){
                    //QMessageBox::information(this, "exist", tmptutorpaths[ii]);
                    loadTutorData(false, tmptutorpaths[ii]);
                } else {
                    //QMessageBox::information(this, "non-exist", tutorpaths[ii]);
                    QStringList tutordirlists = tmptutorpaths[ii].split("/");
                    QString tutorfilename = tutordirlists[tutordirlists.length()-1];
                    QString tmpdir = "";
                    targetPath = "";
                    filefound = false;
                    for (int jj=0; jj<tutordirlists.length()-1; ++jj){
                        tmpdir = tmpdir + tutordirlists[jj] + "/";
                        if (QFile::exists(tmpdir + tutorfilename)){
                            targetPath = tmpdir + tutorfilename;
                            filefound = true;
                        }
                    }
                    if (filefound){
                        loadTutorData(false, targetPath);
                    } else {
                        tmpdir = "";
                        for (int jj=0; jj<dirlists.length()-1; ++jj){
                            tmpdir = tmpdir + dirlists[jj] + "/";
                            if (QFile::exists(tmpdir + tutorfilename)){
                                targetPath = tmpdir + tutorfilename;
                                filefound = true;
                            }
                        }
                        if (filefound){
                            loadTutorData(false, targetPath);
                        } else {
                            QMessageBox::information(this, "Unable to load tutor file", "invalid path:\n" + tutorpaths[ii]);
                        }
                    }
                }
            }
        }
    }
}

void WidgetIntanChart::setData(QString fileName){
    // load data file and read other than tutornum and predictnum

    if (fileName.isEmpty()){
        return;
    } else {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            //QMessageBox::information(this, "Unable to load file", file.errorString());
            return;
        }

        QString line;
        QStringList strin;
        QVector<int> tmpelectrodearray;
        tmpelectrodearray.resize(nChannel);

        QTextStream in(&file);
        if (in.readLine() != "masasortdata"){
            file.close();
            return;
        }
        QString rhdfilename = in.readLine();
        QString rhdpathname = in.readLine();
        line = in.readLine();
        strin = line.split("\t"); // parameternames in QStringList
        for (int ii=0; ii<nChannel; ++ii){
            line = in.readLine();
            strin = line.split("\t");
            tmpelectrodearray[ii] = strin.at(0).toInt();
            spikedetected[ii] = strin.at(1).toInt();
            threstype[ii] = strin.at(2);
            thresval[ii] = strin.at(3).toDouble();
            thresvalmax[ii] = strin.at(4).toDouble();
            spikeprepeak_init[ii] = strin.at(5).toDouble();
            spikepostpeak_init[ii] = strin.at(6).toDouble();
            baseprepeak_init[ii] = strin.at(7).toDouble();
            basepostpeak_init[ii] = strin.at(8).toDouble();
            spikemergethres_init[ii] = strin.at(9).toDouble();
            rasterresolution_init[ii] = strin.at(10).toDouble();
            highpass_freq[ii] = strin.at(11).toDouble();
            highpass_attn[ii] = strin.at(12).toDouble();
            highpass_ripple[ii] = strin.at(13).toDouble();
            lowpass_freq[ii] = strin.at(14).toDouble();
            lowpass_attn[ii] = strin.at(15).toDouble();
            lowpass_ripple[ii] = strin.at(16).toDouble();
            spike_lowpass_freq[ii] = strin.at(17).toDouble();
            spike_lowpass_attn[ii] = strin.at(18).toDouble();
            spike_lowpass_ripple[ii] = strin.at(19).toDouble();
            spike_realign_thres[ii] = strin.at(20).toDouble();
            spike_upsample_scale[ii] = strin.at(21).toInt();
            trainnormval[ii] = strin.at(22).toDouble();
            param_C[ii] = strin.at(23).toDouble();
            param_gamma[ii] = strin.at(24).toDouble();
            strindsspike[ii] = strin.at(25);
        }
        file.close();
        parentwindow->setElectrodeArray(tmpelectrodearray);
    }

}

void WidgetIntanChart::saveTutorData(){
    //masasortMainWindow* parentwindow = (masasortMainWindow*)parentWidget();
    //QString rhdfilename = parentwindow->getFileName();
    QString rhdfilename = parentwindow->getFileName();
    QString rhdpathname = parentwindow->getFilePathName();
    rhdfilename.chop(4); // remove ".rhd"

    // save a data file that is linked to the tutordata file
    //saveData();

    QString fileName = QFileDialog::getSaveFileName(this, "Save tutor data as:", rhdpathname + "/" + rhdfilename + "_tutor.txt", "Tutor file (*tutor.txt);;ASCII (*.txt);;All Files (*)");
    rhdfilename= rhdfilename + ".rhd";
    if (fileName.isEmpty()){
        QMessageBox::information(this, "Error", "Open a file first");
        return;
    } else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, "Unable to save file", file.errorString());
            return;
        }

        int maxlength = 0;
        for (int ii=0; ii<nChannel; ++ii){
            if (maxlength < tutornum[ii].length()){
                maxlength = tutornum[ii].length();
            }
        }
        if (maxlength < nChannel){
            maxlength = nChannel;
        }
        //QMessageBox::information(this, "info", QString::number(maxlength));

        QVector<int> ntutor;
        ntutor.resize(nChannel);
        // write parametes
        for (int ii=0; ii<nChannel; ++ii){
            ntutor[ii] = 0;
            for (int jj=0; jj<tutornum[ii].length(); ++jj){
                if (tutornum[ii][jj] >= 0){
                    ntutor[ii] += 1;
                }
            }
        }
        QVector<int>& electrodearray = parentwindow->getElectrodeArray();

        QTextStream out(&file);
        out << "masasorttutor" << "\r\n";
        out << "path" << "\t";
        out << "Chname" << "\t";
        out << "Spikename" << "\t";
        out << "tutornum" << "\t";
        out << "electrodearray" << "\t";
        out << "threstype" << "\t";
        out << "thresval" << "\t";
        out << "stat_ampthresval" << "\t";
        out << "thresvalmax" << "\t";
        out << "stat_ampthresvalmax" << "\t";
        out << "spikeprepeak_init" << "\t";
        out << "spikepostpeak_init" << "\t";
        out << "highpass_freq" << "\t";
        out << "highpass_attn" << "\t";
        out << "highpass_ripple" << "\t";
        out << "lowpass_freq" << "\t";
        out << "lowpass_attn" << "\t";
        out << "lowpass_ripple" << "\t";
        out << "spike_lowpass_freq" << "\t";
        out << "spike_lowpass_attn" << "\t";
        out << "spike_lowpass_ripple" << "\t";
        out << "spike_realign_thres" << "\t";
        out << "spike_upsample_scale" << "\t";
        out << "trainnormval" << "\t";
        out << "strindsspike" << "\t";
        out << "featuresize" << "\r\n";
        for (int ii=0; ii<nChannel; ++ii){
            for (int jj=0; jj<tutornum[ii].length(); ++jj){
                if (tutornum[ii][jj] >= 0){
                    //if (!dataDir.isEmpty() && !dataFileName.isEmpty()){
                    //    out << dataDir << dataFileName << "\t";
                    //}
                    out << rhdpathname << "\t";
                    out << ii+1 << "\t"; // intan Ch name
                    out << jj+1 << "\t"; // #spike
                    out << tutornum[ii][jj] << "\t";

                    // parameters
                    out << electrodearray[ii] << "\t";
                    out << threstype[ii] << "\t";
                    out << thresval[ii] << "\t";
                    out << stat_ampthresval[ii] << "\t";
                    out << thresvalmax[ii] << "\t";
                    out << stat_ampthresvalmax[ii] << "\t";
                    out << spikeprepeak_init[ii] << "\t";
                    out << spikepostpeak_init[ii] << "\t";
                    out << highpass_freq[ii] << "\t";
                    out << highpass_attn[ii] << "\t";
                    out << highpass_ripple[ii] << "\t";
                    out << lowpass_freq[ii] << "\t";
                    out << lowpass_attn[ii] << "\t";
                    out << lowpass_ripple[ii] << "\t";
                    out << spike_lowpass_freq[ii] << "\t";
                    out << spike_lowpass_attn[ii] << "\t";
                    out << spike_lowpass_ripple[ii] << "\t";
                    out << spike_realign_thres[ii] << "\t";
                    out << spike_upsample_scale[ii] << "\t";
                    out << trainnormval[ii] << "\t";
                    out << strindsspike[ii] << "\t";
                    out << events[ii][jj].length()*(nadditionalfeatures+1) << "\t";
                    for (int kk=0; kk<events[ii][jj].length(); ++kk){
                        if (nadditionalfeatures == 0 && kk ==events[ii][jj].length()-1){
                            out << events[ii][jj][kk] << "\r\n";
                        } else {
                            out << events[ii][jj][kk] << "\t";
                        }
                    }
                    for (int kk=0; kk<sspike[ii].length(); ++kk){
                        for (int ll=0; ll<sspike[ii][kk][jj].length(); ++ll){
                            if (kk == sspike[ii].length()-1 && ll == sspike[ii][kk][jj].length()-1){
                                out << sspike[ii][kk][jj][ll] << "\r\n";
                            } else {
                                out << sspike[ii][kk][jj][ll] << "\t";
                            }
                        }
                    }
                }
            }
        }

        for (int ii=0; ii<addTutor_tutornum.length(); ++ii){
            if (addTutor_tutornum[ii] >= 0){
                out << addTutor_path[ii] << "\t";
                out << addTutor_indCh[ii]+1 << "\t"; // intan Ch name
                out << addTutor_indSpike[ii]+1 << "\t"; // #spike
                out << addTutor_tutornum[ii] << "\t";

                // parameters
                out << electrodearray[addTutor_indCh[ii]] << "\t";
                out << threstype[addTutor_indCh[ii]] << "\t";
                out << thresval[addTutor_indCh[ii]] << "\t";
                out << stat_ampthresval[addTutor_indCh[ii]] << "\t";
                out << thresvalmax[addTutor_indCh[ii]] << "\t";
                out << stat_ampthresvalmax[addTutor_indCh[ii]] << "\t";
                out << spikeprepeak_init[addTutor_indCh[ii]] << "\t";
                out << spikepostpeak_init[addTutor_indCh[ii]] << "\t";
                out << highpass_freq[addTutor_indCh[ii]] << "\t";
                out << highpass_attn[addTutor_indCh[ii]] << "\t";
                out << highpass_ripple[addTutor_indCh[ii]] << "\t";
                out << lowpass_freq[addTutor_indCh[ii]] << "\t";
                out << lowpass_attn[addTutor_indCh[ii]] << "\t";
                out << lowpass_ripple[addTutor_indCh[ii]] << "\t";
                out << spike_lowpass_freq[addTutor_indCh[ii]] << "\t";
                out << spike_lowpass_attn[addTutor_indCh[ii]] << "\t";
                out << spike_lowpass_ripple[addTutor_indCh[ii]] << "\t";
                out << spike_realign_thres[addTutor_indCh[ii]] << "\t";
                out << spike_upsample_scale[addTutor_indCh[ii]] << "\t";
                out << trainnormval[addTutor_indCh[ii]] << "\t";
                out << strindsspike[addTutor_indCh[ii]] << "\t";
                out << addTutor_features[ii].length() << "\t";
                for (int jj=0; jj<addTutor_features[ii].length(); ++jj){
                    if (jj == addTutor_features[ii].length()-1){
                        out << addTutor_features[ii][jj] << "\r\n";
                    } else {
                        out << addTutor_features[ii][jj] << "\t";
                    }
                }
            }
        }
        file.close();
    }
}

void WidgetIntanChart::deleteTutorData(){
    tutorpaths.clear();
    addTutor_path.clear();
    addTutor_indCh.clear();
    addTutor_indSpike.clear();
    addTutor_tutornum.clear();
    addTutor_features.clear();
    updateSpikeID();
}


void WidgetIntanChart::loadTutorData(bool overwriteparams, QString fileName){
    //masasortMainWindow* parentwindow = (masasortMainWindow*)parentWidget();
    //QString rhdfilename = parentwindow->getFileName();

    if (fileName.isEmpty()){
        fileName = QFileDialog::getOpenFileName(this, "Load tutor data:", "", "Tutor file (*tutor.txt);;ASCII (*.txt);;All Files (*)");
    }
    if (fileName.isEmpty()){
        return;
    } else {
        QString rhdfilename = parentwindow->getFileName();

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, "Unable to open tutor file", file.errorString());
            return;
        }

        QVector<double> addTutor_feature;
        int filebytesize = file.size();

        QString line;
        bool nonmatch = false;
        int featuresize;

        QTextStream in(&file);
        if (in.readLine() != "masasorttutor"){
            file.close();
            QMessageBox::information(this, "Error", "Different format. Open tutor file.");
            return;
        }
        line = in.readLine();
        QStringList strin = line.split("\t");

        if (strin.at(0).compare("path") != 0){
            // this is electrode array
            QVector<int> elecarray;
            elecarray.resize(strin.length());
            for (int ii=0; ii<strin.length(); ++ii){
                elecarray[ii] = strin.at(ii).toInt();
            }
            parentwindow->setElectrodeArray(elecarray);
            line = in.readLine();
        }

        QVector<boolean> foundfilename;
        QVector<int> tmpelectrodearray;
        tmpelectrodearray.resize(nChannel);

        while (!in.atEnd()){
            line = in.readLine();
            QStringList strin = line.split("\t");
            int indp = 0;
            foundfilename.append(false);
            //addTutor_datafile.append(strin.at(0));
            // THESE VARIABLES SHOULD BE INTEGRATED TO ADDTUTOR_PATH2 (but what is ADDTUTOR_PATH2???)
            addTutor_path.append(strin.at(indp));

            QStringList dirlists = addTutor_path[addTutor_path.length()-1].split("/");
            QString tutorfilename = dirlists[dirlists.length()-1];
            if (tutorfilename.compare(rhdfilename) == 0){ // tutor file includes this file
                foundfilename[foundfilename.length()-1] = true;
                //QMessageBox::information(this, "Error", "Result\n" + rhdfilename + "\n" + tutorfilename);
            }

            int indCh = strin.at(++indp).toInt() - 1;
            addTutor_indCh.append(indCh);
            addTutor_indSpike.append(strin.at(++indp).toInt() - 1);
            addTutor_tutornum.append(strin.at(++indp).toInt());
            //parameters
            if (overwriteparams || foundfilename[foundfilename.length()-1]){
                tmpelectrodearray[indCh] = strin.at(++indp).toInt();
                threstype[indCh] = strin.at(++indp);
                thresval[indCh] = strin.at(++indp).toDouble();
                stat_ampthresval[indCh] = strin.at(++indp).toDouble();
                thresvalmax[indCh] = strin.at(++indp).toDouble();
                stat_ampthresvalmax[indCh] = strin.at(++indp).toDouble();
                spikeprepeak_init[indCh] = strin.at(++indp).toDouble();
                spikepostpeak_init[indCh] = strin.at(++indp).toDouble();
                highpass_freq[indCh] = strin.at(++indp).toDouble();
                highpass_attn[indCh] = strin.at(++indp).toDouble();
                highpass_ripple[indCh] = strin.at(++indp).toDouble();
                lowpass_freq[indCh] = strin.at(++indp).toDouble();
                lowpass_attn[indCh] = strin.at(++indp).toDouble();
                lowpass_ripple[indCh] = strin.at(++indp).toDouble();
                spike_lowpass_freq[indCh] = strin.at(++indp).toDouble();
                spike_lowpass_attn[indCh] = strin.at(++indp).toDouble();
                spike_lowpass_ripple[indCh] = strin.at(++indp).toDouble();
                spike_realign_thres[indCh] = strin.at(++indp).toDouble();
                spike_upsample_scale[indCh] = strin.at(++indp).toInt();
                trainnormval[indCh] = strin.at(++indp).toDouble();
                strindsspike[indCh] = strin.at(++indp);
                featuresize = strin.at(++indp).toInt();
                // remove \" because the read QString somehow includes "" in the variable.
                if (strindsspike[indCh].left(1) == "\"" && strindsspike[indCh].right(1) == "\""){
                    strindsspike[indCh] = strindsspike[indCh].split("\"")[1];
                }
            } else {
                if (!nonmatch){
                    if (spikedetected[indCh] && threstype[indCh].compare("autoval") != 0){
                        if (tmpelectrodearray[indCh] != strin.at(++indp).toInt()){
                            nonmatch = true;
                        }
                        if (threstype[indCh] != strin.at(++indp)){
                            nonmatch = true;
                        }
                        if (thresval[indCh] != strin.at(++indp).toDouble()){
                            nonmatch = true;
                        }
                        if (thresvalmax[indCh] != strin.at(++ ++indp).toDouble()){
                            nonmatch = true;
                        }
                        if (spikeprepeak_init[indCh] != strin.at(++ ++indp).toDouble()){
                            nonmatch = true;
                        }
                        if (spikepostpeak_init[indCh] != strin.at(++indp).toDouble()){
                            nonmatch = true;
                        }
                        if (highpass_freq[indCh] != strin.at(++indp).toDouble()){
                            nonmatch = true;
                        }
                        if (highpass_attn[indCh] != strin.at(++indp).toDouble()){
                            nonmatch = true;
                        }
                        if (highpass_ripple[indCh] != strin.at(++indp).toDouble()){
                            nonmatch = true;
                        }
                        if (lowpass_freq[indCh] != strin.at(++indp).toDouble()){
                            nonmatch = true;
                        }
                        if (lowpass_attn[indCh] != strin.at(++indp).toDouble()){
                            nonmatch = true;
                        }
                        if (lowpass_ripple[indCh] != strin.at(++indp).toDouble()){
                            nonmatch = true;
                        }
                        if (spike_lowpass_freq[indCh] != strin.at(++indp).toDouble()){
                            nonmatch = true;
                        }
                        if (spike_lowpass_attn[indCh] != strin.at(++indp).toDouble()){
                            nonmatch = true;
                        }
                        if (spike_lowpass_ripple[indCh] != strin.at(++indp).toDouble()){
                            nonmatch = true;
                        }
                        if (spike_realign_thres[indCh] != strin.at(++indp).toDouble()){
                            nonmatch = true;
                        }
                        if (spike_upsample_scale[indCh] != strin.at(++indp).toInt()){
                            nonmatch = true;
                        }
                        if (trainnormval[indCh] != strin.at(++indp).toDouble()){
                            // *** trainnormval may be 1000 (DEFAULT) or auto value (MAX) set in runAutoTrainin. No problem if values are different.
                            // nonmatch = true;
                        }
                        if (strindsspike[indCh] != strin.at(++indp)){
                            nonmatch = true;
                        }
                    }
                }
            }

            addTutor_feature.clear();
            for (int ii=26; ii<strin.length(); ++ii){
                addTutor_feature.append(strin.at(ii).toDouble());
                //QMessageBox::information(this, "Warning", QString::number(ii));
            }
            addTutor_features.append(addTutor_feature);
        }
        file.close();

        if (overwriteparams){
            // *** I think this is not necessary 2/15/2018
            //parentwindow->setElectrodeArray(tmpelectrodearray);
        }

        // *** Check if tutor data files include rhdfilename. Then, integrate them to tutornum
        for (int ii=0; ii<nChannel; ++ii){
            boolean flag_found = false;
            for (int jj=0; jj<foundfilename.length(); ++jj){
                if (foundfilename[jj]){
                    flag_found = true;
                    break;
                }
            }
            // !!! There might be a bug around here,
            if (flag_found && !spikedetected[ii]){
                detectSpike(ii, true);
                if (spikedetected[ii] && eventpnts[ii].length() > 0){
                    //QMessageBox::information(this, "!info", "num " + QString::number(jj));
                    resampleSpike(ii);
                    setFeatures(ii);
                }
            }
        }
        for (int ii=foundfilename.length()-1; ii>=0; --ii){
            if (foundfilename[ii]){
                tutornum[addTutor_indCh[ii]][addTutor_indSpike[ii]] = addTutor_tutornum[ii];
                addTutor_path.removeAt(ii);
                addTutor_indCh.removeAt(ii);
                addTutor_indSpike.removeAt(ii);
                addTutor_tutornum.removeAt(ii);
                addTutor_features.removeAt(ii);
                foundfilename.removeAt(ii);
            }
        }

        tutorpaths.append(fileName);
        if (nonmatch){
            QMessageBox::information(this, "Warning", "Some tutor files seem to have different parameters.");
        }

        //QMessageBox::information(this, "Warning", QString::number(addTutor_tutornum.length()));
        // ** Is this necessary???
        for (int ii=0; ii<addTutor_tutornum.length(); ++ii){
            addTutor_path2[addTutor_indCh[ii]].append(addTutor_path[ii]);
            addTutor_indSpike2[addTutor_indCh[ii]].append(addTutor_indSpike[ii]);
            addTutor_tutornum2[addTutor_indCh[ii]].append(addTutor_tutornum[ii]);
            addTutor_features2[addTutor_indCh[ii]].append(addTutor_features[ii]);
        }
        updateSpikeID();
    }
}



void WidgetIntanChart::setShowAllSpike(bool showall){
    if (windowfeature){
        windowfeature->setAllSpike(showall);
    }
    // change scatterpoints for detect
    int indCh = window2->getSpikeCh();

    drawDetect(indCh);
}

void WidgetIntanChart::setShowAddedTutor(){
    int lenaddtutor = addTutor_tutornum.length();
    if (lenaddtutor <= 0){
        return;
    }
}

void WidgetIntanChart::setTutorBox(int numTutor){
    if (windowfeature && window2){
        windowfeature->setTutorBox(numTutor, window2->getShowNoise(), window2->getShowTutor());
    }
}

void WidgetIntanChart::calcHitSpike(QPointF pos, int indChart){
    int indCh = indChart + stChart;
    double errorval = 5;
    double val;
    QVector<int> tmphit;
    QVector<bool> showtutor;
    bool shownoise;
    bool showAllSpike;
    bool showthis;

    QVector<double>& datat = parentwindow->getDatat();
    if (window2){
        showtutor = window2->getShowTutor();
        shownoise = window2->getShowNoise();
        showAllSpike = window2->getShowAllSpike();
    }
    for (int ii=eventpnts[indCh].length()-1; ii>=0; --ii){
        if (window2){
            if (showAllSpike){
                if ((int)predictnum[indCh][ii] >= 0){
                    if (!showtutor[(int)predictnum[indCh][ii]]){
                        continue;
                    }
                }
            } else {
                if ((int)tutornum[indCh][ii] >= 0){
                    if (!showtutor[(int)tutornum[indCh][ii]]){
                        continue;
                    }
                }
            }
        }
        val = abs(pos.x() - datat[eventpnts[indCh][ii]]) * abs(pos.y()  - eventpeakvals[indCh][ii]);
        if ((abs(pos.x() - datat[eventpnts[indCh][ii]]) < (axesX[indChart]->max() - axesX[indChart]->min()) / 1000*errorval) && (abs(pos.y()  - eventpeakvals[indCh][ii]) < ((axesY[indChart]->max() - axesY[indChart]->min()) / 100*errorval))){
            tmphit.append(ii);
            break;
        }
    }
    setHitiis(tmphit, indChart);
}

void WidgetIntanChart::drawDetect(int indCh){
    QVector<double>& datat = parentwindow->getDatat();
    QVector<bool> showtutor;
    bool shownoise;
    bool showAllSpike;
    if (window2){
        showtutor = window2->getShowTutor();
        shownoise = window2->getShowNoise();
        showAllSpike = window2->getShowAllSpike();
    }
    for (int ii=0; ii<nChart; ++ii){
        if (indCh == ii+stChart){
            seriesthres[ii]->clear();
            seriesthres[ii]->append(datat[0], stat_ampthresval[indCh]);
            seriesthres[ii]->append(datat[datat.length()-1], stat_ampthresval[indCh]);
            for(int jj=0; jj<maxSpike; ++jj){
                seriesdetect[ii*maxSpike+jj]->clear();
            }
            //QMessageBox::information(this, "hit!!", QString::number(hitii));
            //QMessageBox::information(this, "ii!!", QString::number(ii));
            if (window2){
                if (showAllSpike){
                    for(int jj=0; jj<predictnum[indCh].length(); ++jj){
                        boolean flaghitii = false;
                        for (int kk=0; kk<hitiis.length(); ++kk){
                            if (jj == hitiis[kk]){
                                //QMessageBox::information(this, "hit1!!", QString::number(indCh));
                                seriesdetect[ii*maxSpike+11]->append(datat[eventpnts[indCh][jj]], eventpeakvals[indCh][jj]);
                                flaghitii = true;
                                break;
                            }
                        }
                        if (!flaghitii){
                            if (predictnum[indCh][jj] >= 0){
                                if (showtutor[predictnum[indCh][jj]]){
                                    seriesdetect[ii*maxSpike+(int)predictnum[indCh][jj]]->append(datat[eventpnts[indCh][jj]], eventpeakvals[indCh][jj]);
                                }
                            } else {
                                if (shownoise){
                                    seriesdetect[ii*maxSpike+10]->append(datat[eventpnts[indCh][jj]], eventpeakvals[indCh][jj]);
                                }
                            }
                        }
                    }
                } else {
                    //QMessageBox::information(this, "tutornumlen", QString::number(tutornum[indCh].length()));
                    //QMessageBox::information(this, "eventpntslen", QString::number(eventpnts[indCh].length()));
                    for(int jj=0; jj<tutornum[indCh].length(); ++jj){
                        boolean flaghitii = false;
                        for (int kk=0; kk<hitiis.length(); ++kk){
                            if (jj == hitiis[kk]){
                                //QMessageBox::information(this, "hit1!!", QString::number(indCh));
                                seriesdetect[ii*maxSpike+11]->append(datat[eventpnts[indCh][jj]], eventpeakvals[indCh][jj]);
                                flaghitii = true;
                                break;
                            }
                        }
                        if (!flaghitii){
                            if (tutornum[indCh][jj] >= 0){
                                if (showtutor[tutornum[indCh][jj]]){
                                    seriesdetect[ii*maxSpike+(int)tutornum[indCh][jj]]->append(datat[eventpnts[indCh][jj]], eventpeakvals[indCh][jj]);
                                }
                            } else {
                                if (shownoise) {
                                    seriesdetect[ii*maxSpike+10]->append(datat[eventpnts[indCh][jj]], eventpeakvals[indCh][jj]);
                                }
                            }
                        }
                    }
                }
            } else {
                for(int jj=0; jj<tutornum[indCh].length(); ++jj){
                    boolean flaghitii = false;
                    for (int kk=0; kk<hitiis.length(); ++kk){
                        if (jj == hitiis[kk]){
                            //QMessageBox::information(this, "hit3!!", QString::number(indCh));
                            seriesdetect[ii*maxSpike+11]->append(datat[eventpnts[indCh][jj]], eventpeakvals[indCh][jj]);
                            flaghitii = true;
                            break;
                        }
                    }
                    if (!flaghitii){
                        seriesdetect[ii*maxSpike+10]->append(datat[eventpnts[indCh][jj]], eventpeakvals[indCh][jj]);
                    }
                }
            }
        }
    }
}

QVector<QVector<double>> WidgetIntanChart::getSpikeTime(bool startwithzero, bool writeIfTutor){
    // startwithzero, t[0] = 0s, or use the datat
    QVector<QVector<double>> tspikes;
    QVector<QVector<double>> tspike;
    QVector<double>& datat = parentwindow->getDatat();
    tspike.resize(10);

    int offset = datat[0];
    for (int ii=0; ii<nChannel; ++ii){
        for (int jj=0; jj<predictnum[ii].length(); ++jj){
            if (predictnum[ii][jj] >= 1){
                if (startwithzero){
                    tspike[predictnum[ii][jj]].append(datat[eventpnts[ii][jj]] - offset);
                } else {
                    tspike[predictnum[ii][jj]].append(datat[eventpnts[ii][jj]]);
                }
            }
        }

        if (writeIfTutor){
            if (tutordetected[ii]){
                for (int jj=1; jj<10; ++jj){
                    for (int kk=0; kk<addTutor_tutornum.length(); ++kk){
                        if (ii == addTutor_indCh[kk] && jj == addTutor_tutornum[kk]){
                            // QMessageBox::information(this, "hit3!!", QString::number(ii) + "," + QString::number(jj) + "," + QString::number(addTutor_tutornum[kk]));
                            tspikes.append(tspike[jj]);
                            break;
                        }
                    }
                    tspike[jj].clear();
                }
            }
        } else {
            for (int jj=1; jj<10; ++jj){
                if (tspike[jj].length() > 0){
                    tspikes.append(tspike[jj]);
                }
                tspike[jj].clear();
            }
        }
    }
    return tspikes;
}

void WidgetIntanChart::saveSpikeTime(QString fileName){

    QString rhdfilename = parentwindow->getFileName();
    QString rhdpathname = parentwindow->getFilePathName();
    rhdfilename.chop(4); // remove ".rhd"

    bool writeIfTutor = true; // *** DEFAULT

    QVector<QVector<double>> tspikes;
    tspikes = getSpikeTime(true, writeIfTutor);

    int maxNumSpike=0;
    for (int ii=0; ii<tspikes.length(); ++ii){
        if (tspikes[ii].length() > maxNumSpike){
            maxNumSpike = tspikes[ii].length();
        }
    }

    QVector<QString> spikename;
    for (int ii=0; ii<nChannel; ++ii){
        //*** Write the data if tutors are detected
        if (writeIfTutor){
            if (tutordetected[ii]){
                for (int jj=1; jj<10; ++jj){
                    for (int kk=0; kk<addTutor_tutornum.length(); ++kk){
                        if (ii == addTutor_indCh[kk] && jj == addTutor_tutornum[kk]){
                            spikename.append("Ch" + QString::number(ii+1) + "_" + QString::number(jj));
                            break;
                        }
                    }
                }
            }
        // *** Write the data only when spikes exist in the
        } else {
            for (int jj=1; jj<10; ++jj){
                for (int kk=0; kk<predictnum[ii].length(); ++kk){
                    if (jj == predictnum[ii][jj]){
                        spikename.append("Ch" + QString::number(ii+1) + "_" + QString::number(jj));
                        break;
                    }
                }
            }
        }
    }

//    QMessageBox::information(this, "savespiketime", fileName);

    if (fileName.isEmpty()){
        fileName = QFileDialog::getSaveFileName(this, "Save a vector as:", rhdfilename + "_tspike.txt", "ASCII (*.txt);;All Files (*)");
    }
    if (fileName.isEmpty()){
        return;
    } else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, "Unable to save file", file.errorString());
            return;
        }
        QTextStream out(&file);
/*
        if (writeIfTutor){
            for (int ii=0; ii<spikename.length(); ++ii){
                if (ii < spikename.length() - 1){
                    out << spikename[ii] << "\t";
                } else {
                    out << spikename[ii] << "\r\n";
                }
            }
            for (int ii=0; ii<maxNumSpike; ++ii){
                for (int jj=0; jj<spikename.length(); ++jj){
                    if (tspikes[jj].length() > ii){
                        if (jj < spikename.length() - 1){
                            out << tspikes[jj][ii] << "\t";
                        } else {
                            out << tspikes[jj][ii] << "\r\n";
                        }
                    } else {
                        if (jj < spikename.length() - 1){
                            out << "\t";
                        } else {
                            out << "\r\n";
                        }
                    }
                }
            }
        } else { */
            for (int ii=0; ii<spikename.length(); ++ii){
                if (ii < spikename.length() - 1){
                    out << spikename[ii] << "\t";
                } else {
                    out << spikename[ii] << "\r\n";
                }
            }
            for (int ii=0; ii<maxNumSpike; ++ii){
                for (int jj=0; jj<spikename.length(); ++jj){
                    if (tspikes[jj].length() > ii){
                        if (jj < spikename.length() - 1){
                            out << tspikes[jj][ii] << "\t";
                        } else {
                            out << tspikes[jj][ii] << "\r\n";
                        }
                    } else {
                        if (jj < spikename.length() - 1){
                            out << "\t";
                        } else {
                            out << "\r\n";
                        }
                    }
                }
            }
//        }
        file.close();
    }

}



void WidgetIntanChart::saveSpikes(QString fileName){

    QString rhdfilename = parentwindow->getFileName();
    QString rhdpathname = parentwindow->getFilePathName();
    rhdfilename.chop(4); // remove ".rhd"

    int maxNumSpike=0;
    int lenspikename=0;

    QVector<QVector<double>> allevents;
    QVector<QVector<QVector<double>>> allevent;
    allevent.resize(10);
    QVector<QString> spikename;

    for (int ii=0; ii<nChannel; ++ii){
        for (int jj=0; jj<predictnum[ii].length(); ++jj){
            if (predictnum[ii][jj] >= 1){
                if (predictnum[ii][jj] >= 9){
                    //QMessageBox::information(this, "pred ii", QString::number(ii));
                    //QMessageBox::information(this, "pred jj", QString::number(jj));
                    //QMessageBox::information(this, "pred ", QString::number(predictnum[ii][jj]));
                }
                allevent[predictnum[ii][jj]].append(events[ii][jj]);
            }
        }
        //QMessageBox::information(this, "savespiketime2", QString::number(ii));
        for (int jj=0; jj<10; ++jj){
            if (allevent[jj].length() > 0){ // there are spikes for this unit
                for (int kk=0; kk<allevent[jj].length(); ++kk){
                    allevents.append(allevent[jj][kk]);
                    if (allevent[jj][kk].length() > maxNumSpike){
                        maxNumSpike = allevent[jj][kk].length();
                    }
                    spikename.append("Ch" + QString::number(ii+1) + "_" + QString::number(jj) + "_spk" + QString::number(kk));
                    lenspikename += 1;
                }
                // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! there are no spikes in this unit
//            } else if (){
//
            }
            allevent[jj].clear();
        }
    }

//    QMessageBox::information(this, "savespiketime", fileName);

    if (fileName.isEmpty()){
        fileName = QFileDialog::getSaveFileName(this, "Save a vector as:", rhdfilename + "_spk.txt", "ASCII (*.txt);;All Files (*)");
    }
    if (fileName.isEmpty()){
        return;
    } else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, "Unable to save file", file.errorString());
            return;
        }
        QTextStream out(&file);

        for (int ii=0; ii<spikename.length(); ++ii){
            if (ii < spikename.length() - 1){
                out << spikename[ii] << "\t";
            } else {
                out << spikename[ii] << "\r\n";
            }
        }
        for (int ii=0; ii<maxNumSpike; ++ii){
            for (int jj=0; jj<spikename.length(); ++jj){
                if (allevents[jj].length() > ii){
                    if (jj < spikename.length() - 1){
                        out << allevents[jj][ii] << "\t";
                    } else {
                        out << allevents[jj][ii] << "\r\n";
                    }
                } else {
                    if (jj < spikename.length() - 1){
                        out << "\t";
                    } else {
                        out << "\r\n";
                    }
                }
            }
        }
        file.close();
    }

}


void WidgetIntanChart::saveVectorDouble(QString fileName, QVector<double> savevector){
    if (fileName.isEmpty()){
        fileName = QFileDialog::getSaveFileName(this, "Save a vector as:", "vector.txt", "ASCII (*.txt);;All Files (*)");
    }
    if (fileName.isEmpty()){
        return;
    } else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, "Unable to save file", file.errorString());
            return;
        }
        QTextStream out(&file);
        for (int ii=0; ii<savevector.length(); ++ii){
            out << savevector[ii] << "\r\n";
        }
        file.close();
    }
}
