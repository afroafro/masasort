#include "QChartViewIntan.h"
#include <QApplication>
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QIODevice>
#include <QMessageBox>
#include <QtCharts>
#include "WidgetIntanChart.h"

using namespace std;
using namespace QtCharts;

QChartViewIntan::QChartViewIntan(QChart *chart, QWidget *parent) : QChartView(chart, parent), m_isTouching(false){
    setRubberBand(QChartView::HorizontalRubberBand);
    ID = -1;
    mouserelease_busy = false;
}

void QChartViewIntan::setID(int tmpID){
    ID = tmpID;
}

int QChartViewIntan::getID(){
    return ID;
}

void QChartViewIntan::keyPressEvent(QKeyEvent *event){
    //parentwidget = (WidgetIntanChart*)parentWidget();
    parentwidget->keyPressReceiver(event);
}

bool QChartViewIntan::viewportEvent(QEvent *event){
    if (event->type() == QEvent::TouchBegin) {
        // By default touch events are converted to mouse events. So
        // after this event we will get a mouse event also but we want
        // to handle touch events as gestures only. So we need this safeguard
        // to block mouse events that are actually generated from touch.
        m_isTouching = true;

        // Turn off animations when handling gestures they
        // will only slow us down.
        chart()->setAnimationOptions(QChart::NoAnimation);
    }
    return QChartView::viewportEvent(event);
}

void QChartViewIntan::mousePressEvent(QMouseEvent *event){
    if (m_isTouching){
        return;
    }
    pospressed = QWidget::mapFromGlobal(QCursor::pos());
    QChartView::mousePressEvent(event);
}

void QChartViewIntan::mouseMoveEvent(QMouseEvent *event){
    if (m_isTouching){
        return;
    }
    QChartView::mouseMoveEvent(event);
}

void QChartViewIntan::mouseReleaseEvent(QMouseEvent *event){
    if (m_isTouching){
        m_isTouching = false;
    }
    QChartView::mouseReleaseEvent(event);

    if (mouserelease_busy){
        return;
    } else {
        mouserelease_busy = true; // This is to avoid computations when users click rapidly. I am not sure this works...
        // !!! this produces some problems
        parentwidget = (WidgetIntanChart*)parentWidget();

        // Because we disabled animations when touch event was detected
        // we must put them back on.
        //chart()->setAnimationOptions(QChart::SeriesAnimations);

        QPointF pos = QWidget::mapFromGlobal(QCursor::pos());
        if (pos.x() == pospressed.x() && pos.y() == pospressed.y()){
            //QMessageBox::information(this, "hit!!", QString::number(pos.x()) + "," + QString::number(pos.y()));
            pos = chart()->mapFromParent(pos);
            //QMessageBox::information(this, "hit!!", QString::number(pos.x()) + "," + QString::number(pos.y()));
            pos = chart()->mapToValue(pos);
            //QMessageBox::information(this, "hit!!", QString::number(pos.x()) + "," + QString::number(pos.y()));

            parentwidget->calcHitSpike(pos,ID);

        }
        //flagmousepressed = false;
        parentwidget->alignAxesXto(ID);
        mouserelease_busy = false;
    }
}
