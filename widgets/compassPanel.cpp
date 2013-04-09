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

#include "config.h"
#include "compassPanel.h"

void CompassPanel::paintEvent(QPaintEvent *e){
    bool inv = compass->inverse;
    
    QPainter p(this);
    
    QColor col = compass->invalid ? QColor(32,32,32) : 
    (inv?Qt::white:Qt::blue);
    
    
    float minsize = width()<height()?width():height();
    float ratio = (float)width()/(float)height();
    p.fillRect(0,0,width(),height(),QBrush(col));
        
    p.setPen(Qt::black);
    p.setBrush(inv?Qt::black:Qt::white);
    
    static const QPointF points[3]= {
        QPointF(-0.05,0.2),
        QPointF(0.05,0.2),
        QPointF(0,-0.4)
    };
    
    p.translate(width()/2,height()/2);
    p.scale(width(),height());
    
    if(!compass->invalid)
        p.rotate(compass->value); // in DEGREES
        
    p.drawPolygon(points,3);
}
