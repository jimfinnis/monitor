/**
 * \file
 * Brief description. Longer description.
 *
 * 
 */

#include "gauge.h"

#include <QPainter>
#include <QTimer>

#include "../config.h"
#include "../exception.h"
#include "../datamgr.h"
#include "../tokens.h"

static QColor defaultGreenCol(0,255,0);
static QColor defaultRedCol(255,0,0);
static QColor defaultYellowCol(255,255,0);
static int defaultDarkFactor = 400;

Gauge::Gauge(QWidget *parent,Tokeniser *t) :
    QWidget(NULL)
{
    value = 0;
    renderer = NULL;
    
    char title[64];
    title[0]=0;
    char subtitle[64];
    fontScale=1;
    subtitle[0]=0;
    
    ConfigRect pos;
    
    bool done = false;

    pos = ConfigManager::parseRect();
    
    t->getnextcheck(T_OCURLY);
    
    DataBuffer<float> *buf=NULL;
    redLevel = 0.8;
    yellowLevel = 0.6;
    
    static float prevRed=-1;
    static float prevYellow=-1;
    
    static QColor prevGreenCol = defaultGreenCol;
    static QColor prevRedCol = defaultRedCol;
    static QColor prevYellowCol = defaultYellowCol;
    static int prevDarkFactor = defaultDarkFactor;
    
    redCol = defaultRedCol;
    greenCol = defaultGreenCol;
    yellowCol = defaultYellowCol;
    darkFactor = defaultDarkFactor;
    
    inverse = ConfigManager::inverse;
    
    while(!done){
        switch(t->getnext()){
        case T_FONTSCALE:
            fontScale = t->getnextfloat();
            break;
        case T_EXPR:
        case T_VAR:
            t->rewind();
            buf = ConfigManager::parseFloatSource();
            break;
        case T_TITLE:
            t->getnextstring(title);
            break;
        case T_SUBTITLE:
            t->getnextstring(subtitle);
            break;
        case T_CCURLY:
            done=true;
            break;
        case T_COLOURS:
            if(t->getnext()==T_PREVIOUS){
                greenCol=prevGreenCol;
                yellowCol=prevYellowCol;
                redCol=prevRedCol;
            } else {
                t->rewind();
                greenCol = ConfigManager::parseColour(Qt::green);
                yellowCol = ConfigManager::parseColour(Qt::yellow);
                redCol = ConfigManager::parseColour(Qt::red);
                prevGreenCol = greenCol;
                prevYellowCol = yellowCol;
                prevRedCol = redCol;
            }
            break;
        case T_DARKEN:
            if(t->getnext()==T_PREVIOUS){
                darkFactor = prevDarkFactor;
            }else{
                t->rewind();
                darkFactor = t->getnextint();
            }
            break;
        case T_LEVELS:
            {
            if(!buf)
                throw ParseException(t,"specify source before levels");
                if(t->getnext()==T_PREVIOUS){
                    if(prevRed<0)
                        throw ParseException(t,"no previous level set");
                    redLevel = prevRed;
                    yellowLevel = prevYellow;
                }else{
                    t->rewind();
                    yellowLevel = t->getnextfloat();
                    if(!buf->isAutoRange())
                        yellowLevel = (yellowLevel-buf->minVal)/(buf->maxVal-buf->minVal);
                    redLevel = t->getnextfloat();
                    if(!buf->isAutoRange())
                        redLevel = (redLevel-buf->minVal)/(buf->maxVal-buf->minVal);
                    prevRed=redLevel;
                    prevYellow=yellowLevel;
                }
            }
            break;
        default:
            throw Exception(t->getline()).set("Unexpected '%s'",t->getstring());
        }
    }
    
    if(!buf)
        throw Exception("no data source given for gauge",t->getline());
    
    if(!title[0])
        strcpy(title,buf->name);
    
    layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    main = new GaugeInternal(this);
    label = new QLabel(title);
    ConfigManager::setStyle(label);
    label->setAlignment(Qt::AlignCenter);
    if(subtitle[0]){
        label2 = new QLabel(subtitle);
        label2->setMaximumSize(10000,40);
        label2->setAlignment(Qt::AlignCenter);
        label2->setStyleSheet("font:8pt;");
    }else label2=NULL;
    

    main->setMinimumSize(pos.minsizex,pos.minsizey);
    label->setMaximumSize(10000,20);
    layout->addWidget(main);
    layout->addWidget(label);
    if(label2)
        layout->addWidget(label2);
    setLayout(layout);
    
    label->setContentsMargins(0,0,0,0);
    main->setContentsMargins(0,0,0,0);
    setContentsMargins(0,0,0,0);
    layout->setContentsMargins(0,0,0,0);
    
    renderer = new DataRenderer(main,buf);
    
    QGridLayout *l = (QGridLayout*)parent->layout();
    l->addWidget(this,pos.y,pos.x,pos.h,pos.w);
}

void GaugeInternal::paintEvent(QPaintEvent *event){
    gauge->handlePaint(event,this);
    QWidget::paintEvent(event);
}


void Gauge::handlePaint(UNUSED QPaintEvent *p,GaugeInternal *widget){
    
    float maxVal=100;
    bool centered=false;
    
    bool invalid = true;
    if(renderer){
        DataBuffer<float> *b = renderer->getBuffer()->getFloatBuffer();
        // set range and centering, rather crudely
        if(b->minVal < -0.0001f) centered=true;
        maxVal = b->maxVal;
        
        // get most recent datum
        Datum<float> *d = b->read(0);
        if(d && d->isRecent()){
            value = d->d; // get data if any recent
            invalid = false;
        }
    }

    int w = widget->width();
    int h = widget->height();
    
    float qq = inverse ? 300 : 100;

    QColor redOn = redCol.darker(qq);
    QColor redOff = redOn.darker(darkFactor);
    QColor yellowOn = yellowCol.darker(qq);
    QColor yellowOff = yellowOn.darker(darkFactor);
    QColor greenOn = greenCol.darker(qq);
    QColor greenOff = greenOn.darker(darkFactor);

    QColor grey(QColor(64,64,64));

    QPainter painter(widget);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(0,0,w,h,inverse?Qt::white:
                     Qt::black);

    float sweep = 270;
    float tickWidth = 10;
    float tickMinRadius = 70;
    float tickMaxRadius = 95;
    int ticks = 21;
    bool showValue = true;

    int tickYellow = yellowLevel*ticks;
    int tickRed = redLevel*ticks;

    if(centered){tickYellow/=2; tickRed/=2;}

    float t; // get this into 0-1 range
    if(centered)
        t = (value + maxVal)/(maxVal*2);
    else
        t = value / maxVal;

    int valueTick = t*ticks;

    painter.translate(w/2,h/2);    // centre the origin
    int shortestSide = qMin(w,h);
    painter.scale(shortestSide/200.0,shortestSide/200.0); // set to a 200x200 square (-100 to 100)

    if(invalid)
        painter.setBrush(grey);
    else
        painter.setBrush(greenOn);

    float angle = -sweep*0.5;
    float step = sweep/(float)(ticks-1);
    bool on= centered ? false : true;

    QFont f = painter.font();
    f.setPointSizeF(f.pointSizeF()*fontScale*1.5);
    painter.setFont(f);

    for(int i=0;i<ticks;i++){
        if(i==valueTick)on=!on;
        if(centered && i==ticks/2)on=!on;

        painter.save();
        painter.rotate(angle+180);
        painter.translate(0,tickMinRadius);

        painter.setPen(Qt::NoPen);
        int q = centered ? abs(i-ticks/2) : i;
        if(!invalid){
            if(q>=tickRed)
                painter.setBrush(on?redOn:redOff);
            else if(q>=tickYellow)
                painter.setBrush(on?yellowOn:yellowOff);
            else
                painter.setBrush(on?greenOn:greenOff);
        }
        
        if(!inverse || on){ // off ticks are not drawn in inverse
            painter.drawRect(-tickWidth/2,0,tickWidth,tickMaxRadius-tickMinRadius);
            angle += step;
        }
        painter.restore();

        if(showValue){
            painter.setPen(invalid?grey:
                           (inverse?Qt::black:Qt::white));
            painter.drawText(-100,-50,200,100,Qt::AlignCenter,
                             invalid ? QString("no data") : QString::number(value,'g',4));
        }
    }

}

