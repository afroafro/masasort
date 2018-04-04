#ifndef QCHARTVIEWSPIKES_H
#define QCHARTVIEWSPIKES_H

#include <QApplication>
#include <QtCharts>

//consider not to include this. Finish with forward declaration
#include "QWidgetSpikes.h"
#include "QWidgetFeature.h"

using namespace std;
using namespace QtCharts;

class QWidgetSpikes;
class QChartViewSpikes : public QChartView{
    Q_OBJECT
public:
    QChartViewSpikes (QChart* chart, QWidget* parent=0);
    void setID(int);
    int getID();
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    bool viewportEvent(QEvent *event);
private:
    bool flagpressed;
    QWidgetSpikes* parentwidgetspikes;
    QWidgetFeature* parentwidgetfeature;
    int ID;
    bool m_isTouching;
    bool mouserelease_busy;
    QPointF pospressed;
};

#endif // QCHARTVIEWSPIKES_H
