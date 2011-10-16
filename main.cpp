#include <QtGui>
#include "mainwindow.h"
#include "videodevice.h"

int main(int argc,char *argv[])
{
    system(". setqt4env");
    QApplication app(argc,argv);
    MainWindow FaceRec;
    FaceRec.resize(465,300);
    FaceRec.show();
    FaceRec.move ((QApplication::desktop()->width() - FaceRec.width())/2,(QApplication::desktop()->height() - FaceRec.height())/2);
    return app.exec();
}
