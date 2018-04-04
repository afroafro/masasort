#include "QWidgetFeature.h"
#include "QWidgetSpikes.h"
#include "QChartViewSpikes.h"

QWidgetFeature::QWidgetFeature(QWidget *parent, int nadditionalfeature) : QWidget(parent, Qt::Window){
    parentwidget = (WidgetIntanChart*)parentWidget();
    nChart = nadditionalfeature;
    stChart = 0;
    nChannel = parentwidget->getnChannel();
    hitii = -1;

    axisYmax = 0;
    axisYmin = 0;

    lenseries = 0;


    //lenseries = parentwidget->lenseries;

    linepenspike = parentwidget->getLinepenspike();
    linepencurrent =  parentwidget->getLinepencurrent();
    linepentutor = parentwidget->getLinepentutor();
    shownoise = true;

    linepencursor = parentwidget->getLinepencursor();

    showAllSpike = false;
    chartLayout = new QGridLayout;
    chartLayout->setRowMinimumHeight(50,20);
    chartLayout->setContentsMargins(0,0,0,0);
    chartLayout->setSpacing(0);

    //series->setUseOpenGL(true);

    showtutor.resize(nChart);
    for (int ii=0; ii<nChart; ++ii){
        chart[ii] = new QChart();
        chart[ii]->legend()->hide();
        chart[ii]->setMargins(QMargins(0,0,0,0));
        chart[ii]->setMinimumHeight(20.0);
        //chart[ii]->setAnimationOptions(QChart::SeriesAnimations);

        axesX[ii] = new QValueAxis;
        axesY[ii] = new QValueAxis;
        chart[ii]->addAxis(axesX[ii], Qt::AlignBottom);
        chart[ii]->addAxis(axesY[ii], Qt::AlignLeft);
        for (int jj=0; jj<lenseries; ++jj){
            spikeseries[ii].append(new QLineSeries());
            chart[ii]->addSeries(spikeseries[ii][jj]);
            spikeseries[ii][jj]->attachAxis(axesX[ii]);
            spikeseries[ii][jj]->attachAxis(axesY[ii]);
        }

        cursorline.append(new QLineSeries());
        cursorline[ii]->setPen(linepencursor);
        chart[ii]->addSeries(cursorline[ii]);
        cursorline[ii]->attachAxis(axesX[ii]);
        cursorline[ii]->attachAxis(axesY[ii]);

        chartview[ii] = new QChartViewSpikes(chart[ii]);
        chartview[ii]->setRenderHint(QPainter::Antialiasing);
        chartview[ii]->setID(ii);

        showtutor[ii] = true;

    }
    for (int ii=0; ii<nChart; ++ii){
        //QMessageBox::information(this, "!!", QString::number(nChart));
        chartLayout->addWidget(chartview[ii+stChart], ii, 1);
    }
    this->setLayout(chartLayout);
    resize(200,200);
    //QMessageBox::information(this, "info", "endconstructor");

}


void QWidgetFeature::setSpikes(QVector<int> query_indsspike, QVector<QVector<QVector<double>>> query_sspike, QVector<double> query_tutornum, QVector<double> query_predictnum){
    indsspike = query_indsspike;
    sspike = query_sspike;
    tutornum = query_tutornum;
    predictnum = query_predictnum;
    int lenspikes = tutornum.length();

    //QMessageBox::information(this, "!!", QString::number(lenseries));
    //QMessageBox::information(this, "!!", QString::number(lenspikes));
    if (lenseries < lenspikes){
        for (int ii=0; ii<nChart; ++ii){
            for (int jj=lenseries; jj<lenspikes; ++jj){
                spikeseries[ii].append(new QLineSeries());
                chart[ii]->addSeries(spikeseries[ii][jj]);
                spikeseries[ii][jj]->attachAxis(axesX[ii]);
                spikeseries[ii][jj]->attachAxis(axesY[ii]);
            }
        }
        //QMessageBox::information(this, "!!", "only shows 2000 spikes (see C++ code)");
    } else if (lenseries > lenspikes){
        for (int ii=0; ii<nChart; ++ii){
            for (int jj=lenseries-1; jj>=lenspikes; --jj){
                chart[ii]->removeSeries(spikeseries[ii][jj]);
                delete(spikeseries[ii].takeAt(jj));
            }
        }
    }
    //QMessageBox::information(this, "!!size", QString::number(spikeseries.size()));
    //QMessageBox::information(this, "!!lenspike", QString::number(lenspikes));
    lenseries = lenspikes;

    drawSpikes();
}

void QWidgetFeature::changeSpikes(){
    for (int ii=0; ii<sspike.length(); ++ii){
        for (int jj=0; jj<sspike[ii].length(); ++jj){
            spikeseries[ii][jj]->setVisible(false);
            boolean flaghitii = false;
            for (int kk=0; kk<hitiis.length(); ++kk){
                if (jj == hitiis[kk]){
                    spikeseries[ii][jj]->setPen(linepencurrent);
                    spikeseries[ii][jj]->setVisible(true);
                    flaghitii = true;
                    break;
                }
            }
            if (!flaghitii){
                if (showAllSpike){
                    if ((int)predictnum[jj] >= 0){
                        spikeseries[ii][jj]->setPen(linepentutor[(int)predictnum[jj]]);
                        if (showtutor[(int)predictnum[jj]]){
                            spikeseries[ii][jj]->setVisible(true);
                        }
                    } else {
                        spikeseries[ii][jj]->setPen(linepenspike);
                        if (shownoise){
                            spikeseries[ii][jj]->setVisible(true);
                        }
                    }
                } else {
                    if ((int)tutornum[jj] >= 0){
                        spikeseries[ii][jj]->setPen(linepentutor[(int)tutornum[jj]]);
                        if (showtutor[(int)tutornum[jj]]){
                            spikeseries[ii][jj]->setVisible(true);
                        }
                    } else {
                        spikeseries[ii][jj]->setPen(linepenspike);
                        if (shownoise){
                            spikeseries[ii][jj]->setVisible(true);
                        }
                    }
                }
            }
        }
    }

//    spikechart->createDefaultAxes();
}
void QWidgetFeature::drawSpikes(){

    for (int ii=0; ii<nChart; ++ii){
        for (int jj=0; jj<lenseries; ++jj){
            spikeseries[ii][jj]->clear();
        }
    }

    int maxy;
    int maxx;
    int miny;
    int minx;
    bool showthis;
    for (int ii=0; ii<sspike.length(); ++ii){
        maxy = sspike[ii][0][0];
        miny = sspike[ii][0][0];
        maxx = 0;
        minx = 0;
        for (int jj=0; jj<sspike[ii].length(); ++jj){
            showthis = false;
            boolean flaghitii = false;
            for (int kk=0; kk<hitiis.length(); ++kk){
                if (jj == hitiis[kk]){
                    spikeseries[ii][jj]->setPen(linepencurrent);
                    showthis = true;
                    flaghitii = true;
                    break;
                }
            }
            if (!flaghitii){
                if (showAllSpike){
                    if ((int)predictnum[jj] >= 0){
                        spikeseries[ii][jj]->setPen(linepentutor[(int)predictnum[jj]]);
                        if (showtutor[(int)predictnum[jj]]){
                            showthis = true;
                        }
                    } else {
                        spikeseries[ii][jj]->setPen(linepenspike);
                        if (shownoise){
                            showthis = true;
                        }
                    }
                } else {
                    if ((int)tutornum[jj] >= 0){
                        spikeseries[ii][jj]->setPen(linepentutor[(int)tutornum[jj]]);
                        if (showtutor[(int)tutornum[jj]]){
                            showthis = true;
                        }
                    } else {
                        spikeseries[ii][jj]->setPen(linepenspike);
                        if (shownoise){
                            showthis = true;
                        }
                    }
                }
            }

            for (int kk=0; kk<sspike[ii][jj].length(); ++kk){
                if (showthis){
                    //QMessageBox::information(this, "show", QString::number(sspike[ii][jj][kk]));
                    spikeseries[ii][jj]->append(kk, sspike[ii][jj][kk]);
                }
                if (miny > sspike[ii][jj][kk]){
                    miny = sspike[ii][jj][kk];
                }
                if (maxy < sspike[ii][jj][kk]){
                    maxy = sspike[ii][jj][kk];
                }
                if (minx > kk){
                    minx = kk;
                }
                if (maxx < kk){
                    maxx = kk;
                }
            }
        }

        if (axisYmin == 0 && axisYmax == 0){
            axesY[ii]->setRange((miny - miny%100 - 100), (maxy - maxy%100 + 100));
        } else {
            axesY[ii]->setRange(axisYmin, axisYmax);
        }
        axesX[ii]->setRange((minx - 1), (maxx + 1));
    }

//    spikechart->createDefaultAxes();
}

void QWidgetFeature::setPredictnum(QVector<double> query_predictnum){
    predictnum = query_predictnum;
}
void QWidgetFeature::setTutornum(QVector<double> query_tutornum){
    tutornum = query_tutornum;
}
/*
void QWidgetFeature::setHitii(int query_hitii){
    hitii = query_hitii;
    changeSpikes();
}
*/
void QWidgetFeature::setHitiis(QVector<int> query_hitii){
    hitiis = query_hitii;
    changeSpikes();
}

void QWidgetFeature::calcHitSpike(QPointF pos, int ID){
    for (int ii=0; ii<nChart; ++ii){
        cursorline[ii]->clear();
    }
    double errorval = 5;
    double val;
    hitiis.clear();
    bool showthis;
    for (int jj=sspike[ID].length()-1; jj>=0; --jj){
        if (showAllSpike){
            if ((int)predictnum[jj] >= 0){
                if (!showtutor[(int)predictnum[jj]]){
                    continue;
                }
            }
        } else {
            if ((int)tutornum[jj] >= 0){
                if (!showtutor[(int)tutornum[jj]]){
                    continue;
                }
            } else {
                if (!shownoise){
                    continue;
                }
            }
        }
        for (int kk=0; kk<sspike[ID][jj].length()-1; ++kk){
            val = (pos.y() - sspike[ID][jj][kk])*(kk+1 - pos.x()) - (sspike[ID][jj][kk+1] - pos.y())*(pos.x() - kk);
            if ( val < errorval && val > -errorval){
                if (((pos.y() >= sspike[ID][jj][kk]) && (pos.y() <= sspike[ID][jj][kk+1]) && (pos.x() >= kk) && (pos.x() <= kk+1)) || ((pos.y() <= sspike[ID][jj][kk]) && (pos.y() >= sspike[ID][jj][kk+1]) && (pos.x() >= kk) && (pos.x() <= kk+1))){
                    hitiis.append(jj);
                    break;
                }
                // && (spikes[ii][jj] < posmin.y()) && (jj < posmax.x()) && (jj > posmin.x())){
            }
        }
        if (hitiis.length() > 0){
            break;
        }
    }
    parentwidget->setHitiis(hitiis);
    //changeSpikes();
    //setFocus();
}

void QWidgetFeature::calcHitSpikeDragged(QPointF posP, QPointF posR, int ID){
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

    int lenspike = sspike[ID][0].length();
    int stspike = 0;
    int enspike = lenspike-1;
    for (int ii=0; ii<lenspike-1; ++ii){ // ii is tspike
        if(ii <= minx && minx <= ii+1){
            stspike = ii;
        }
        if(ii <= maxx && maxx <= ii+1){
            enspike = ii+1;
            break;
        }
    }

    for (int ii=sspike[ID].length()-1; ii>=0; --ii){
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
        for (int jj=stspike; jj<enspike; ++jj){
            // *** check if the rectangle spanning each line overlaps
            if (sspike[ID][ii][jj] <= sspike[ID][ii][jj+1]){
                if (maxy < sspike[ID][ii][jj] || sspike[ID][ii][jj+1] < miny){
                    continue;
                }
            } else {
                if (maxy < sspike[ID][ii][jj+1] || sspike[ID][ii][jj] < miny){
                    continue;
                }
            }
            // *** calculation of intersection
            double Ax = posP.x();
            double Ay = posP.y();
            double Bx = posR.x();
            double By = posR.y();
            double Cx = jj;
            double Cy = sspike[ID][ii][jj];
            double Dx = jj+1;
            double Dy = sspike[ID][ii][jj+1];
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
    parentwidget->setHitiis(hitiis);
}

void QWidgetFeature::drawCursorLine(QPointF posPress, QPointF posMove, int ID){
    for (int ii=0; ii<nChart; ++ii){
        cursorline[ii]->clear();
    }
    cursorline[ID]->append(posPress);
    cursorline[ID]->append(posMove);
}

void QWidgetFeature::keyPressEvent(QKeyEvent *event){
    switch (event->key()) {
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

void QWidgetFeature::setAxesY(){
    QVector<QString> dialoglabel;
    QVector<QString> dialogvalue;
    QVector<QString> returnvalue;
    dialoglabel.append("Y max");
    dialoglabel.append("Y min (reset if max&min == 0)");
    dialogvalue.append(QString::number(axisYmax));
    dialogvalue.append(QString::number(axisYmin));
    returnvalue = myDialog("Y range:", dialoglabel, dialogvalue);
    if (returnvalue.length() == 0){
        return;
    }
    axisYmax = returnvalue[0].toDouble();
    axisYmin = returnvalue[1].toDouble();
}


void QWidgetFeature::setAllSpike(bool showall){
    showAllSpike = showall;
    changeSpikes();
}

void QWidgetFeature::setTutorBox(int numTutor, bool tmpshownoise, QVector<bool> tmpshowtutor){
    if (numTutor == -1){
        shownoise = tmpshownoise;
    } else {
        showtutor = tmpshowtutor;
    }
    changeSpikes();
}



QVector<QString> QWidgetFeature::myDialog(QString dialogtitle, QVector<QString> dialoglabel, QVector<QString> dialogvalue){
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
