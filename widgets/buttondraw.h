/**
 * @file
 * Draw methods for a button style switch or momentary
 *
 * long description, many sentences.
 * 
 */

#ifndef __BUTTONDRAW_H
#define __BUTTONDRAW_H

#include <QWidget>
#include <QTimer>
#include <QPainter>

#include "config.h"

class ButtonDrawer {
    QWidget *widget;
    QString title;
    int textw;
    bool inverse;
public:
    QColor waitingCol;
    QColor OKCol;
    
    void init(QWidget *w, QString t, bool inv,ConfigRect *pos){
        widget = w;
        title = t;
        inverse = inv;
        
        // work out the size taking the text into account
        
        QFontMetrics metrics(widget->font());
        int textw = metrics.width(title)+10;
        printf("%d > %d\n",textw,pos->minsizex);
        if(textw>pos->minsizex)pos->minsizex=textw;
        
        // and then set the minimum size of the widget
        // accordingly
        widget->setMinimumSize(pos->minsizex,pos->minsizex); // ALWAYS SQUARE
        
        waitingCol = Qt::red;
        OKCol = Qt::gray;
    }
    
    void draw(
              QPaintEvent *event,
              UDPState state) {
        QPainter p(widget);
        
        int w = widget->width();
        int h = widget->height();
        
        int size = w>h?h:w; // length of shortest edge
        
        int x = (w-size)/2;
        int y = (h-size)/2;
        
        int textsecsize = size/3;
        
        QBrush brush(Qt::SolidPattern);
        
        switch(state){
        case INIT:
            // in a switch, we're waiting to see what the remote value
            // is before we can set the current switch position
            brush.setColor(Qt::yellow); // yellow barberpole
            brush.setStyle(Qt::BDiagPattern);
            break;
        case UNSENT:
            brush.setColor(QColor(64,64,64));
            break;
        case UNACK:
            brush.setColor(Qt::white);
            brush.setStyle(Qt::BDiagPattern);
            break;
        case BADACK:
            brush.setColor(Qt::white);
            brush.setStyle(Qt::DiagCrossPattern);
            break;
        case WAITING:
            brush.setColor(waitingCol);
            break;
        case OK:
            brush.setColor(OKCol);
            break;
        }
        
        if(brush.style()!=Qt::SolidPattern)
            p.fillRect(x,y,size,size-textsecsize,Qt::black);
        
        p.fillRect(x,y,size,size-textsecsize,brush);
        p.setPen(inverse?Qt::black:Qt::white);
        p.drawRect(x,y,size-1,size-1);
        
        p.drawText(x,y+size-textsecsize,size,textsecsize,Qt::AlignHCenter | Qt::AlignVCenter,title);
        
    }
};

#endif /* __BUTTONDRAW_H */
