/**
 * \file
 * Brief description. Longer description.
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#include "momentary.h"

#include <QPainter>
#include <QGridLayout>

#include "../config.h"
#include "../exception.h"
#include "../datamgr.h"
#include "../tokens.h"
#include "../doubletime.h"
#include "../udp.h"
#include "../app.h"


Momentary::Momentary(QWidget *parent,Tokeniser *t) :
QWidget(NULL)
{
    bool always=false;
    renderer = NULL;
    immediate =false;
    char outVar[64]; //!< name of the variable to write to
    char keyname[64];
    title="";
    keyname[0]=0;
    outVal=1;
    out=NULL;
    nudge=NULL;
    
    ConfigRect pos = ConfigManager::parseRect();
    QGridLayout *l = (QGridLayout*)parent->layout();
    l->addWidget(this,pos.y,pos.x,pos.h,pos.w);
    setMinimumSize(100,100);
    
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
        case T_SET:
            outVal = t->getnextfloat();
            break;
        case T_TITLE:{
            char str[256];
            t->getnextstring(str);
            title = QString(str);
            
            break;
        }
        case T_ALWAYS:
            always=true;
            break;
        case T_IMMEDIATE:
            immediate=true;
            break;
        case T_NUDGE:{
            char buf[256];
            t->getnextident(buf);
            nudge = ConfigManager::getNudgeable(buf);
            nudgeType = ConfigManager::parseNudgeType();
            break;}
        case T_CCURLY:
            done=true;
            break;
        case T_KEY:
            t->getnextstring(keyname);
            getApp()->setKey(keyname,this);
            break;
        default:
            throw Exception().set("Unexpected '%s'",t->getstring());
        }
    }
    
    if(!outVar[0] && !nudge)
        throw Exception("no output name or nudge given for momentary");
    
    if(nudge && buf)
        throw Exception("cannot have a nudge momentary with a feedback source");
    
    // create a new outvar
    if(!nudge){
        out = new OutValue(outVar,0,always);
        out->listener=this; // so we know when our var was sent
        UDPClient::getInstance()->add(out);
        timer.start(500);
    }
    
    if(title.size()==0)
        title = QString(outVar);
    
    if(keyname[0]){
        QString foo(keyname);
        foo=foo.toUpper();
        foo.append(": ");
        foo.append(title);
        title = foo;
    }
    
    renderer = buf ? new DataRenderer(this,buf) : NULL;
    
    if(out){
        state = UNSENT;
        out->set(outVal);
    } else {
        state = OK;
    }
    
    connect(&timer,SIGNAL(timeout()),this,SLOT(timerTick()));
    
    // initialise the button drawer
    bd.init(this,title,ConfigManager::inverse,&pos);
}

void Momentary::onSend(){
    if(renderer)
        state = UNACK;
    else{
        waitct=0;
        state = WAITING; // no feedback, just go straight to done
    }
    update(); // redraw
}

void Momentary::timerTick(){
    printf("blouk\n");
    stateCheck();
}

void Momentary::stateCheck(){
    DataBuffer<float> *b;
    Datum<float> *d;
    printf("state check state %d waitct %d\n",state,waitct);
    switch(state){
    case UNSENT:
        break;
    case UNACK:
        b = renderer->getBuffer()->getFloatBuffer();
        d = b->read(0);
        // This is unusual in that we explicitly check
        // that the most recent packet arrived AFTER
        // the UDP packet was sent. Best make sure the
        // rover really does send packets used in this
        // in a timely manner!
        if(d && d->isRecent() && d->t > out->timeSent){
            if(d->d > 0.5f) // it's a boolean
                state = OK; // done - no need to go into WAITING, really.
            else
                state = BADACK;
            
        }
        break;
    case BADACK:
        // not much we can do here.
        timer.stop();
        break;
    case WAITING:
        if(waitct++==4){
            state = OK;
            waitct=0;
            timer.stop();
            update();
        }
        break;
    case OK:
        break;
    }
}
                  

void Momentary::onKey(){
    if(nudge){
        waitct=0;
        state = WAITING;
        nudge->nudge(nudgeType);
    } else {
        state = UNSENT;
        out->set(outVal);
        if(immediate)
            UDPClient::getInstance()->update();
    }
    update();
    timer.start(100);
    
}
void Momentary::mousePressEvent(QMouseEvent *event){
    onKey();
    QWidget::mousePressEvent(event);
}


void Momentary::paintEvent(QPaintEvent *event){
    
    stateCheck();
    bd.draw(event,state);
    QWidget::paintEvent(event);
}

