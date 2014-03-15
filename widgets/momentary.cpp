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

/// momentary button states 


static class MOMOKState : public State<Momentary> {
public:
    virtual UDPState getUDPState(){return OK;}
    virtual void onClick(Momentary *t);
} okState;

static class MOMUnsentState : public State<Momentary> {
public:
    virtual UDPState getUDPState(){return UNSENT;}
    virtual void onDataSent(Momentary *t);
} unsentState;

static class MOMUnackState : public State<Momentary> {
public:
    virtual UDPState getUDPState(){return UNACK;}
    virtual void onNewData(Momentary *t,float v);
    virtual void onClick(Momentary *t);
} unackState ;

static class MOMBadackState : public State<Momentary> {
public:
    virtual UDPState getUDPState(){return BADACK;}
    virtual void onClick(Momentary *t);
} badackState;

static class MOMWaitState : public State<Momentary> {
public:
    virtual UDPState getUDPState(){return WAITING;}
    virtual void onEnter(State<Momentary> &prevState, Momentary *t);
    virtual void onTimerTick(Momentary *t);
    virtual void onClick(Momentary *t);
} waitState;


void MOMOKState::onClick(Momentary *t){
    t->performAction(); // also handles state change
}

void MOMUnsentState::onDataSent(Momentary *t){
    if(t->hasFeedback())
        t->go(unackState);
    else
        t->go(waitState);
}

void MOMUnackState::onNewData(Momentary *t,float v){
    // make sure a matching value was returned
    if(v!=t->getSendValue())
        t->go(badackState);
    else
        t->go(waitState);
}

void MOMUnackState::onClick(Momentary *t){
    // we've clicked an unack - reset the error
    t->go(okState);
}

void MOMBadackState::onClick(Momentary *t){
    // we've clicked a badack - reset the error
    t->go(okState);
}

void MOMWaitState::onEnter(State<Momentary> &prevState, Momentary *t){
    t->startTimer(300);
}
void MOMWaitState::onTimerTick(Momentary *t){
    if(t->waitct++>4){
        t->waitct=0;
        t->stopTimer();
        t->go(okState);
    }
}

void MOMWaitState::onClick(Momentary *t){
    t->performAction();// also handles state change
}



void Momentary::performAction(){
    if(isSpecial){
        doSpecial();
        go(waitState);
    } else if(nudge){
//        printf("snark: momentary got nudge\n");
        nudge->nudge(nudgeType);
        go(waitState);
    } else {
        go(unsentState);
        out->set(outVal);
        if(immediate)
            UDPClient::getInstance()->update();
    }
}

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
    isSpecial=false;
    
    int setWidth=-1,setHeight=-1;
    
    while(!done){
        switch(t->getnext()){
        case T_EXPR: // optional, provides a 'feedback' value
        case T_VAR:
            t->rewind();
            buf = ConfigManager::parseFloatSource();
            break;
        case T_SIZE:
            setWidth = t->getnextint();
            t->getnextcheck(T_COMMA);
            setHeight = t->getnextint();
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
            try {
                nudge = ConfigManager::getNudgeable(buf);
                nudgeType = ConfigManager::parseNudgeType();
            } catch(Exception &e){
                throw Exception(e,t->getline());
            }
            break;}
        case T_CCURLY:
            done=true;
            break;
        case T_KEY:
            t->getnextstring(keyname);
            try {
                getApp()->setKey(keyname,this);
            } catch(Exception& e){
                throw Exception(e,t->getline());
            }
            break;
        case T_SPECIAL:
            isSpecial=true;
            t->getnextstring(special);
            break;
        default:
            throw Exception(t->getline()).set("Unexpected '%s'",t->getstring());
        }
    }
    
    if(!outVar[0] && !nudge && !isSpecial)
        throw Exception("no output name or nudge given for momentary",t->getline());
    
    if(nudge && buf)
        throw Exception("cannot have a nudge momentary with a feedback source",t->getline());
    
    // create a new outvar
    if(!nudge){
        out = new OutValue(outVar,0,always);
        out->listener=this; // so we know when our var was sent
        UDPClient::getInstance()->add(out);
    }
    
    if(title.size()==0){
        if(isSpecial)
            title = QString(special);
        else
            title = QString(outVar);
    }
    
    if(keyname[0]){
        QString foo(keyname);
        foo=foo.toUpper();
        foo.append(": ");
        foo.append(title);
        title = foo;
    }
    
    renderer = buf ? new DataRenderer(this,buf) : NULL;
    
    connect(&timer,SIGNAL(timeout()),this,SLOT(timerTick()));
    
    // initialise the button drawer
    bd.init(this,title,ConfigManager::inverse,&pos);
    bd.waitingCol = Qt::green; // the wait state is just visual feedback,.
    
    // overrides what was done in ButtonDraw::init()
    if(setWidth>0){
        setMinimumSize(setWidth,setHeight);
        setMaximumSize(setWidth,setHeight);
    }
    
    
    machine.start(okState,this);
}

void Momentary::onSend(){
    machine.get().onDataSent(this);
}

void Momentary::timerTick(){
    checkForNewData();
    machine.get().onTimerTick(this);
}


void Momentary::checkForNewData(){
    if(renderer){
        DataBuffer<float> *b = renderer->getBuffer()->getFloatBuffer();
            
        // get most recent datum
        Datum<float> *d = b->read(0);
        if(d) {
            if(d->isRecent()){
                if(d->t > out->timeSent){
                    machine.get().onNewData(this,d->d); 
                }
            } else {
//                go(initState);// stale data, return to init
            }
        }
            
    }
}

void Momentary::onKey(){
    machine.get().onClick(this);
    
}
void Momentary::mousePressEvent(QMouseEvent *event){
    machine.get().onClick(this);
    QWidget::mousePressEvent(event);
}


void Momentary::paintEvent(QPaintEvent *event){
    checkForNewData();
    bd.draw(event,machine.get().getUDPState());
    QWidget::paintEvent(event);
}



/// Special actions for momentaries

void Momentary::doSpecial(){
    printf("Performing %s\n",special);
    if(!strcmp(special,"quit"))
        exit(0);
    else if(!strcmp(special,"resetmaps"))
        getApp()->resetAllMaps();
    
    else if(!strcmp(special,"startlog"))
        getApp()->startLog();
    else if(!strcmp(special,"stoplog"))
        getApp()->stopLog();
    else
        printf("Unknown special: %s\n",special);
}
