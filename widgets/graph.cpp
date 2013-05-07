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
#include <QGridLayout>
#include <QImage>

#include "../config.h"
#include "../exception.h"
#include "../datamgr.h"
#include "../tokens.h"

#define DEFAULTWIDTHINSECONDS 30

void Graph::paintEvent(QPaintEvent *event){
    double w = width();

    tNow = DataManager::getTimeNow() - DataManager::packetTimeOffset;
    
    pixPerSec = w/widthInSeconds;
    
    // create a backing store onto which we draw, to make sure
    // that we have all blend modes available
    QImage image(width(),height(),QImage::Format_RGB32);
    image.fill(inverse?0xffffffff:0);
    
    // create a painter on that image
    QPainter painter(&image);
    
    
    QFont font("Sans",8,QFont::Normal);
    painter.setFont(font);
    
    // now iterate over the renderers
    
    for(int i=0;i<renderers.size();i++){
        renderers[i]->render(i,this,painter);
    }
    
    // write the image to the widget
    QPainter painter2(this);
    QRect dirty = event->rect();
    painter2.drawImage(dirty,image,dirty);
    QWidget::paintEvent(event);
    
}


Graph::Graph(QWidget *parent,Tokeniser *t) : QWidget(NULL){
    
    ConfigRect pos;
    pos.x = -1;
    
    bool done = false;
    
    pos = ConfigManager::parseRect();
    inverse = ConfigManager::inverse;
    
    t->getnextcheck(T_OCURLY);
    widthInSeconds = DEFAULTWIDTHINSECONDS;
    DataBuffer<float> *buf;
    
    while(!done){
        switch(t->getnext()){
        case T_TIME:
            widthInSeconds=t->getnextfloat();
            break;
        case T_EXPR:
        case T_VAR:{
            t->rewind();
            buf = ConfigManager::parseFloatSource();
            t->getnextcheck(T_OCURLY);
            GraphFloatRenderer *r = new GraphFloatRenderer(this,buf,t);
            renderers.append(r);
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
     
    setMinimumSize(pos.minsizex,pos.minsizey);
    QGridLayout *l = (QGridLayout*)parent->layout();
    l->addWidget(this,pos.y,pos.x,pos.h,pos.w);
    
    ConfigManager::setStyle(this);
}

// parse a float var after the "var x {" point
GraphFloatRenderer::GraphFloatRenderer(Graph *g,DataBuffer<float> *b,Tokeniser *t) :
DataRenderer(g,b){
    bool done = false;
    color = QColor(g->inverse?Qt::black:Qt::white);
    width = g->inverse?3:1;
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
    
    // and work out the range, which should cover an area slightly 
    // wider than the buffer
    
    float w = (b->maxVal - b->minVal)*0.1f; // width of extra coverage
    minVal = b->minVal - w;
    maxVal = b->maxVal + w;
}

void GraphFloatRenderer::render(int idx,Graph *g,QPainter &painter){
    double h = g->height();
    double w = g->width();
    DataBuffer<float> *buf = buffer->getFloatBuffer();
    
    float yscale = h/(maxVal-minVal);
    
    QFontMetrics metrics = painter.fontMetrics();
    
    // initialise
    int n = 0;
    float prevx = -1,prevy=-1;
    double prevTime=0;
    
    QString legend = buf->name;
    
    int legendy = 10+metrics.height()*idx;
    QRectF rect(110,legendy,w,metrics.height());
    
    painter.setPen(QPen(g->inverse?
                        Qt::black:
                        Qt::white));
    painter.drawText(rect,Qt::AlignLeft,legend);
    
    QPen pen(color);
    pen.setWidthF(width);
    painter.setPen(pen);
    
    painter.drawLine(10,legendy+10,100,legendy+10);
    
    if(!g->inverse)
        painter.setCompositionMode(QPainter::CompositionMode_Plus);
    for(;;){
        Datum<float> *d = buf->read(n++);
        if(!d)break;

        float x = g->tNow-(double)d->t;
        x = w-x*g->pixPerSec;

        float y = d->d - minVal;
        y = h - y*yscale;
        
        if(prevTime){
            painter.drawLine(prevx,prevy,x,y);
            if(x<=0)break;
        }
        
        
        prevy=y;
        prevx=x;
        prevTime=d->t;
    }
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
}
