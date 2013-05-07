/**
 * \file
 * Brief description. Longer description.
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <QMessageBox>
#include <QDesktopWidget>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <QVariant>
#include <QThread>

#include "app.h"
#include "window.h"
#include "config.h"
#include "datamgr.h"
#include "qcommandline.h"
#include "udp.h"
#include "worker.h"

static QString configFile("config");
static int port = 13231;

bool portSetInCmdLine=false;

static const struct QCommandLineConfigEntry conf[] = {
    { QCommandLine::Option, 'p', "port", "UDP port number", QCommandLine::Optional},
    { QCommandLine::Option, 'f', "file", "config file",     QCommandLine::Optional},
    
    { QCommandLine::None, '\0', NULL, NULL, QCommandLine::Default }
};


/*
 * You might be wondering why I've done this. Well, it turns out that
 * QTimer behaves differently on Fedora and Ubuntu, let alone Windows
 * and MacOS. On Fedora, it works fine. On Ubuntu, the timer only runs
 * WHILE THE MOUSE IS MOVING. ffs. So I've started a thread which runs
 * in a timed loop (Worker) which sends signals to the things it should.
 */

Worker *Application::createTimer(const char *n,int interval){
    QThread *thread = new QThread;
    Worker *worker = new Worker(n,interval);
    printf("Worker; %p\n",worker);
    worker->moveToThread(thread);
    connect(thread,SIGNAL(started()),worker,SLOT(process()));
    connect(worker,SIGNAL(finished()),thread,SLOT(quit()));
    connect(worker,SIGNAL(finished()),worker,SLOT(deleteLater()));
    connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()));
    thread->start();
    return worker;
}

        
Application::Application(int argc,char *argv[]) : QApplication(argc,argv){
    
    DataManager::init();

    QCommandLine cmdLine(this);
    cmdLine.setConfig(conf);
    cmdLine.enableHelp(true);
    
    
    connect(&cmdLine, SIGNAL(switchFound(const QString &)),
            this, SLOT(switchFound(const QString &)));
    connect(&cmdLine, SIGNAL(optionFound(const QString &, const QVariant &)),
            this, SLOT(optionFound(const QString &, const QVariant &)));
    connect(&cmdLine, SIGNAL(paramFound(const QString &, const QVariant &)),
            this, SLOT(paramFound(const QString &, const QVariant &)));
    connect(&cmdLine, SIGNAL(parseError(const QString &)),
            this, SLOT(parseError(const QString &)));
    cmdLine.parse();
    
    ConfigManager::parseFile(configFile);
    
    if(ConfigManager::port>0 && !portSetInCmdLine)
        port = ConfigManager::port;
    
    qDebug() << "Port: " << port << ", Config: " << configFile << endl;
    
    connect(createTimer("gfx",ConfigManager::graphicalUpdateInterval),
            SIGNAL(tick()),this,SLOT(update()));
    connect(createTimer("udpsend",ConfigManager::sendInterval*1000),
            SIGNAL(tick()),this,SLOT(udpSend()));
    
    udpServer = new UDPServer(port);
    udpServer->setListener(this);
    
    UDPClient::getInstance()->setAddress(ConfigManager::udpSendAddr,
                                         ConfigManager::udpSendPort);
    
}

Window *Application::createWindow(){
    Window *w = new Window();
    // central widget required with a grid layout
    w->setCentralWidget(new QWidget());
    QGridLayout *l = new QGridLayout;
    l->setHorizontalSpacing(6);
    l->setVerticalSpacing(6);
    w->centralWidget()->setLayout(l);
    return w;
}


Application::~Application()
{
    delete udpServer;
}

void Application::update(){
    printf("tick\n");
    DataManager::updateAll();
}

void Application::udpSend(){
    UDPClient::getInstance()->update();
}

void Application::fatalError(const QString &details)
{
    QMessageBox mbox(QMessageBox::Critical,
                     "Fatal Error",
                     "A fatal error has occurred",
                     QMessageBox::Ok
                     );
    mbox.setDetailedText(details);
    mbox.exec();
    quit();
}

void Application::processUDP(char *s,int size){
    DataManager::parsePacket(s,size);
}



/*
 * 
 * Command line parsing
 * 
 */

void Application::switchFound(const QString& s){
}
void Application::optionFound(const QString&s, const QVariant& v){
    bool ok;
//    qDebug() << "Option: " << s << " Value: " <<v;
    
    if(s=="port"){
        port = v.toInt(&ok);
        if(!ok)throw Exception("bad port specified");
        portSetInCmdLine=true;
    } else if(s=="file"){
        configFile = v.toString();
    }
}
void Application::paramFound(const QString&s, const QVariant& v){
}
void Application::parseError(const QString& s){
    throw Exception().set("error: %s",s.toLatin1().data());
}


bool Application::keyPress(int key){
    if(keyHandlers.contains(key)){
        keyHandlers.value(key)->onKey();
        return true;
    }else 
          return false;
}

void Application::setKey(const char *keyname, KeyHandler *h){
    // keyname is either a character like 'a' or '1', or an integer
    // Qt keycode.
    
    int key;
    if(strlen(keyname)==1)
        key = toupper(keyname[0]);
    else if(!strcasecmp(keyname,"home"))
        key = Qt::Key_Home;
    else if(!strcasecmp(keyname,"end"))
        key = Qt::Key_End;
    else if(!strcasecmp(keyname,"pgup"))
        key = Qt::Key_PageUp;
    else if(!strcasecmp(keyname,"pgdn"))
        key = Qt::Key_PageDown;
    else if(!strcasecmp(keyname,"ins"))
        key = Qt::Key_Insert;
    else if(!strcasecmp(keyname,"del"))
        key = Qt::Key_Delete;
    else if(!strcasecmp(keyname,"up"))
        key = Qt::Key_Up;
    else if(!strcasecmp(keyname,"down"))
        key = Qt::Key_Down;
    else if(!strcasecmp(keyname,"left"))
        key = Qt::Key_Left;
    else if(!strcasecmp(keyname,"right"))
        key = Qt::Key_Right;
    else key = atoi(keyname);
    
    if(keyHandlers.contains(key))
        throw Exception().set("we already have a handler for key '%c'",key);
    
    keyHandlers.insert(key,h);
}
    
