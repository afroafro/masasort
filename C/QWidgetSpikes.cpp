#include "QWidgetSpikes.h"
#include <QApplication>
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QIODevice>
#include <QMessageBox>
#include <QtCharts>
#include <QChartViewSpikes.h>
#include <time.h>

using namespace std;
using namespace QtCharts;

//QChartViewSpikes::QChartViewSpikes(QChart *chart, QWidget *parent) : QChartView(chart, parent){
QWidgetSpikes::QWidgetSpikes(QWidget *parent) : QWidget(parent, Qt::Window){
    mainLayout2 = new QGridLayout;
    QMenuBar *menuBar = new QMenuBar;
    QMenu* fileMenu = new QMenu("&File", this);
    menuBar->addMenu(fileMenu);
    QAction* openAction = new QAction("Save spikes");
    openAction->setData(0);
    fileMenu->addAction(openAction);
    mainLayout2->setMenuBar(menuBar);
    connect(fileMenu, SIGNAL(triggered(QAction*)), this, SLOT(pushFile(QAction*)));
    spikechart = new QChart();
    spikechart->legend()->hide();
    spikechart->setMargins(QMargins(0,0,0,0));
    spikechart->setMinimumHeight(20.0);
    calchitspikebusy = false;
    axisYmin = 0;
    axisYmax = 0;
    lentutor = 10;

    // *** window size
    int window_w = 400;
    int window_h = 600;

    parentwidget = (WidgetIntanChart*)parentWidget();

    int indVlayout = 0;
    showAllSpike = false;

    linepenspike = parentwidget->getLinepenspike();
    linepencurrent =  parentwidget->getLinepencurrent();
    linepencursor = parentwidget->getLinepencursor();
    linepentutor =  parentwidget->getLinepentutor();
    linepennoise =  parentwidget->getLinepennoise();

    QGroupBox *radiobox = new QGroupBox();
    QHBoxLayout *radiolayout = new QHBoxLayout();
    radiobutton0 = new QRadioButton(radiobox);
    radiobutton1 = new QRadioButton(radiobox);
    radiobutton2 = new QRadioButton(radiobox);
    radiobutton0->setChecked(true);
    radiobutton0->setText("tutor");
    radiobutton1->setText("all");
    radiobutton2->setText("added");
    QObject::connect(radiobutton0, SIGNAL(released()), this, SLOT(pushtutorSpike()));
    QObject::connect(radiobutton1, SIGNAL(released()), this, SLOT(pushallSpike()));
    QObject::connect(radiobutton2, SIGNAL(released()), this, SLOT(pushAddedTutor()));
    radiolayout->addWidget(radiobutton0);
    radiolayout->addWidget(radiobutton1);
    radiolayout->addWidget(radiobutton2);
    radiobox->setLayout(radiolayout);
    mainLayout2->addWidget(radiobox, indVlayout, 0);
    indVlayout += 1;

    //QValueAxis *axesX2 = new QValueAxis;
    //chart2->addAxis(axesX2, Qt::AlignBottom);
    //QValueAxis *axesY2 = new QValueAxis;
    //chart2->addAxis(axesY2, Qt::AlignLeft);
    chartview2 = new QChartViewSpikes(spikechart, this);
    chartview2->setRenderHint(QPainter::Antialiasing);
    mainLayout2->addWidget(chartview2, indVlayout, 0);
    indVlayout += 1;

    QWidget *tutorboxes = new QWidget(this);
    QGridLayout *tutorboxLayout = new QGridLayout();
    QButtonGroup *tutorboxgroupnoise = new QButtonGroup(tutorboxes);
    tutorRadionoise = new QRadioButton();
    tutorboxgroupnoise->addButton(tutorRadionoise);
    tutorboxgroupnoise->setExclusive(false);
    tutorRadionoise->setChecked(true);
    tutorRadionoise->setText("n/a");
    tutorRadionoise->setEnabled(true);
    QSignalMapper* tutorboxMappernoise = new QSignalMapper(tutorRadionoise);
    QObject::connect(tutorRadionoise, SIGNAL(released()), tutorboxMappernoise, SLOT(map()));
    tutorboxMappernoise->setMapping(tutorRadionoise, -1);
    QObject::connect(tutorboxMappernoise, SIGNAL(mapped(int)), this, SLOT(pushTutorBox(int)));
    shownoise = true;
    tutorboxLayout->addWidget(tutorRadionoise, 0, 0);

    QButtonGroup *tutorboxgroup[lentutor];
    showtutor.resize(lentutor);
    for (int ii=0; ii<lentutor; ++ii){
        tutorboxgroup[ii] = new QButtonGroup(tutorboxes);
        tutorRadio[ii] = new QRadioButton();
        tutorboxgroup[ii]->addButton(tutorRadio[ii]);
        tutorboxgroup[ii]->setExclusive(false);
        tutorRadio[ii]->setChecked(true);
        tutorRadio[ii]->setText(QString::number(ii));
        tutorRadio[ii]->setEnabled(false);
        tutorboxMapper[ii] = new QSignalMapper(tutorRadio[ii]);
        QObject::connect(tutorRadio[ii], SIGNAL(released()), tutorboxMapper[ii], SLOT(map()));
        tutorboxMapper[ii]->setMapping(tutorRadio[ii], ii);
        QObject::connect(tutorboxMapper[ii], SIGNAL(mapped(int)), this, SLOT(pushTutorBox(int)));
        showtutor[ii] = true;
        if (ii < 5){
            tutorboxLayout->addWidget(tutorRadio[ii], 0, ii+1);
        } else {
            tutorboxLayout->addWidget(tutorRadio[ii], 1, ii-5);
        }
    }
    tutorboxes->setLayout(tutorboxLayout);
    mainLayout2->addWidget(tutorboxes, indVlayout, 0);
    indVlayout += 1;

    // buttons
    int nButton = 6;
    QPushButton *buttonSpike[nButton];
    buttonSpike[0] = new QPushButton("showCurrentSpike");
    QObject::connect(buttonSpike[0], SIGNAL(released()), parentWidget(), SLOT(showCurrentSpike()));
    buttonSpike[1] = new QPushButton("removeTrainedSpike");
    QObject::connect(buttonSpike[1], SIGNAL(released()), parentWidget(), SLOT(removeTrainedSpike()));
    buttonSpike[2] = new QPushButton("addFeatures");
    QObject::connect(buttonSpike[2], SIGNAL(released()), parentWidget(), SLOT(addFeatures()));
    buttonSpike[3] = new QPushButton("showFeatures");
    QObject::connect(buttonSpike[3], SIGNAL(released()), parentWidget(), SLOT(showFeatures()));
    buttonSpike[4] = new QPushButton("Register");
    QObject::connect(buttonSpike[4], SIGNAL(released()), parentWidget(), SLOT(registerSpike()));
    buttonSpike[5] = new QPushButton("Train");
    QObject::connect(buttonSpike[5], SIGNAL(released()), parentWidget(), SLOT(pushTrainSpike()));
    for (int ii=0; ii<nButton; ++ii){
        mainLayout2->addWidget(buttonSpike[ii], ii+indVlayout, 0);
    }
    resize(200,400);
    setLayout(mainLayout2);

    lenseries = 0;
    axisX = new QValueAxis;
    axisY = new QValueAxis;
    spikechart->addAxis(axisX, Qt::AlignBottom);
    spikechart->addAxis(axisY, Qt::AlignLeft);
    for (int ii=0; ii<lenseries; ++ii){
        //spikeseries[ii] = new QLineSeries();
        spikeseries.append(new QLineSeries());
        spikechart->addSeries(spikeseries[ii]);
        spikeseries[ii]->attachAxis(axisX);
        spikeseries[ii]->attachAxis(axisY);
    }
    cursorline = new QLineSeries();
    cursorline->setPen(linepencursor);
    spikechart->addSeries(cursorline);
    cursorline->attachAxis(axisX);
    cursorline->attachAxis(axisY);

    this->resize(window_w, window_h);
}

void QWidgetSpikes::setSpikes(QVector<QVector<double>> query_spikes, QVector<double> query_tspike, int query_indCh, QVector<double> query_tutornum, QVector<double> query_predictnum){
    int lenspikes = query_spikes.length();
    if (lenspikes > 5000){
        QMessageBox::information(this, "!!", "discarded because there are 5000 spikes (see C++ code)");
        return;
    }
    spikes = query_spikes;
    indCh = query_indCh;
    tspike = query_tspike;
    hitii = -1;
    tutornum = query_tutornum;
    predictnum = query_predictnum;

    if (lenseries < lenspikes){
        for (int ii=lenseries; ii<lenspikes; ++ii){
            spikeseries.append(new QLineSeries());
            spikechart->addSeries(spikeseries[ii]);
            spikeseries[ii]->attachAxis(axisX);
            spikeseries[ii]->attachAxis(axisY);
        }
        //QMessageBox::information(this, "!!", "only shows 2000 spikes (see C++ code)");
    } else if (lenseries > lenspikes){
        for (int ii=lenseries-1; ii>=lenspikes; --ii){
            spikechart->removeSeries(spikeseries[ii]);
            delete(spikeseries.takeAt(ii));
        }
    }
    //QMessageBox::information(this, "!!size", QString::number(spikeseries.size()));
    //QMessageBox::information(this, "!!lenspike", QString::number(lenspikes));
    lenseries = lenspikes;

    // add additional tutor if exist
    int lenaddtutor = parentwidget->addTutor_tutornum2[indCh].length();
    //QMessageBox::information(this, "!!lenspike", QString::number(lenaddtutor));
    for (int ii=0; ii<lenaddtutor; ++ii){
        spikeseries.append(new QLineSeries());
        spikechart->addSeries(spikeseries[lenseries]);
        spikeseries[lenseries]->attachAxis(axisX);
        spikeseries[lenseries]->attachAxis(axisY);
        lenseries += 1;
    }

    //*** Radio initialize
    for(int ii=0; ii<lentutor; ++ii){
        setTutorRadio(ii, false);
    }
    //*** Enable the buttons as needed
    for(int ii=0; ii<tutornum.length(); ++ii){
        if (tutornum[ii] >= 0){
            setTutorRadio((int)tutornum[ii], true);
        } else if (predictnum[ii] >= 0) {
            setTutorRadio((int)predictnum[ii], true);
        }
    }
    drawSpikes();
}

void QWidgetSpikes::setTutorRadio(int numtutor, bool tutorbool){
    tutorRadio[numtutor]->setEnabled(tutorbool);
}

void QWidgetSpikes::setNoiseRadio(bool noisebool){
    tutorRadionoise->setEnabled(noisebool);
}

int QWidgetSpikes::getSpikeCh(){
    return indCh;
}

void QWidgetSpikes::changeSpikes(){
    int lenall = spikeseries.length();
    int lenspike = spikes.length();
    for (int ii=0; ii<lenall; ++ii){
        spikeseries[ii]->setVisible(false);
    }

    if (radiobutton2->isChecked()){
        // *** Show only additional tutors
        int lenaddtutor =  parentwidget->addTutor_tutornum2[indCh].length();
        if (lenaddtutor > 0){
            QVector<int> addtutortutornum2 = parentwidget->addTutor_tutornum2[indCh];
            QVector<QVector<double>> addtutorfeature2 = parentwidget->addTutor_features2[indCh];

            for (int ii=0; ii<lenaddtutor; ++ii){
                spikeseries[ii+lenspike]->setVisible(false);
                if (ii+lenspike == hitii){ // !!! this will not happen
                    spikeseries[ii+lenspike]->setPen(linepencurrent);
                    spikeseries[ii+lenspike]->setVisible(true);
                } else {
                    if ((int)addtutortutornum2[ii] >= 0){
                        spikeseries[ii+lenspike]->setPen(linepentutor[(int)addtutortutornum2[ii]]);
                        if (showtutor[(int)addtutortutornum2[ii]]){
                            spikeseries[ii+lenspike]->setVisible(true);
                        }
                    } else {
                        spikeseries[ii+lenspike]->setPen(linepenspike);
                        if (shownoise){
                            spikeseries[ii+lenspike]->setVisible(true);
                        }
                    }
                }
            }
        }
    } else {
        // *** Show spikes in this file
        for (int ii=0; ii<lenspike; ++ii){
            boolean flaghitii = false;
            // *** User-selected spike
            for (int jj=0; jj<hitiis.length(); ++jj){
                if (ii == hitiis[jj]){
                    spikeseries[ii]->setPen(linepencurrent);
                    spikeseries[ii]->setVisible(true);
                    flaghitii = true;
                    break;
                }
            }
            // *** Unselected spike
            if (!flaghitii){
                if (showAllSpike){
                    if ((int)predictnum[ii] >= 0){
                        spikeseries[ii]->setPen(linepentutor[(int)predictnum[ii]]);
                        if (showtutor[(int)predictnum[ii]]){
                            spikeseries[ii]->setVisible(true);
                        }
                    } else {
                        spikeseries[ii]->setPen(linepenspike);
                        if (shownoise){
                            spikeseries[ii]->setVisible(true);
                        }
                    }
                } else {
                    if ((int)tutornum[ii] >= 0){
                        spikeseries[ii]->setPen(linepentutor[(int)tutornum[ii]]);
                        if (showtutor[(int)tutornum[ii]]){
                            spikeseries[ii]->setVisible(true);
                        }
                    } else {
    //                    QMessageBox::information(this, "info", QString::number(tutornum[ii]));
                        spikeseries[ii]->setPen(linepenspike);
                        if (shownoise){
                            spikeseries[ii]->setVisible(true);
                        }
                    }
                }
            }
        }
        for (int ii=0; ii<indspikehide.length(); ++ii){
            spikeseries[indspikehide[ii]]->setVisible(false);
        }
    }
}

void QWidgetSpikes::drawSpikes(){

    //for (int ii=0; ii<spikes.length(); ++ii){
    //    spikechart->removeAllSeries(); // !!!! this gives me an error !!!
        //spikechart->removeSeries(spikeseries[ii]); // !!!! this gives me an error !!!
    //}

    int maxy = spikes[0][0];
    int miny = spikes[0][0];
    int lenspike = spikes.length();

    for (int ii=0; ii<lenseries; ++ii){
        spikeseries[ii]->clear();
    }

    for (int ii=0; ii<lenspike; ++ii){
        for (int jj=0; jj<spikes[ii].length(); ++jj){
            spikeseries[ii]->append(tspike[jj], spikes[ii][jj]);
            if (miny > spikes[ii][jj]){
                miny = spikes[ii][jj];
            }
            if (maxy < spikes[ii][jj]){
                maxy = spikes[ii][jj];
            }
        }
    }

    // add additional tutors
    int lenaddtutor =  parentwidget->addTutor_tutornum2[indCh].length();
    //QMessageBox::information(this, "!!lenspike", QString::number(lenaddtutor));

    QVector<int> addtutortutornum2 = parentwidget->addTutor_tutornum2[indCh];
    QVector<QVector<double>> addtutorfeature2 = parentwidget->addTutor_features2[indCh]; // this includes additiona features
    for (int ii=0; ii<lenaddtutor; ++ii){
        for (int jj=0; jj<tspike.length(); ++jj){
            spikeseries[ii+lenspike]->append(tspike[jj], addtutorfeature2[ii][jj]);
            if (miny > addtutorfeature2[ii][jj]){
                miny = addtutorfeature2[ii][jj];
            }
            if (maxy < addtutorfeature2[ii][jj]){
                maxy = addtutorfeature2[ii][jj];
            }
        }
    }

    // START FROM HERE
    /*
    //QMessageBox::information(this, "!!", QString::number(lenseries));
    //QMessageBox::information(this, "!!", QString::number(spikes.length()));
    bool showthis;
//    QMessageBox::information(this, "info", QString::number(spikes.length()));
    for (int ii=0; ii<lenspike; ++ii){
        showthis = false;
        if (ii == hitii){
            spikeseries[ii]->setPen(linepencurrent);
            showthis = true;
        } else {
            if (showAllSpike){
                if ((int)predictnum[ii] >= 0){
                    spikeseries[ii]->setPen(linepentutor[(int)predictnum[ii]]);
                    if (showtutor[(int)predictnum[ii]]){
                        showthis = true;
                    }
                } else {
                    spikeseries[ii]->setPen(linepen);
                    if (shownoise){
                        showthis = true;
                    }
                }
            } else {
                if ((int)tutornum[ii] >= 0){
                    spikeseries[ii]->setPen(linepentutor[(int)tutornum[ii]]);
                    if (showtutor[(int)tutornum[ii]]){
                        showthis = true;
                    }
                } else {
//                    QMessageBox::information(this, "info", QString::number(tutornum[ii]));
                    spikeseries[ii]->setPen(linepen);
                    if (shownoise){
                        showthis = true;
                    }
                }
            }
        }
//        QMessageBox::information(this, "info", "ok");
        for (int jj=0; jj<spikes[ii].length(); ++jj){
            if (showthis){
                spikeseries[ii]->append(tspike[jj], spikes[ii][jj]);
            }
            if (miny > spikes[ii][jj]){
                miny = spikes[ii][jj];
            }
            if (maxy < spikes[ii][jj]){
                maxy = spikes[ii][jj];
            }
        }
    }

    int lenaddtutor =  parentwidget->addTutor_tutornum2[indCh].length();
    if (lenaddtutor > 0){
        QVector<int> addtutortutornum2 = parentwidget->addTutor_tutornum2[indCh];
        QVector<QVector<double>> addtutorfeature2 = parentwidget->addTutor_features2[indCh];
        // additional tutor
        for (int ii=0; ii<lenaddtutor; ++ii){
            showthis = false;
            if (ii == hitii){
                spikeseries[ii+lenspike]->setPen(linepencurrent);
                showthis = true;
            } else {
                if ((int)addtutortutornum2[ii] >= 0){
                    spikeseries[ii+lenspike]->setPen(linepentutor[(int)addtutortutornum2[ii]]);
                    if (showtutor[(int)addtutortutornum2[ii]]){
                        showthis = true;
                    }
                } else {
//                    QMessageBox::information(this, "info", QString::number(tutornum[ii]));
                    spikeseries[ii+lenspike]->setPen(linepen);
                    if (shownoise){
                        showthis = true;
                    }
                }
            }
    //        QMessageBox::information(this, "info", "ok");
            for (int jj=0; jj<addtutorfeature2[ii].length(); ++jj){
                if (showthis){
                    spikeseries[ii]->append(tspike[jj], addtutorfeature2[ii][jj]);
                }
                if (miny > addtutorfeature2[ii][jj]){
                    miny = addtutorfeature2[ii][jj];
                }
                if (maxy < addtutorfeature2[ii][jj]){
                    maxy = addtutorfeature2[ii][jj];
                }
            }
        }
    }
    */
    double axisymax;
    double axisymin;
    double axisxmax = (ceil(tspike[spikes[0].length()-1]/0.1)*0.1);
    double axisxmin = (floor(tspike[0]/0.1)*0.1);
    if (axisYmax == 0 && axisYmin == 0){
        axisymax = (maxy - maxy%100 + 100);
        axisymin = (miny - miny%100 - 100);
    } else {
        axisymax = axisYmax;
        axisymin = axisYmin;
    }
    axisY->setRange(axisymin, axisymax);
    double ticknum;
    // to set a tick to zero, set the greatest common divisor/factor as tick
    ticknum=0.0;
    if (axisymax > -axisymin){
        for (double ii=-axisymin; ii>=100; ii-=100){
            if (axisymax/ii == round(axisymax/ii)){
                ticknum = ii;
                break;
            }
        }
    } else {
        for (int ii=axisymax; ii>=100; ii-=100){
            if (axisymin/ii == round(axisymin/ii)){
                ticknum = ii;
                break;
            }
        }
    }
    if (ticknum == 0.0){
        ticknum = 100;
    }
    axisY->setTickCount((int)((axisymax-axisymin)/ticknum)+1);
    axisY->setGridLineColor(QColor(230,230,230,100));
    if (ticknum > 100){
        axisY->setMinorTickCount((int)(ticknum/100)-1);
        axisY->setMinorGridLineColor(QColor(240,240,240,50));
    }

    axisX->setRange(axisxmin, axisxmax);
    // to set a tick to zero, set the greatest common divisor/factor as tick
    ticknum=0.0;
    if (axisxmax > -axisxmin){
        for (double ii=-axisxmin; ii>=0.1; ii-=0.1){
            if (axisxmax/ii == round(axisxmax/ii)){
                ticknum = ii;
                break;
            }
        }
    } else {
        for (double ii=axisxmax; ii>=0.1; ii-=0.1){
            if (axisxmin/ii == round(axisxmin/ii)){
                ticknum = ii;
                break;
            }
        }
    }
    if (ticknum == 0.0){
        ticknum = 0.1;
    }
    axisX->setTickCount((int)((axisxmax-axisxmin)/ticknum)+1);
    axisX->setGridLineColor(QColor(230,230,230,100));
    //QMessageBox::information(this, "hit!!", QString::number((int)((axisxmax-axisxmin)/ticknum)+1));
    if (ticknum > 0.1){
        axisX->setMinorTickCount((int)(ticknum/0.1)-1);
        axisX->setMinorGridLineColor(QColor(240,240,240,50));
    }

    changeSpikes();
    /*
    time_t t0;
    time_t t1;
    time(&t0);
    time(&t1);
    QMessageBox::information(this, "hit!!", QString::number(difftime(t1,t0)));
    */

//    spikechart->createDefaultAxes();
}

QVector<double> QWidgetSpikes::getTutornum(){
    return tutornum;
}

void QWidgetSpikes::setTutornum(QVector<double> tmptutornum){
    tutornum = tmptutornum;
    if (!showAllSpike){
        updateTutorRadioTutornum();
    }
}

void QWidgetSpikes::setPredictnum(QVector<double> query_predictnum){
    predictnum = query_predictnum;
    if (showAllSpike){
        updateTutorRadioPredictnum();
    }
}

void QWidgetSpikes::updateTutorRadioTutornum(){
    for (int ii=0; ii<lentutor; ++ii){
        setTutorRadio(ii, false);
    }
    setNoiseRadio(false);
    for (int ii=0; ii<tutornum.length(); ++ii){
        if ((int)tutornum[ii] >= 0){
            setTutorRadio((int)tutornum[ii], true);
        } else {
            setNoiseRadio(true);
        }
    }
}

void QWidgetSpikes::updateTutorRadioPredictnum(){
    for (int ii=0; ii<lentutor; ++ii){
        setTutorRadio(ii, false);
    }
    setNoiseRadio(false);
    for (int ii=0; ii<predictnum.length(); ++ii){
        if ((int)predictnum[ii] >= 0){
            setTutorRadio((int)predictnum[ii], true);
        } else {
            setNoiseRadio(true);
        }
    }
}

int QWidgetSpikes::getCurrentSpike(){
    return hitii;
}

void QWidgetSpikes::inputTutornum(int inputnum){
    for (int ii=0; ii<hitiis.length(); ++ii){
        tutornum[hitiis[ii]] = (double)inputnum;
    }
    if (inputnum < 0){
        if (!showAllSpike){
            updateTutorRadioTutornum();
        }
    } else {
        setTutorRadio(inputnum, true);
    }
    changeSpikes();
}

void QWidgetSpikes::keyPressEvent(QKeyEvent *event){

    switch (event->key()){
        case Qt::Key_Minus:
            inputTutornum(-1);
            break;
        case Qt::Key_0:
            inputTutornum(0);
            break;
        case Qt::Key_1:
            inputTutornum(1);
            break;
        case Qt::Key_2:
            inputTutornum(2);
            break;
        case Qt::Key_3:
            inputTutornum(3);
            break;
        case Qt::Key_4:
            inputTutornum(4);
            break;
        case Qt::Key_5:
            inputTutornum(5);
            break;
        case Qt::Key_6:
            inputTutornum(6);
            break;
        case Qt::Key_7:
            inputTutornum(7);
            break;
        case Qt::Key_8:
            inputTutornum(8);
            break;
        case Qt::Key_9:
            inputTutornum(9);
            break;
        case Qt::Key_Delete:
            if (hitiis.length() == 0){
                boolean showhiddenspikes = false;
                if (indspikehide.length() > 0){
                    indspikehidetmp = indspikehide;
                    indspikehide.clear();
                } else {
                    indspikehide = indspikehidetmp;
                }
            } else {
                for (int ii=0; ii<hitiis.length(); ++ii){
                    boolean tmpadd = true;
                    for (int jj=0; jj<indspikehide.length(); ++jj){
                        if (hitiis[ii] == indspikehide[jj]){
                            tmpadd = false;
                            break;
                        }
                    }
                    if (tmpadd){
                        indspikehide.append(hitiis[ii]);
                    }
                }
            }
            changeSpikes();
            break;
        case Qt::Key_Left:
            if (hitiis.length() == 0){
                hitiis.append(spikes.length()-1);
            } else {
                int tmpnum = hitiis[0];
                hitiis.clear();
                if (tmpnum <= 0){
                    hitiis.append(spikes.length()-1);
                } else {
                    hitiis.append(tmpnum - 1);
                }
            }
            changeSpikes();
            break;
        case Qt::Key_Right:
            if (hitiis.length() == 0){
                hitiis.append(0);
            } else {
                int tmpnum = hitiis[0];
                hitiis.clear();
                if (tmpnum >= spikes.length()-1){
                    hitiis.append(0);
                } else {
                    hitiis.append(tmpnum + 1);
                }
            }
            changeSpikes();
            break;
        case Qt::Key_Up:
            /*
            if (event->modifiers() == Qt::ShiftModifier){
                for (int ii=0; ii<spikes[hitii].length(); ++ii){
                    spikes[hitii][ii] += 1;
                }
            } else {
                if (hitii == 0){
                    hitii = spikes.length()-1;
                } else {
                    hitii -= 1;
                }
            }
            changeSpikes();
            */
            break;
        case Qt::Key_Down:
        /*
            if (event->modifiers() == Qt::ShiftModifier){
                for (int ii=0; ii<spikes[hitii].length(); ++ii){
                    spikes[hitii][ii] -= 1;
                }
            } else {
                if (hitii == spikes.length()-1){
                    hitii = 0;
                } else {
                    hitii += 1;
                }
            }
            changeSpikes();
            */
            break;
        case Qt::Key_D:
            if (event->modifiers() == Qt::ControlModifier){
                setAxesY();
            }
            break;
        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

void QWidgetSpikes::setAxesY(){
    QVector<QString> dialoglabel;
    QVector<QString> dialogvalue;
    QVector<QString> returnvalue;
    dialoglabel.append("Y max");
    dialoglabel.append("Y min (reset if max&min == 0)");
    dialogvalue.append(QString::number(axisYmax));
    dialogvalue.append(QString::number(axisYmin));
    returnvalue = parentwidget->myDialog("Y range:", dialoglabel, dialogvalue);
    if (returnvalue.length() == 0){
        return;
    }
    axisYmax = returnvalue[0].toDouble();
    axisYmin = returnvalue[1].toDouble();
    drawSpikes();
}

/*
void QWidgetSpikes::mouseMoveEvent(QMouseEvent *event){
    QWidget::mouseMoveEvent(event);

}

void QWidgetSpikes::mouseReleaseEvent(QMouseEvent *event){

    // Because we disabled animations when touch event was detected
    // we must put them back on.
    //chart()->setAnimationOptions(QChart::SeriesAnimations);

    QWidget::mouseReleaseEvent(event);

}
*/

void QWidgetSpikes::pushFile(QAction *action){
    int value = action->data().toInt();
    if (value == 0){//
        saveSpikes();
    }
}

void QWidgetSpikes::saveSpikes(){
    int spikeCh = getSpikeCh();
    if (tutornum.length() == 0){
        QMessageBox::information(this, "Error", "No training data is detected.");
        return;
    }
    if (predictnum.length() == 0){
        QMessageBox::information(this, "Error", "No classified data is detected.");
        return;
    }

    int numspike = 1;

    QString fileName = QFileDialog::getSaveFileName(this, "Save as", "spike"+QString::number(spikeCh)+".txt", "ASCII (*.txt);;All Files (*)");
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

        QTextStream out(&file);
        for (int ii=0; ii<tutornum.length(); ++ii){
            if (predictnum[ii] == numspike){
                for (int jj=0; jj<spikes[ii].length()-1; ++jj){
                    out << QString::number(spikes[ii][jj]) << "\t";
                }
                out << QString::number(spikes[ii][spikes[ii].length()]) << "\r\n";
                // out << QString::number(tutornum[ii]) << "\t" << QString::number(predictnum[ii]) << "\r\n";
            }
        }
        file.close();

    }

}

void QWidgetSpikes::pushtutorSpike(){
    showAllSpike = radiobutton1->isChecked();
    parentwidget->setShowAllSpike(showAllSpike);
    if (showAllSpike){
        updateTutorRadioPredictnum();
    } else {
        updateTutorRadioTutornum();
    }
    changeSpikes();
}

void QWidgetSpikes::pushallSpike(){
    showAllSpike = radiobutton1->isChecked();
    parentwidget->setShowAllSpike(showAllSpike);
    if (showAllSpike){
        updateTutorRadioPredictnum();
    } else {
        updateTutorRadioTutornum();
    }
    changeSpikes();
}

void QWidgetSpikes::pushAddedTutor(){
    for (int ii=0; ii<lentutor; ++ii){
        setTutorRadio(ii, false);
    }
    setNoiseRadio(false);
    QVector<int> addtutortutornum2 = parentwidget->addTutor_tutornum2[indCh];
    for (int ii=0; ii<addtutortutornum2.length(); ++ii){
        if ((int)addtutortutornum2[ii] >= 0){
            setTutorRadio((int)addtutortutornum2[ii], true);
        } else {
            setNoiseRadio(true);
        }
    }
    changeSpikes();
}


bool QWidgetSpikes::getShowAllSpike(){
    return showAllSpike;
}

void QWidgetSpikes::calcHitSpike(QPointF pos){
    cursorline->clear();
    double errorval = 10;
    double val;
    hitiis.clear();
    int lenspike = tspike.length();
    int stspike = -1;
    for (int ii=0; ii<lenspike-1; ++ii){
        if(tspike[ii] <= pos.x() && pos.x() <= tspike[ii+1]){
            stspike = ii;
            break;
        }
    }
    if (stspike != -1){
        for (int ii=spikes.length()-1; ii>=0; --ii){
            if (showAllSpike){
                if ((int)predictnum[ii] >= 0){
                    if (!showtutor[(int)predictnum[ii]]){
                        continue;
                    }
                }
            } else {
                if ((int)tutornum[ii] >= 0){
                    if (!showtutor[(int)tutornum[ii]]){
                        continue;
                    }
                } else {
                    if (!shownoise){
                        continue;
                    }
                }
            }
            boolean flaghide = 0;
            for (int jj=0; jj<indspikehide.length(); ++jj){
                if (indspikehide[jj] == ii){
                    flaghide = 1;
                    break;
                }
            }
            if(flaghide){
                continue;
            }

            val = (pos.y() - spikes[ii][stspike])*(tspike[stspike+1] - pos.x()) - (spikes[ii][stspike+1] - pos.y())*(pos.x() - tspike[stspike]);
            if ( val < errorval && val > -errorval){
                if (((pos.y() >= spikes[ii][stspike]) && (pos.y() <= spikes[ii][stspike+1]) && (pos.x() >= tspike[stspike]) && (pos.x() <= tspike[stspike+1])) || ((pos.y() <= spikes[ii][stspike]) && (pos.y() >= spikes[ii][stspike+1]) && (pos.x() >= tspike[stspike]) && (pos.x() <= tspike[stspike+1]))){
                    hitiis.append(ii);
                    break;
                }
            }
            if (hitiis.length() > 0){
                break;
            }
        }
    }
    parentwidget->setHitiis(hitiis,indCh);
        //QMessageBox::information(this, "hit!!", "ok");
        //setFocus();
}

void QWidgetSpikes::drawCursorLine(QPointF posPress, QPointF posMove){
    cursorline->clear();
    cursorline->append(posPress);
    cursorline->append(posMove);
}

void QWidgetSpikes::calcHitSpikeDragged(QPointF posP, QPointF posR){
    double errorval = 10;
    double val;
    double minx;
    double maxx;
    double miny;
    double maxy;
    if (posP.x() <= posR.x()){
        minx = posP.x();
        maxx = posR.x();
    } else {
        minx = posR.x();
        maxx = posP.x();
    }
    if (posP.y() <= posR.y()){
        miny = posP.y();
        maxy = posR.y();
    } else {
        miny = posR.y();
        maxy = posP.y();
    }

    hitiis.clear();

    int lenspike = tspike.length();
    int stspike = 0;
    int enspike = lenspike-1;
    for (int ii=0; ii<lenspike-1; ++ii){
        if(tspike[ii] <= minx && minx <= tspike[ii+1]){
            stspike = ii;
        }
        if(tspike[ii] <= maxx && maxx <= tspike[ii+1]){
            enspike = ii+1;
            break;
        }
    }

    for (int ii=spikes.length()-1; ii>=0; --ii){
        if (showAllSpike){
            if ((int)predictnum[ii] >= 0){
                if (!showtutor[(int)predictnum[ii]]){
                    continue;
                }
            }
        } else {
            if ((int)tutornum[ii] >= 0){
                if (!showtutor[(int)tutornum[ii]]){
                    continue;
                }
            } else {
                if (!shownoise){
                    continue;
                }
            }
        }
        boolean flaghide = 0;
        for (int jj=0; jj<indspikehide.length(); ++jj){
            if (indspikehide[jj] == ii){
                flaghide = 1;
                break;
            }
        }
        if(flaghide){
            continue;
        }
        for (int jj=stspike; jj<enspike; ++jj){
            // *** check if the rectangle spanning each line overlaps
            if (spikes[ii][jj] <= spikes[ii][jj+1]){
                if (maxy < spikes[ii][jj] || spikes[ii][jj+1] < miny){
                    continue;
                }
            } else {
                if (maxy < spikes[ii][jj+1] || spikes[ii][jj] < miny){
                    continue;
                }
            }
            // *** calculation of intersection
            double Ax = posP.x();
            double Ay = posP.y();
            double Bx = posR.x();
            double By = posR.y();
            double Cx = tspike[jj];
            double Cy = spikes[ii][jj];
            double Dx = tspike[jj+1];
            double Dy = spikes[ii][jj+1];
            Bx -= Ax;
            By -= Ay;
            Cx -= Ax;
            Cy -= Ay;
            Dx -= Ax;
            Dy -= Ay;
            double distAB = sqrt(Bx*Bx+By*By);
            //  (2) Rotate the system so that point B is on the positive X axis.
            double theCos = Bx/distAB;
            double theSin = By/distAB;
            double newX = Cx * theCos + Cy * theSin;
            Cy = Cy * theCos - Cx * theSin;
            Cx = newX;
            newX = Dx * theCos + Dy * theSin;
            Dy = Dy * theCos - Dx * theSin;
            Dx = newX;
            if (Cy < 0 && Dy < 0 || Cy >= 0 && Dy >= 0){
                continue;
            }
            double ABpos = Dx + (Cx - Dx) * Dy / (Dy - Cy);
            if (ABpos < 0 || ABpos > distAB){
                continue;
            } else {
                hitiis.append(ii);
            }
        }
    }
    parentwidget->setHitiis(hitiis,indCh);

}

void QWidgetSpikes::pushTutorBox(int numTutor){
    if (numTutor == -1){
        shownoise = tutorRadionoise->isChecked();
    } else {
        showtutor[numTutor] = tutorRadio[numTutor]->isChecked();
    }
    parentwidget->setTutorBox(numTutor);
    changeSpikes();
}

QVector<bool> QWidgetSpikes::getShowTutor(){
    return showtutor;
}

bool QWidgetSpikes::getShowNoise(){
    return tutorRadionoise->isChecked();
}

/*
void QWidgetSpikes::setHitii(int query_hitii){
    hitii = query_hitii;
    changeSpikes();
}
*/
void QWidgetSpikes::setHitiis(QVector<int> query_hitii){
    hitiis = query_hitii;
    changeSpikes();
}

