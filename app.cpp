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
#include <QKeyEvent>

#include "app.h"
#include "window.h"
#include "config.h"
#include "datamgr.h"
#include "qcommandline.h"
#include "udp.h"
#include "waypointdialog.h"
#include "widgets/statusBlock.h"
#include "waypoint/waypoint.h"

static QString configFile("config");
static QString waypointFile;

static int port = DEFAULT_PORT;

bool portSetInCmdLine=false;

static const struct QCommandLineConfigEntry conf[] = {
    { QCommandLine::Option, 'p', "port", "UDP port number", QCommandLine::Optional},
    { QCommandLine::Option, 'l', "log", "Log file", QCommandLine::Optional},
    { QCommandLine::Option, 'f', "file", "config file",     QCommandLine::Optional},
    { QCommandLine::Option, 'w', "waypoints", "waypoint file",     QCommandLine::Optional},
    
    { QCommandLine::None, '\0', NULL, NULL, QCommandLine::Default }
};

static int ackerr;
static int ackwasas;


// sets the error code..
extern "C" {
void waypointAckFunc(int e,int was){
    printf("Ack called %d:%d\n",e,was);fflush(stdout);
    extern Application *gApp;
    ackerr = e;
    ackwasas = was;
    // fire off the message a after a bit
    QTimer::singleShot(500,gApp,SLOT(ackMessage()));
    
}
}

// ..which is displayed by this slot once a timer
// has elasped, to avoid ackflooding (this is modal)
void Application::ackMessage(){
    if(ackerr){
        
        int stat = wpStatus();
        int err = stat>>8;
        int state = (stat>>1)&0x7f;
        int txc = stat&1;
            
        QString ss;
        if(err==3)
            ss.sprintf("Comms timeout");
        else
            ss.sprintf("Error: %d  State: %d",err,state);
        
        QMessageBox::critical(NULL,"Critical error",
                              QString("Waypoint transfer failed:\n")+ss);
        
        wpReset();
    } else {
        QString ss("transfer appears complete.");
        if(ackwasas)
            ss += QString("\nTransit set copied to working set.");
        QMessageBox::information(NULL,"Waypoint transfer",ss);
        // should be safe.
        wpCopyTransitToWorking(0);
    }
}    


Application::Application(int argc,char *argv[]) : QApplication(argc,argv){
    DataManager::init();
    StatusColour::initColours();
    
    for(int i=0;i<10;i++)
        keyWindow[i]=NULL;
    
    installEventFilter(this);
    
    wpStateBuffer = DataManager::createFloatBuffer("wpstate",100,0,100);
    wpErrorBuffer = DataManager::createFloatBuffer("wperror",100,0,100);
    
    QCommandLine cmdLine(this);
    cmdLine.setConfig(conf);
    cmdLine.enableHelp(true);
    
    logFileName = DEFAULT_LOGFILE;
    
    wpInit(udpSendFunc,waypointAckFunc);
    
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
    if(!waypointFile.isNull()){
        WaypointDialog::doLoad(NULL,waypointFile);
    }
    
    if(ConfigManager::port>0 && !portSetInCmdLine)
        port = ConfigManager::port;
    
    qDebug() << "Port: " << port << ", Config: " << configFile << endl;
    
    
    QTimer *timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(update()));
    timer->start(ConfigManager::graphicalUpdateInterval);
    
    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(udpSend()));
    timer->start(ConfigManager::sendInterval*1000);
    
    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(waypointTick()));
    timer->start(700);
    
#if DIAMOND
    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(pollDiamond()));
    timer->start(100);
#endif
            
    udpServer = new UDPServer(port);
    udpServer->setListener(this);
    
    UDPClient::getInstance()->setPort(ConfigManager::udpSendPort);
    logFile = NULL;
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

#if DIAMOND
void Application::pollDiamond(){
    DataManager::pollDiamond();
}
#endif


Application::~Application()
{
    UDPClient::getInstance()->send("QUIT=1");
    delete udpServer;
}

void Application::update(){
    DataManager::updateAll();
    checkAudio();
}

void Application::quitAction(){
    UDPClient::getInstance()->send("QUIT=1");
    exit(0);
}

void Application::udpSend(){
    UDPClient::getInstance()->update();
}


void Application::waypointTick(){
    wpTick();
    int stat = wpStatus();
    int err = stat>>8;
    int state = (stat>>1)&0x7f;
    int txc = stat&1;
    
    
    wpErrorBuffer->write(DataManager::getTimeNow(),
                         (float)err);
    wpStateBuffer->write(DataManager::getTimeNow(),
                         (float)state);
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
    quitAction();
}

void Application::processUDP(char *s,int size){
    if(*s=='+'){
        printf("Received: %s\n",s);fflush(stdout);
        wpProcessString(s);
    }
    else {
        DataManager::parsePacket(s,size);
        if(logFile){
            logFile->write(s,size-1); // ignore the null...
            logFile->write("\n",1);
        }
    }
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
    } else if(s=="log"){
        logFileName = v.toString();
    } else if(s=="file"){
        configFile = v.toString();
    } else if(s=="waypoints"){
        waypointFile = v.toString();
    } else 
        throw Exception("bad option in command line");
}
void Application::paramFound(const QString&s, const QVariant& v){
}
void Application::parseError(const QString& s){
    throw Exception().set("command line parse error: %s",s.toLatin1().data());
}


bool Application::keyPress(int key){
    if(keyHandlers.contains(key)){
        printf("Calling handler\n");
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
    
    printf("Inserting key handler for %s\n",keyname);
    if(keyHandlers.contains(key))
        throw Exception().set("we already have a handler for key '%c'",key);
    
    keyHandlers.insert(key,h);
}

    
void Application::addAudio(const char *warning,DataBuffer<float> *buf,bool speech){
    AudioWarning *w = new AudioWarning(warning,buf,speech);
    audioWarnings.append(w);
}
void Application::checkAudio(){
    foreach(AudioWarning *a, audioWarnings){
        a->check();
    }
}

void Application::startLog(){
    if(!logFile){
        logFile = new QFile(logFileName);
        if(logFile->open(QIODevice::WriteOnly|QIODevice::Text)){
            printf("Log file opened\n");
        } else {
            printf("---- LOG FILE FAILED TO OPEN\n");
            delete logFile;
            logFile = NULL;
        }
    }else printf("logfile already open\n");
}

void Application::stopLog(){
    if(logFile){
        printf("Closing logfile\n");
        logFile->flush();
        logFile->close();
        delete logFile;
        logFile =NULL;
    }else printf("logfile already closed\n");
}

bool Application::eventFilter(QObject *obj, QEvent *ev){
    if(ev->type() == QEvent::KeyPress){
        int k = ((QKeyEvent *)ev)->key();
        if(k>=Qt::Key_0 && k<=Qt::Key_9){
            k-=Qt::Key_0;
            printf("key %d got\n",k);
            Window *w = keyWindow[k];
            if(w){
                w->raise();
                w->activateWindow();
                w->showFullScreen();
                return true;
            }
        }
        return keyPress(k);
    }
    return QApplication::eventFilter(obj,ev);
}

