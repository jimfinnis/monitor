/**
 * \file
 * Defines the main application class.
 * 
 */


#include <QtGui/QApplication>
#include <QGridLayout>
#include <QHash>
#include <QFile>
#include "window.h"
#include "udp.h"
#include "keyhandler.h"
#include "datamgr.h"

#ifndef __APP_H
#define __APP_H

#include "datamgr.h"
#include "audio.h"
#include "defaults.h"

/// the main application

class Application : public QApplication, UDPListener {
    Q_OBJECT

public slots:
    void fatalError(const QString& details);
    void update();
    void waypointTick();
#if DIAMOND
    void pollDiamond();
#endif
    void udpSend();
    void switchFound(const QString& s);
    void optionFound(const QString&s, const QVariant& v);
    void paramFound(const QString&s, const QVariant& v);    
    void parseError(const QString& s);
    void ackMessage();
    
signals:
    void mapreset();
public:
    
    /// create a window with a layout and central widget,
    /// add it to the application. Returns the window;
    /// you can call centralWidget()->layout() to get the layout.
    
    Window *createWindow();
    
    explicit Application(int argc,char *argv[]);
    virtual void processUDP(char *s,int size);
    
    ~Application();
    
    /// handle a key press in any window
    bool keyPress(int key);
    void setKey(const char *keyname, KeyHandler *h);
    
    /// tell all maps to reset themselves
    void resetAllMaps(){
        emit mapreset();
    }
    
    /// tell the logging system to start logging to a file ("capture.log" by default)
    void startLog();
    void stopLog();
    
    /// exit the application, first sending a QUIT=1 message over UDP.
    void quitAction();
    
    void addAudio(const char *warning,DataBuffer<float> *buf,bool speech);
    /// periodic check for audio warnings
    void checkAudio();
    
    /// set a key to switch to a window - n between 0 and 9.
    void setWindowKey(int n,Window *w){
        keyWindow[n]=w;
    }
    
protected:
    bool eventFilter(QObject *obj, QEvent *ev);
    
private:
    
    /// the name of the logfile
    QString logFileName;
    
    /// the UDP server object, which reads data from the robot
    class UDPServer *udpServer;
    
    /// a list of keycode->widget mappings, used by keyPress.
    QHash<int,KeyHandler *>keyHandlers;
    QList<AudioWarning *>audioWarnings;
    
    /// used to keep waypointing protocol state
    DataBuffer<float> *wpStateBuffer;
    /// used for waypointing protocol error state
    DataBuffer<float> *wpErrorBuffer;
    
    /// the actual log file, which is null if we're not logging
    QFile *logFile;
    
    /// windows to be switched to on keypress
    Window *keyWindow[10];
};

/// get a pointer to the Application instance
inline Application *getApp(){
    return (Application *)qApp;
}

#endif /* __APP_H */
