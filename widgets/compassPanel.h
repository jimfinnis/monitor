/**
 * \file
 * Brief description. Longer description.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __COMPASSPANEL_H
#define __COMPASSPANEL_H

#include "compass.h"

/// the core class for drawing a compass
class CompassPanel : public QWidget {
    Q_OBJECT
          
public:
    
    virtual ~CompassPanel(){}
    
    CompassPanel(Compass *c) : QWidget(NULL){
        compass = c;
    }
    
    virtual void paintEvent(QPaintEvent *e);
    
public slots:
    void dataChanged(){
        update();
    }
private:
    Compass *compass;
};



#endif /* __COMPASSPANEL_H */
