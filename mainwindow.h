#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <marble/MarbleWidget.h>

#include "udp.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public UDPListener
{
    Q_OBJECT

public slots:
    void fatalError(const QString& details);
    void update();
    void switchFound(const QString& s);
    void optionFound(const QString&s, const QVariant& v);
    void paramFound(const QString&s, const QVariant& v);    
    void parseError(const QString& s);
    
    
public:
    explicit MainWindow(QWidget *parent = 0);
    virtual void process(const char *s,int size);


    ~MainWindow();
    

private:
    Ui::MainWindow *ui;
    UDPServer *udpServer;
};

#endif // MAINWINDOW_H
