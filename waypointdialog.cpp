#include "waypointdialog.h"
#include "ui_waypointdialog.h"
#include "waypoint/waypoint.h"

#include <QtWidgets>

WaypointDialog::WaypointDialog(int wp,QWidget *parent) :
QDialog(parent),ui(new Ui::WaypointDialog)
{
    // first thing - copy the old data.
    waypoint=wp;
    double *d = wpGet(wp);
    oldData = new double[wpGetNumFields()];
    for(int i=0;i<wpGetNumFields();i++){
        oldData[i]=d[i];
    }
    
    connect(this,SIGNAL(rejected()),this,SLOT(onReject()));
    
    
    model = new WaypointModel(wp);
    // make the parent redraw if the data changes
    connect(model,SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            parent,
            SLOT(update()));
    
    ui->setupUi(this);
    
    ui->tableView->setModel(model);
    // make the table fill its space
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
//    connect(ui->saveButton,SIGNAL(clicked(bool)),this,SLOT(onSave()));
//    connect(ui->loadButton,SIGNAL(clicked(bool)),this,SLOT(onLoad()));
    
}

WaypointDialog::~WaypointDialog()
{
    delete ui;
    delete model;
    delete oldData;
}


void WaypointDialog::onReject(){
    // put the old data back
    double *d = wpGet(waypoint);
    for(int i=0;i<wpGetNumFields();i++){
        d[i]=oldData[i];
    }
}

static void fileError(int err,QString file){
    QMessageBox mbox(QMessageBox::Critical,
                     "Cannot open file",
                     QString("File : "+file+"\nError code: ")+QString::number(err),
                     QMessageBox::Ok
                     );
    mbox.exec();
    
}

void WaypointDialog::save(QWidget *parent){
    QString fileName = QFileDialog::getSaveFileName(parent,"Save Waypoint File",QString("waypoints.txt"),"Waypoint Files (*.txt *.wyp)");
    if(fileName.isNull())
        return;
    
    qDebug() << "Save: " << fileName;
    
    QByteArray ba = fileName.toLatin1();
    int rv = wpSave(ba.data());
    
    if(rv)
        fileError(rv,fileName);
    else
        QMessageBox::information(parent,"Save",
                                 QString::number(wpGetCount())+
                                 QString(" points saved."));
}

void WaypointDialog::onSave(){
    save(this);
}

void WaypointDialog::doLoad(QWidget *parent,QString fn){
    QByteArray ba = fn.toLatin1();
    int rv = wpLoad(ba.data());
    
    if(rv)
        fileError(rv,fn);
    else {
        QString status = QString::number(wpGetCount())+
              QString(" points loaded.");
        if(wpLoadStatus()&1)
            status += "\nUnknown fields were specified";
        if(wpLoadStatus()&2){
            status += "\nSpecified fields were not provided";
            status += "\n (and have been set to defaults)";
        }

        QMessageBox::information(parent,"Load",status);
    }
}

void WaypointDialog::load(QWidget *parent){
    QString fileName = QFileDialog::getOpenFileName(parent,"Open Waypoint File",QString(),"Waypoint Files (*.txt *.wyp)");
    if(fileName.isNull())
        return;
    doLoad(parent,fileName);
    qDebug() << "Load: " << fileName;
}

void WaypointDialog::onLoad(){
    load(this);
}

