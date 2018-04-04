#ifndef QCHARTVIEWINTAN_H
#define QCHARTVIEWINTAN_H

#include <QApplication>
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QIODevice>
#include <QMessageBox>
#include <QtCharts>

using namespace std;
using namespace QtCharts;

class WidgetIntanChart;
class QChartViewIntan : public QChartView{
public:
    QChartViewIntan (QChart* chart, QWidget* parent=0);
    void setID(int);
    int getID();
protected:
    bool viewportEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent *event);
private:
    bool m_isTouching;
    bool mouserelease_busy;
    int ID;
    WidgetIntanChart* parentwidget;
    QPointF pospressed;
    //bool flagmousepressed;
};

#endif // QCHARTVIEWINTAN_H
