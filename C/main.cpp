#include <QApplication>
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QIODevice>
#include <QMessageBox>
#include <QtCharts>
#include <masasortMainWindow.h>

using namespace std;
using namespace QtCharts;


int main(int argc, char *argv[])
{

    QApplication app (argc, argv);
    masasortMainWindow window;
    window.grabGesture(Qt::PanGesture);
    window.grabGesture(Qt::PinchGesture);
    window.resize(400, 300);
    window.show();
    return app.exec();


}
