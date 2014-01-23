#ifndef WAYPOINTDIALOG_H
#define WAYPOINTDIALOG_H

#include <QDialog>
#include "waypointmodel.h"

namespace Ui {
class WaypointDialog;
}


class WaypointDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit WaypointDialog(int wp,QWidget *parent = 0);
    ~WaypointDialog();
    
    /// does not open a dialog
    static void doLoad(QWidget *parent,QString fn);
    /// opens a dialog
    static void load(QWidget *parent=0);
    /// opens a dialog
    static void save(QWidget *parent=0);
    
    
private:
    Ui::WaypointDialog *ui;
    WaypointModel *model;
    double *oldData;
    int waypoint;
public slots:
    void onReject();
    void onSave();
    void onLoad();
};

#endif // WAYPOINTDIALOG_H
