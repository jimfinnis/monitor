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
#include "datamgr.h"

#ifndef __APP_H
#define __APP_H

#include "datamgr.h"
#include "audio.h"

/// the main application

class Application : public QApplication, UDPListener {
    Q_OBJECT

public slots:
    void fatalError(const QString& details);
    void update();
    void waypointTick();
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
    
    void addAudio(const char *warning,DataBuffer<float> *buf,bool speech);
    void checkAudio();
    
    
private:
    class UDPServer *udpServer;
    /// a list of keycode->widget mappings, used by keyPress.
    QHash<int,KeyHandler *>keyHandlers;
    QList<AudioWarning *>audioWarnings;
    
    /// used to keep waypointing protocol state
    DataBuffer<float> *wpStateBuffer;
    /// used for waypointing protocol error state
    DataBuffer<float> *wpErrorBuffer;
};

/// get a pointer to the Application instance
inline Application *getApp(){
    return (Application *)qApp;
}

#endif /* __APP_H */
