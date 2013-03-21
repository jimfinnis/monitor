/**
 * \file
 * Brief description. Longer description.
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#include <QPainter>
#include <QPaintEvent>

#include "compassPanel.h"

void CompassPanel::paintEvent(QPaintEvent *e){
    QPainter p(this);
    
    QColor col = compass->invalid ? QColor(32,32,32) : Qt::blue;
    
    float minsize = width()<height()?width():height();
    
    p.fillRect(0,0,width(),height(),QBrush(col));
        
    static const QPointF points[3]= {
        QPointF(-0.05,0.2),
        QPointF(0.05,0.2),
        QPointF(0,-0.4)
    };
    
    p.translate(width()/2,height()/2);
    p.scale(minsize,minsize);
    
    if(!compass->invalid)
        p.rotate(compass->value); // in DEGREES
        
    p.setPen(Qt::black);
    p.setBrush(Qt::white);
    p.drawPolygon(points,3);
}
