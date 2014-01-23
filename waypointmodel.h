/**
 * \file
 * Brief description. Longer description.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __WAYPOINTMODEL_H
#define __WAYPOINTMODEL_H

#include <QAbstractTableModel>

class WaypointModel : public QAbstractTableModel {
    Q_OBJECT
    int waypoint; //!< the waypoint we're looking at
public:
    
    WaypointModel(int wp,QObject *parent=0);
    
    QVariant headerData(int section,Qt::Orientation orientation,int role) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
};



#endif /* __WAYPOINTMODEL_H */
