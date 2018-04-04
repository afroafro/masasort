#include "QWidgetDisplay.h"
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
QWidgetDisplay::QWidgetDisplay(QWidget *parent) : QWidget(parent, Qt::Window){
    mainLayout2 = new QGridLayout;
    QMenuBar *menuBar = new QMenuBar;
    QMenu* fileMenu = new QMenu("&File", this);
    menuBar->addMenu(fileMenu);
    QAction* openAction = new QAction("Save data");
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

    parentwidget = (WidgetIntanChart*)parentWidget();

    int indVlayout = 0;
    showAllSpike = false;

    linepen.setWidth(0.5);
    linepen.setColor(QColor(160,160,160,40));
    linepencurrent.setWidth(0.5);
    linepencurrent.setColor(QColor(0,0,0,255));

    //QValueAxis *axesX2 = new QValueAxis;
    //chart2->addAxis(axesX2, Qt::AlignBottom);
    //QValueAxis *axesY2 = new QValueAxis;
    //chart2->addAxis(axesY2, Qt::AlignLeft);
    chartview2 = new QChartViewSpikes(spikechart, this);
    chartview2->setRenderHint(QPainter::Antialiasing);
    mainLayout2->addWidget(chartview2, indVlayout, 0);
    indVlayout += 1;

    shownoise = true;

    // buttons
    resize(200,400);
    setLayout(mainLayout2);

    lenseries = 0;
    axisX = new QValueAxis;
    axisY = new QValueAxis;
    spikechart->addAxis(axisX, Qt::AlignBottom);
    spikechart->addAxis(axisY, Qt::AlignLeft);
}

void QWidgetDisplay::setData(QVector<double> query_ydata, QVector<double> query_tdata){
    ydata.append(query_ydata);
    tdata.append(query_tdata);
    hitii = -1;
    int lendata = ydata.length();

    if (lenseries < lendata){
        for (int ii=lenseries; ii<lendata; ++ii){
            spikeseries.append(new QLineSeries());
            spikechart->addSeries(spikeseries[ii]);
            spikeseries[ii]->attachAxis(axisX);
            spikeseries[ii]->attachAxis(axisY);
        }
        //QMessageBox::information(this, "!!", "only shows 2000 spikes (see C++ code)");
    } else if (lenseries > lendata){
        for (int ii=lenseries-1; ii>=lendata; --ii){
            spikechart->removeSeries(spikeseries[ii]);
            delete(spikeseries.takeAt(ii));
        }
    }
    //QMessageBox::information(this, "!!size", QString::number(spikeseries.size()));
    //QMessageBox::information(this, "!!lenspike", QString::number(lenspikes));
    lenseries = lendata;

    drawData();
}


void QWidgetDisplay::changeData(){
    int lenall = spikeseries.length();
    int lendata = ydata.length();
    for (int ii=0; ii<lenall; ++ii){
        spikeseries[ii]->setVisible(false);
    }
    for (int ii=0; ii<lendata; ++ii){
        if (ii == hitii){
            spikeseries[ii]->setPen(linepencurrent);
            spikeseries[ii]->setVisible(true);
        } else {
            spikeseries[ii]->setPen(linepen);
            spikeseries[ii]->setVisible(true);
        }
    }
}

void QWidgetDisplay::drawData(){

    int maxy = ydata[0][0];
    int miny = ydata[0][0];
    int maxx = tdata[0][0];
    int minx = tdata[0][0];
    int lendata = ydata.length();

    for (int ii=0; ii<lenseries; ++ii){
        spikeseries[ii]->clear();
    }

    for (int ii=0; ii<lendata; ++ii){
        for (int jj=0; jj<ydata[ii].length(); ++jj){
            spikeseries[ii]->append(tdata[ii][jj], ydata[ii][jj]);
            if (miny > ydata[ii][jj]){
                miny = ydata[ii][jj];
            }
            if (maxy < ydata[ii][jj]){
                maxy = ydata[ii][jj];
            }
            if (minx > tdata[ii][jj]){
                minx = tdata[ii][jj];
            }
            if (maxx < tdata[ii][jj]){
                maxx = tdata[ii][jj];
            }
        }
    }

    double axisymax;
    double axisymin;
    double axisxmax = maxx;
    double axisxmin = minx;
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
    axisX->setGridLineColor(QColor(230,230,230,100));
    //QMessageBox::information(this, "hit!!", QString::number((int)((axisxmax-axisxmin)/ticknum)+1));

    changeData();

}


int QWidgetDisplay::getCurrentSpike(){
    return hitii;
}

void QWidgetDisplay::keyPressEvent(QKeyEvent *event){

    if(-1 < hitii && hitii < ydata.length() ){
        switch (event->key()){
        case Qt::Key_Left:
            if (hitii == 0){
                hitii = ydata.length()-1;
            } else {
                hitii -= 1;
            }
            changeData();
            break;
        case Qt::Key_Right:
            if (hitii == ydata.length()-1){
                hitii = 0;
            } else {
                hitii += 1;
            }
            changeData();
            break;
        case Qt::Key_Up:
            if (event->modifiers() == Qt::ShiftModifier){
                for (int ii=0; ii<ydata[hitii].length(); ++ii){
                    ydata[hitii][ii] += 1;
                }
            } else {
                if (hitii == 0){
                    hitii = ydata.length()-1;
                } else {
                    hitii -= 1;
                }
            }
            changeData();
            break;
        case Qt::Key_Down:
            if (event->modifiers() == Qt::ShiftModifier){
                for (int ii=0; ii<ydata[hitii].length(); ++ii){
                    ydata[hitii][ii] -= 1;
                }
            } else {
                if (hitii == ydata.length()-1){
                    hitii = 0;
                } else {
                    hitii += 1;
                }
            }
            changeData();
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
}

void QWidgetDisplay::setAxesY(){
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
    drawData();
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

void QWidgetDisplay::pushFile(QAction *action){
    int value = action->data().toInt();
    if (value == 0){//
        saveSpikes();
    }
}

void QWidgetDisplay::saveSpikes(){

    QString fileName = QFileDialog::getSaveFileName(this, "Save as", "data.txt", "ASCII (*.txt);;All Files (*)");
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
        for (int ii=0; ii<ydata.length(); ++ii){
//            out << QString::number(tutornum[ii]) << QString::number(predictnum[ii]) << "\r\n";
        }
        file.close();

    }

}


void QWidgetDisplay::calcHitSpike(QPointF pos){
    double errorval = 10;
    double val;
    int tmphit = -1;
    int lendata = tdata.length();
    int stdata = -1;
    for (int ii=0; ii<lendata; ++ii){
        for (int jj=0; jj<tdata[ii].length()-1; ++jj){
            if(tdata[ii][jj] <= pos.x() && pos.x() <= tdata[ii][jj+1]){
                stdata = jj;
                break;
            }
        }
        if (stdata != -1){
            for (int ii=ydata.length()-1; ii>=0; --ii){
                val = (pos.y() - ydata[ii][stdata])*(tdata[ii][stdata+1] - pos.x()) - (ydata[ii][stdata+1] - pos.y())*(pos.x() - tdata[ii][stdata]);
                if ( val < errorval && val > -errorval){
                    if (((pos.y() >= ydata[ii][stdata]) && (pos.y() <= ydata[ii][stdata+1]) && (pos.x() >= tdata[ii][stdata]) && (pos.x() <= tdata[ii][stdata+1])) || ((pos.y() <= ydata[ii][stdata]) && (pos.y() >= ydata[ii][stdata+1]) && (pos.x() >= tdata[ii][stdata]) && (pos.x() <= tdata[ii][stdata+1]))){
                        tmphit = ii;
                        break;
                    }
                }
                if (tmphit != -1){
                    break;
                }
            }
        }
    }

    //QMessageBox::information(this, "hit!!", "ok");
    if (tmphit != -1){
        hitii = tmphit;
    } else {
        hitii = -1;
    }
    //setFocus();
}

QPen QWidgetDisplay::getLinepen(){
    return linepen;
}
QPen QWidgetDisplay::getLinepencurrent(){
    return linepencurrent;
}
void QWidgetDisplay::setHitii(int query_hitii){
    hitii = query_hitii;
    changeData();
}
