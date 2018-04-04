#ifndef QWIDGETFEATURE_H
#define QWIDGETFEATURE_H
#include <QApplication>
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QIODevice>
#include <QMessageBox>
#include <QtCharts>
//consider not to include this. Finish with forward declaration

using namespace std;
using namespace QtCharts;

class QChartViewSpikes;
class WidgetIntanChart;
class QWidgetSpikes;
class QWidgetFeature : public QWidget{
    Q_OBJECT
public:
    QWidgetFeature(QWidget* parent=0, int=3);
    void setSpikes(QVector<int>, QVector<QVector<QVector<double>>>,QVector<double>,QVector<double>);
    void setPredictnum(QVector<double>);
    void setTutornum(QVector<double>);
    //void setHitii(int);
    void setHitiis(QVector<int>);
    void calcHitSpike(QPointF, int);
    void calcHitSpikeDragged(QPointF, QPointF, int);
    void drawSpikes();
    void changeSpikes();
    void setAllSpike(bool);
    void setTutorBox(int, bool, QVector<bool>);
    void drawCursorLine(QPointF, QPointF, int);
private:
    QGridLayout *chartLayout;
    QChartViewSpikes *chartview[16];
    QChart *chart[16];
    QVector<QLineSeries*> spikeseries[16];
    //QLineSeries *spikeseries[16][2000];
    QValueAxis *axesX[16];
    QValueAxis *axesY[16];
    double boardSampleRate;
    int nChart;
    int stChart;
    int nChannel;
    QVector<int> indsspike;
    QVector<QVector<QVector<double>>> sspike;
    QPen linepenspike;
    QVector<QPen> linepentutor;
    QPen linepencurrent;
    QPen linepennoise;
    QPen linepencursor;
    QVector<QLineSeries*> cursorline;
    WidgetIntanChart *parentwidget;
    int lenseries;
    int hitii;
    QVector<int> hitiis;
    bool showAllSpike;
    QVector<double> tutornum;
    QVector<double> predictnum;
    QVector<bool> showtutor;
    bool shownoise;
    qreal axisYmax;
    qreal axisYmin;

    void setAxesY();
    QVector<QString> myDialog(QString, QVector<QString>, QVector<QString>);

protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif // QWIDGETFEATURE_H
