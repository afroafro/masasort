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
    void setShowAllSpike(bool);\
    void calcHitSpike(QPointF);
    void calcHitSpikeDragged(QPointF, QPointF);
    void drawCursorLine(QPointF, QPointF);
    QVector<bool> getShowTutor();
    bool getShowNoise();
    bool getShowAllSpike();
    //void setHitii(int);
    void setHitiis(QVector<int>);
    QRadioButton *tutorRadionoise;
    void setTutorRadio(int, bool);
    void setNoiseRadio(bool);
    void inputTutornum(int);
    void updateTutorRadioTutornum();
    void updateTutorRadioPredictnum();

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
    QVector<int> indspikehide;
    QVector<int> indspikehidetmp;
    int hitii;
    QVector<int> hitiis;
    int indCh;
    QVector<QLineSeries*> spikeseries; // *** this used to be QList. I changed it to QVector 9/5/2017. Everything looks fine, but may bring up some errors later...
    QRadioButton *tutorRadio[10];
    QPen linepenspike;
    QPen linepencurrent;
    QVector<QPen> linepentutor;
    QPen linepennoise;
    QPen linepencursor;
    int lenseries;
    int lentutor;
    //QLineSeries *spikeseries[2000];
    QValueAxis *axisX;
    QValueAxis *axisY;
    QSignalMapper* tutorboxMapper[10];
    bool showAllSpike;
    void saveSpikes();
    QVector<bool> showtutor;
    bool shownoise;
    WidgetIntanChart *parentwidget;
    QRadioButton *radiobutton0;
    QRadioButton *radiobutton1;
    QRadioButton *radiobutton2;
    bool calchitspikebusy;
    qreal axisYmax;
    qreal axisYmin;
    void setAxesY();
    QLineSeries* cursorline;
signals:
private slots:
    void pushFile(QAction *action);
    void pushtutorSpike();
    void pushallSpike();
    void pushAddedTutor();
    void pushTutorBox(int);
};

#endif // QCHARTVIEWSPIKES_H
