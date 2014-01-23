/**
 * \file
 * Brief description. Longer description.
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#include <stdio.h>

#include "waypointmodel.h"
#include "waypoint/waypoint.h"

WaypointModel::WaypointModel(int wp,QObject *parent) : QAbstractTableModel(parent) {
    waypoint = wp;
}

int WaypointModel::rowCount(const QModelIndex &) const {
    return wpGetNumFields();
}
int WaypointModel::columnCount(const QModelIndex &) const {
    return 3;
}
QVariant WaypointModel::data(const QModelIndex &index, int role) const {
    int n = index.row();
    int c = index.column();
    double *d;
    
    if(role == Qt::DisplayRole || role ==Qt::EditRole){
        switch(c){
        case 0:
            return QString(wpGetFieldName(n));
        case 1:
            d = wpGet(waypoint);
            // were I to just return a double here, I'd get a doublespinbox - which loses
            // a lot of precision, annoyingly.
            return QString::number(d[n]);
        case 2:
            return QString::number(wpGetDefault(n));
        }
    }
    return QVariant();
}

bool WaypointModel::setData(const QModelIndex & index, const QVariant & value, int role){
    bool ok;
    double v;
    
    if(role==Qt::EditRole){
        int n = index.row();
        int c = index.column();
        switch(c){
        case 0:break;
        case 1:
            v = value.toString().toDouble(&ok);
            if(ok){
                double *d = wpGet(waypoint);
                d[n] = v;
                emit dataChanged(index,index);
                return true;
            }
            break;
        case 2:
            v = value.toString().toDouble(&ok);
            if(ok){
                wpSetDefault(n,v);
                emit dataChanged(index,index);
                return true;
            }
            break;
        }
    }
    return false;
}


QVariant WaypointModel::headerData(int section,Qt::Orientation orientation,int role) const {
    if(role == Qt::DisplayRole){
        if(orientation==Qt::Horizontal){
            switch(section){
            case 0:return QString("field");
            case 1:return QString("value");
            case 2:return QString("default");
            }
        }
    }
    return QVariant();
}

Qt::ItemFlags WaypointModel::flags(const QModelIndex &index) const {
    switch(index.column()){
    case 0:
        break;
    case 1:
    case 2:
        return Qt::ItemIsEditable|Qt::ItemIsEnabled;
    }
    return Qt::NoItemFlags;
}
