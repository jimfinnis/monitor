#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <stdio.h>
#include <math.h>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QLabel>
#include <QTimer>

#include "qcommandline.h"


#include "datamgr.h"
#include "widgetmgr.h"
#include "config.h"

#include "widgets/gauge.h"

static QString configFile("config");
static int port = 13231;

bool portSetInCmdLine=false;

static const struct QCommandLineConfigEntry conf[] = {
    { QCommandLine::Option, 'p', "port", "UDP port number", QCommandLine::Optional},
    { QCommandLine::Option, 'f', "file", "config file",     QCommandLine::Optional},
    
    { QCommandLine::None, '\0', NULL, NULL, QCommandLine::Default }
};
        

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    setStyleSheet("background-color: black; color:white;");

    WidgetManager::addFrame("right",ui->rightFrame);
    WidgetManager::addFrame("bottom",ui->bottomFrame);
    WidgetManager::addFrame("mapframe",ui->mapFrame);

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
    timer->start(2000);
    
    udpServer = new UDPServer(port);
    udpServer->setListener(this);
    
    UDPClient::getInstance()->setAddress(ConfigManager::udpSendAddr,
                                         ConfigManager::udpSendPort);
}

void MainWindow::switchFound(const QString& s){
}
void MainWindow::optionFound(const QString&s, const QVariant& v){
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
void MainWindow::paramFound(const QString&s, const QVariant& v){
}
void MainWindow::parseError(const QString& s){
    throw Exception().set("error: %s",s.toLatin1().data());
}



void MainWindow::update(){
    DataManager::updateAll();
    UDPClient::getInstance()->update();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete udpServer;
}

void MainWindow::fatalError(const QString &details)
{
    QMessageBox mbox(QMessageBox::Critical,
                     "Fatal Error",
                     "A fatal error has occurred",
                     QMessageBox::Ok,
                     this);
    mbox.setDetailedText(details);
    mbox.exec();
    qApp->quit();
}

void MainWindow::process(const char *s,int size){
    
    DataManager::parsePacket(s,size);
}

