#ifndef QPOLYGONSPIKE_H
#define QPOLYGONSPIKE_H

#include <QWidget>
#include <QApplication>
#include <QString>

using namespace std;

class QPolygonSpike : public QPolygon {
public:
    QPolygonSpike();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // QPOLYGONSPIKE_H
