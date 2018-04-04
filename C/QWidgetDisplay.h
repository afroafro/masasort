#ifndef QWIDGETDISPLAY_H
#define QWIDGETDISPLAY_H

#include <QApplication>
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QIODevice>
#include <QMessageBox>
#include <QtCharts>
#include <WidgetIntanChart.h>
//consider not to include this. Finish with forward declaration

using namespace std;
using namespace QtCharts;

class WidgetIntanChart;
class QChartViewSpikes;
class QWidgetDisplay : public QWidget{
    //class QChartViewSpikes : public QChartView{
    Q_OBJECT
public:
    //QChartViewSpikes (QChart* chart, QWidget* parent=0);
    QWidgetDisplay (QWidget* parent=0);
    void setData(QVector<double>, QVector<double>);
    void drawData();
    void changeData();
    int getCurrentSpike();
    void calcHitSpike(QPointF);
    QPen linepen;
    QPen linepencurrent;
    int lenseries;
    void setHitii(int);
    QPen getLinepen();
    QPen getLinepencurrent();

protected:
    // void mouseMoveEvent(QMouseEvent *event);
    // void mouseReleaseEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent *event);
    // void mousePressEvent(QMouseEvent *event);
private:
    QGridLayout* mainLayout2;
    QChart* spikechart;
    QChartViewSpikes* chartview2;
    QVector<QVector<double>> ydata;
    QVector<QVector<double>> tdata;
    int hitii;
    int indCh;
    QList<QLineSeries*> spikeseries;
    //QLineSeries *spikeseries[2000];
    QValueAxis *axisX;
    QValueAxis *axisY;
    QSignalMapper* tutorboxMapper[10];
    bool showAllSpike;
    void saveSpikes();
    QVector<bool> showtutor;
    bool shownoise;
    WidgetIntanChart *parentwidget;
    bool calchitspikebusy;
    qreal axisYmax;
    qreal axisYmin;
    void setAxesY();
signals:
private slots:
    void pushFile(QAction *action);
};

#endif // QCHARTVIEWSPIKES_H
