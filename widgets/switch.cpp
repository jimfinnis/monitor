/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */
#include "switch.h"

#include <QPainter>
#include <QTimer>
#include <QGridLayout>

#include "../config.h"
#include "../exception.h"
#include "../datamgr.h"
#include "../tokens.h"
#include "../doubletime.h"
#include "../udp.h"

Switch::Switch(QWidget *parent,Tokeniser *t) :
QWidget(NULL)
{
    bool always=false;
    value = 0;
    renderer = NULL;
    immediate =false;
    char outVar[64]; //!< name of the variable to write to
    char title[64];
    title[0]=0;
    
    ConfigRect pos = ConfigManager::parseRect();
    
    bool done = false;
    
    t->getnextcheck(T_OCURLY);
    
    DataBuffer<float> *buf=NULL;
    
    outVar[0]=0;
    
    while(!done){
        switch(t->getnext()){
        case T_EXPR: // optional, provides a 'feedback' value
        case T_VAR:
            t->rewind();
            buf = ConfigManager::parseFloatSource();
            break;
        case T_OUT:
            t->getnextident(outVar);
            break;
        case T_TITLE:
            t->getnextstring(title);
            break;
        case T_ALWAYS:
            always=true;
            break;
        case T_IMMEDIATE:
            immediate=true;
            break;
        case T_CCURLY:
            done=true;
            break;
        default:
            throw Exception().set("Unexpected '%s'",t->getstring());
        }
    }
    
    if(!outVar[0])
        throw Exception("no output name given for switch");
    
    // create a new outvar
    
    out = new OutValue(outVar,0,always);
    out->listener=this;
    UDPClient::getInstance()->add(out);
    
    if(!title[0])
        strcpy(title,outVar);
    
    layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    main = new SwitchInternal(this);
    label = new QLabel(title);
    label->setAlignment(Qt::AlignCenter);
    label->setMaximumSize(10000,30);
    
    main->setMinimumSize(50,60);
    layout->addWidget(main);
    layout->addWidget(label);
    setLayout(layout);
    
    renderer = buf ? new DataRenderer(main,buf) : NULL;
    
    value = false;
    out->set(value ? 1 : 0);
    
    QGridLayout *l = (QGridLayout*)parent->layout();
    l->addWidget(this,pos.y,pos.x,pos.h,pos.w);
}

void SwitchInternal::mousePressEvent(QMouseEvent *event)
{
    sw->toggle();
}

void SwitchInternal::paintEvent(QPaintEvent *event){
    sw->handlePaint(event,this);
}


void Switch::handlePaint(UNUSED QPaintEvent *p,SwitchInternal *widget){
    
    enum {
        UNSENT, //!< switch changed but data unsent
        NOACK, //!< switch changed, data sent, but has a feedback var and no data yet received
        BADACK, //!< switch changed, data sent, feedback received but it's wrong!
        OK //!< switch changed, data sent, feedback ok OR switch changed, data sent and no feedback.
    } state;
    
    // has data been sent?
    
    if(out->timeChanged > out->timeSent)
        state = UNSENT;
    else {
        if(renderer){ // has a feedback variable
            state = NOACK; // default state - no acknowledgement yet
            DataBuffer<float> *b = renderer->getBuffer()->getFloatBuffer();
            
            // get most recent datum
            Datum<float> *d = b->read(0);
            if(d && d->isRecent()){
                bool v = d->d > 0.5f; // the incoming data is float 0/1
                if(d->t > out->timeSent){
                    // an ack has been received
                    state = (v == value) ? OK : BADACK; // is it OK?
                }
            }
        }else {
            state = OK; // in the case of no feedback, it's always OK if data has been sent.
        }
    }
    
    int w = widget->width();
    int h = widget->height();
    
    // red is off, green is on.
    
    QPainter painter(widget);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QColor col = value ? Qt::green : Qt::red;
    
    float switchWidth = h*0.5;
    if(switchWidth>w)switchWidth=w;
    QPen pen(col);
    pen.setWidth(5);
    
    painter.setPen(pen);
    
    QBrush brush(Qt::SolidPattern);
    switch(state){
    case UNSENT:
        brush.setColor(QColor(64,64,64));
        break;
    case NOACK:
        brush.setColor(Qt::white);
        brush.setStyle(Qt::BDiagPattern);
        break;
    case BADACK:
        brush.setColor(Qt::white);
        brush.setStyle(Qt::DiagCrossPattern);
        break;
    case OK:
        brush.setColor(col);
        break;
    }
    
    int yoffset=0;
    if(!value)
        yoffset=h/2;
    
    painter.fillRect(w/2-switchWidth/2,yoffset,switchWidth,h/2,brush);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(w/2-switchWidth/2,0,switchWidth,h);
}



void Switch::toggle(){
    value = !value;
    out->set(value ? 1 : 0);
    /// ensure immediate send
    if(immediate)
        UDPClient::getInstance()->update();
    // graphical update
    update();
}
    
        
