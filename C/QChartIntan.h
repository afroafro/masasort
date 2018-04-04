#ifndef QCHARTINTAN_H
#define QCHARTINTAN_H

#include <QtCharts/QChart>
class QGestureEvent;
QT_CHARTS_USE_NAMESPACE

class QChartIntan : public QChart{
public:
    explicit QChartIntan(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    ~QChartIntan();

protected:
    //bool sceneEvent(QEvent *event);

private:
    //bool gestureEvent(QGestureEvent *event);

private:

};

#endif // QCHARTINTAN_H
