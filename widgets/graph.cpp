/**
 * \file
 * Brief description. Longer description.
 *
 * 
 */

#include "graph.h"
#include <time.h>
#include <sys/time.h>
#include <QPainter>
#include <QPaintEvent>
#include <QImage>

#include "../config.h"
#include "../exception.h"
#include "../datamgr.h"
#include "../widgetmgr.h"
#include "../tokens.h"

#define DEFAULTWIDTHINSECONDS 30

void Graph::remove(const char *name){
    GraphDataRenderer *r = renderers.value(name);
    if(r){
        delete r;
        renderers.remove(name);
    }
}

void Graph::paintEvent(QPaintEvent *event){
    double w = width();

    tNow = DataManager::getTimeNow();
    
    pixPerSec = w/widthInSeconds;
    
    // create a backing store onto which we draw, to make sure
    // that we have all blend modes available
    QImage image(width(),height(),QImage::Format_RGB32);
    image.fill(0);
    
    // create a painter on that image
    QPainter painter(&image);
    
    
    // this doesn't work on a lot of systems; we'd probably have to
    // refactor with a QImage backing the data or something.
    painter.setCompositionMode(QPainter::CompositionMode_Plus);
    
    
    // now iterate over the renderers
    QHashIterator<QString, GraphDataRenderer *> i(renderers);
    int idx=0;
    while (i.hasNext()) {
        i.next();
        
        QString name = i.key(); // name of variable
        GraphDataRenderer *r = i.value(); // the renderer
        
        r->render(idx++,this,painter);
    }
    
    // write the image to the widget
    QPainter painter2(this);
    QRect dirty = event->rect();
    painter2.drawImage(dirty,image,dirty);
}


Graph::Graph(const char *frameName,Tokeniser *t) : QWidget(NULL){
    
    ConfigRect pos;
    pos.x = -1;
    
    bool done = false;
    
    t->getnextcheck(T_OCURLY);
    widthInSeconds = DEFAULTWIDTHINSECONDS;
    DataBuffer<float> *buf;
    
    while(!done){
        switch(t->getnext()){
        case T_POS:
            pos = ConfigManager::parseRect();
            break;
        case T_TIME:
            widthInSeconds=t->getnextfloat();
            break;
        case T_EXPR:
        case T_VAR:{
            t->rewind();
            buf = ConfigManager::parseFloatSource();
            t->getnextcheck(T_OCURLY);
            GraphFloatRenderer *r = new GraphFloatRenderer(this,buf,t);
            renderers.insert(buf->name,r);
            break;
        }
        case T_CCURLY:
            done=true;
            break;
        default:
            throw UnexpException(t);
        }
    }
    
    if(pos.x < 0)
        throw Exception("no position given for graph");
     
    WidgetManager::addWidget(frameName,this,pos.x,pos.y,pos.w,pos.h);
}

// parse a float var after the "var x {" point
GraphFloatRenderer::GraphFloatRenderer(Graph *g,DataBuffer<float> *b,Tokeniser *t) :
DataRenderer(g,b){
    bool done = false;
    color = QColor(Qt::white);
    width = 1;
    while(!done){
        switch(t->getnext()){
        case T_WIDTH:
            width = t->getnextfloat();
            break;
        case T_COL:
        case T_COLOUR:
        case T_COLOR:
            color = ConfigManager::parseColour(Qt::white);
            break;
        case T_CCURLY:
            done=true;
            break;
        }
    }
}

void GraphFloatRenderer::render(int idx,Graph *g,QPainter &painter){
    double h = g->height();
    double w = g->width();
    DataBuffer<float> *buf = buffer->getFloatBuffer();
    
    float yscale = h/(buf->maxVal-buf->minVal);
    
    QFontMetrics metrics = painter.fontMetrics();
    
    // initialise
    int n = 0;
    float prevx = -1,prevy=-1;
    double prevTime=0;
    
    QString legend = buf->name;
    
    int legendy = 10+metrics.height()*idx;
    QRectF rect(110,legendy,w,metrics.height());
    
    painter.setPen(QPen(Qt::white));
    painter.drawText(rect,Qt::AlignLeft,legend);
    
    QPen pen(color);
    pen.setWidthF(width);
    painter.setPen(pen);
    
    painter.drawLine(10,legendy+10,100,legendy+10);
    
    for(;;){
        Datum<float> *d = buf->read(n++);
        if(!d)break;

        float x = g->tNow-(double)d->t;
        x = w-x*g->pixPerSec;

        float y = d->d - buf->minVal;
        y = h - y*yscale;
        
        if(prevTime){
            painter.drawLine(prevx,prevy,x,y);
            if(x<=0)break;
        }
        
        
        prevy=y;
        prevx=x;
        prevTime=d->t;
    }
    
    
}
