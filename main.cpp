#include <QtGui/QApplication>
#include "mainwindow.h"
#include "qcommandline.h"
#include "datamgr.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    
    // try to grow the frames within the window
//    w.showFullScreen();
    w.show();

    return a.exec();
}
