#include <QApplication>
#include <QtCharts>
#include "QChartViewSpikes.h"
#include "QWidgetSpikes.h"

using namespace std;
using namespace QtCharts;

QChartViewSpikes::QChartViewSpikes(QChart *chart, QWidget *parent) : QChartView(chart, parent){
    // *** This QChartView is shared by spike window and feature window
    // *** setRubberBand(QChartView::VerticalRubberBand); // this does not work somehow, even though I put viewportEvent
    mouserelease_busy = false;
    flagpressed = false;
    ID = -1;
}
//QChartViewSpike::QChartViewSpike(QChart *chart, QWidgetSpikes *parent){
//    QChartViewSpike(chart, parent);
//}


bool QChartViewSpikes::viewportEvent(QEvent *event){
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

void QChartViewSpikes::mousePressEvent(QMouseEvent *event){
    if (m_isTouching){
        return;
    }
    pospressed = QWidget::mapFromGlobal(QCursor::pos());
    flagpressed = true;
    QWidget::mousePressEvent(event);
}

void QChartViewSpikes::setID(int query_ID){
    ID = query_ID;
}

int QChartViewSpikes::getID(){
    return ID;
}


void QChartViewSpikes::mouseMoveEvent(QMouseEvent *event){
    if (m_isTouching){
        return;
    }
    if (flagpressed){
        QPointF posPress = chart()->mapFromParent(pospressed);
        posPress = chart()->mapToValue(posPress);
        QPointF pos = QWidget::mapFromGlobal(QCursor::pos());
        pos = chart()->mapFromParent(pos);
        pos = chart()->mapToValue(pos);
        parentwidgetfeature = (QWidgetFeature*)parentWidget();
        parentwidgetspikes = (QWidgetSpikes*)parentWidget();
        if (ID >= 0){
            parentwidgetfeature->drawCursorLine(posPress, pos, ID);
        } else {
            parentwidgetspikes->drawCursorLine(posPress, pos);
        }
    }
    QChartView::mouseMoveEvent(event);
}

void QChartViewSpikes::mouseReleaseEvent(QMouseEvent *event){
    if (m_isTouching){
        m_isTouching = false;
    }
    QChartView::mouseReleaseEvent(event);

    if (mouserelease_busy){
        flagpressed = false;
        return;
    } else {
        mouserelease_busy = true; // This is to avoid computations when users click rapidly. I am not sure this works...
        QPointF pos = QWidget::mapFromGlobal(QCursor::pos());
        if (!pos.isNull() && !pospressed.isNull()){
            parentwidgetfeature = (QWidgetFeature*)parentWidget();
            parentwidgetspikes = (QWidgetSpikes*)parentWidget();
            if (pos.x() == pospressed.x() && pos.y() == pospressed.y()){
                pos = chart()->mapFromParent(pos);
                pos = chart()->mapToValue(pos);
                if (ID >= 0){
                    //QMessageBox::information(this, "hit!!", QString::number(ID) + " (" + QString::number(pos.x()) + "," + QString::number(pos.y()) + ")");
                    parentwidgetfeature->calcHitSpike(pos,ID);
                } else {
                    parentwidgetspikes->calcHitSpike(pos);
                }
            } else { // *** dragged
                QPointF posPress = chart()->mapFromParent(pospressed);
                posPress = chart()->mapToValue(posPress);
                pos = chart()->mapFromParent(pos);
                pos = chart()->mapToValue(pos);
                if (ID >= 0){
                    parentwidgetfeature->calcHitSpikeDragged(posPress, pos, ID);
                } else {
                    parentwidgetspikes->calcHitSpikeDragged(posPress, pos);
                }
            }
        }
        mouserelease_busy = false;
        flagpressed = false;
    }
}
