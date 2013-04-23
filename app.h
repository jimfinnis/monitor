/**
 * \file
 * Defines the main application class.
 * 
 */


#include <QtGui/QApplication>
#include <QGridLayout>
#include <QHash>
#include "window.h"
#include "udp.h"
#include "keyhandler.h"

#ifndef __APP_H
#define __APP_H

/// the main application

class Application : public QApplication, UDPListener {
    Q_OBJECT

public slots:
    void fatalError(const QString& details);
    void update();
    void udpSend();
    void switchFound(const QString& s);
    void optionFound(const QString&s, const QVariant& v);
    void paramFound(const QString&s, const QVariant& v);    
    void parseError(const QString& s);
    
public:
    
    /// create a window with a layout and central widget,
    /// add it to the application. Returns the window;
    /// you can call centralWidget()->layout() to get the layout.
    
    Window *createWindow();
    
    explicit Application(int argc,char *argv[]);
    virtual void processUDP(char *s,int size);
    
    ~Application();
    
    /// handle a key press in any window
    void keyPress(int key);
    void setKey(const char *keyname, KeyHandler *h);
private:
    class UDPServer *udpServer;
    /// a list of keycode->widget mappings, used by keyPress.
    QHash<int,KeyHandler *>keyHandlers;
};

/// get a pointer to the Application instance
inline Application *getApp(){
    return (Application *)qApp;
}

#endif /* __APP_H */
