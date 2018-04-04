#include "QChartIntan.h"
#include <QtWidgets/QGesture>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>


QChartIntan::QChartIntan(QGraphicsItem *parent, Qt::WindowFlags wFlags) : QChart(QChart::ChartTypeCartesian, parent, wFlags){

    // Seems that QGraphicsView (QChartView) does not grab gestures.
    // They can only be grabbed here in the QGraphicsWidget (QChart).

    //grabGesture(Qt::PanGesture);
    //grabGesture(Qt::PinchGesture);

}

QChartIntan::~QChartIntan(){
}

/*
bool QChartIntan::sceneEvent(QEvent *event){
    if (event->type() == QEvent::Gesture)
        return gestureEvent(static_cast<QGestureEvent *>(event));
    return QChart::event(event);
}

bool QChartIntan::gestureEvent(QGestureEvent *event){
    if (QGesture *gesture = event->gesture(Qt::PanGesture)) {
        QPanGesture *pan = static_cast<QPanGesture *>(gesture);
        QChart::scroll(-(pan->delta().x()), pan->delta().y());
    }

    if (QGesture *gesture = event->gesture(Qt::PinchGesture)) {
        QPinchGesture *pinch = static_cast<QPinchGesture *>(gesture);
        if (pinch->changeFlags() & QPinchGesture::ScaleFactorChanged)
            QChart::zoom(pinch->scaleFactor());
    }

    return true;
}
*/

/*
void QChartIntan::keyPressEvent(QKeyEvent *event){
    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
    case Qt::Key_Left:
        chart()->scroll(-10, 0);
        break;
    case Qt::Key_Right:
        chart()->scroll(10, 0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0, 10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -10);
        break;
    default:
        QChart::keyPressEvent(event);
    //    break;
    //}

}

void QChartIntan::mousePressEvent(QMouseEvent *event){\
    //QChart::mapFromParent(&parent:)
    QChart::mousePressEvent(event);
}

void QChartIntan::mouseMoveEvent(QMouseEvent *event){
    QChart::mouseMoveEvent(event);

}

void QChartIntan::mouseReleaseEvent(QMouseEvent *event){
    QChart::mouseReleaseEvent(event);
}
*/
