/**
 * \file
 * Brief description. Longer description.
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#include <stdio.h>
#include <math.h>

#include <QMessageBox>
#include <QDesktopWidget>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <QVariant>

#include "app.h"
#include "config.h"
#include "datamgr.h"
#include "qcommandline.h"
#include "udp.h"

static QString configFile("config");
static int port = 13231;

bool portSetInCmdLine=false;

static const struct QCommandLineConfigEntry conf[] = {
    { QCommandLine::Option, 'p', "port", "UDP port number", QCommandLine::Optional},
    { QCommandLine::Option, 'f', "file", "config file",     QCommandLine::Optional},
    
    { QCommandLine::None, '\0', NULL, NULL, QCommandLine::Default }
};
        
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
    
    
    QTimer *timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(update()));
    timer->start(2000); // graphical update interval
    
    // separate timer for UDP send interval
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(udpSend()));
    timer->start(ConfigManager::sendInterval * 1000.0f);
    
    
    udpServer = new UDPServer(port);
    udpServer->setListener(this);
    
    UDPClient::getInstance()->setAddress(ConfigManager::udpSendAddr,
                                         ConfigManager::udpSendPort);
    
}

QMainWindow *Application::createWindow(){
    QMainWindow *w = new QMainWindow();
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

void Application::processUDP(const char *s,int size){
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

