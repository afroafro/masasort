#ifndef QWIDGETSPIKES_H
#define QWIDGETSPIKES_H

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
class QWidgetSpikes : public QWidget{
    //class QChartViewSpikes : public QChartView{
    Q_OBJECT
public:
    //QChartViewSpikes (QChart* chart, QWidget* parent=0);
    QWidgetSpikes (QWidget* parent=0);
    void setSpikes(QVector<QVector<double>>, QVector<double>, int, QVector<double>, QVector<double>);
    void drawSpikes();
    void changeSpikes();
    QVector<double> getTutornum();
    void setTutornum(QVector<double>);
    int getCurrentSpike();
    int getSpikeCh();
    void setPredictnum(QVector<double>);
    void setShowAllSpike(boolean);\
    void calcHitSpike(QPointF);
    QVector<boolean> getShowTutor();
    boolean getShowNoise();
    boolean getShowAllSpike();
    QRadioButton *tutorRadio[10];
    QPen linepen;
    QPen linepencurrent;
    QPen linepentutor[10];
    QPen linepennoise;
    int lenseries;
    void setHitii(int);
    QPen getLinepen();
    QPen getLinepencurrent();
    QPen getLinepentutor(int);
    QPen getLinepennoise();
    QRadioButton *tutorRadionoise;

protected:
    // void mouseMoveEvent(QMouseEvent *event);
    // void mouseReleaseEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent *event);
    // void mousePressEvent(QMouseEvent *event);
private:
    QGridLayout* mainLayout2;
    QChart* spikechart;
    QChartViewSpikes* chartview2;
    QVector<QVector<double>> spikes;
    QVector<double> tspike;
    QVector<double> tutornum;
    QVector<double> predictnum;
    int hitii;
    int indCh;
    QList<QLineSeries*> spikeseries;
    //QLineSeries *spikeseries[2000];
    QValueAxis *axisX;
    QValueAxis *axisY;
    QSignalMapper* tutorboxMapper[10];
    boolean showAllSpike;
    void saveSpikes();
    QVector<boolean> showtutor;
    boolean shownoise;
    WidgetIntanChart *parentwidget;
    QRadioButton *radiobutton0;
    QRadioButton *radiobutton1;
    QRadioButton *radiobutton2;
    boolean calchitspikebusy;
    qreal axisYmax;
    qreal axisYmin;
    void setAxesY();
signals:
private slots:
    void pushFile(QAction *action);
    void pushtutorSpike();
    void pushallSpike();
    void pushAddedTutor();
    void pushTutorBox(int);
};

#endif // QCHARTVIEWSPIKES_H
