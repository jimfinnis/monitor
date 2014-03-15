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
#include "../app.h"





static class SWIInitState : public State<Switch> {
public:
    virtual UDPState getUDPState(){return INIT;}
    virtual void onEnter(State<Switch> &prevState, Switch *t);
    virtual void onNewData(Switch *t,float v);
} initState;

static class SWIOKState : public State<Switch> {
public:
    virtual UDPState getUDPState(){return OK;}
    virtual void onNewData(Switch *t,float v);
    virtual void onClick(Switch *t);
} okState;

static class SWIUnsentState : public State<Switch> {
public:
    virtual UDPState getUDPState(){return UNSENT;}
    virtual void onDataSent(Switch *t);
} unsentState;


static class SWIUnackState : public State<Switch> {
public:
    virtual UDPState getUDPState(){return UNACK;}
    virtual void onNewData(Switch *t,float v);
    virtual void onClick(Switch *t);
} unackState ;

static class SWIBadackState : public State<Switch> {
public:
    virtual UDPState getUDPState(){return BADACK;}
    virtual void onNewData(Switch *t,float v);
    virtual void onClick(Switch *t);
} badackState;


void SWIInitState::onEnter(State<Switch> &prevState, Switch *t){
    if(!t->hasFeedback()){
        t->value = false;
        t->go(okState);
    }
}

void SWIInitState::onNewData(Switch *t,float v){
    t->value = (v>0.5f);
    t->go(okState);
}

void SWIOKState::onClick(Switch *t){
    if(t->hasSetVal)
        t->value = true;
    else
        t->value = !t->value;
    
    t->go(unsentState);
    t->sendData();
}

void SWIOKState::onNewData(Switch *t,float v){
    if(t->hasSetVal)
        t->value = t->matches(v); // does the value match OUR value for a setVal switch?
    else
        t->value = (v>0.5f); // make sure the switch shows what the server is doing
    
    // this will cause continuous updates of the switch, and so fuck
    // up the dialog. 
//    t->go(okState);
}

void SWIUnsentState::onDataSent(Switch *t){
    if(t->hasFeedback())
        t->go(unackState);
    else
        t->go(okState);
}

void SWIUnackState::onNewData(Switch *t,float v){
    bool ok;
    if(t->hasSetVal){
        ok = t->matches(v);
    } else {
        bool q = v>0.5f;
        ok = q==t->value;
    }
    if(ok)
        t->go(okState);
    else
        t->go(badackState);
}
void SWIUnackState::onClick(Switch *t){
    // we've clicked unack - reset the error, setting the
    // switch to the value read from the server
    if(t->hasSetVal)
        t->value = t->matches(t->lastData);
    else
        t->value = !t->value;
    t->go(okState);
}

void SWIBadackState::onNewData(Switch *t,float v){
    // check to see if the problem somehow resolves itself
    bool ok;
    if(t->hasSetVal){
        ok = t->matches(v);
    } else {
        bool q = v>0.5f;
        ok = q==t->value;
    }
    if(ok)
        t->go(okState);
}
void SWIBadackState::onClick(Switch *t){
    // we've clicked a badack - reset the error, setting the
    // switch to the value read from the server
    if(t->hasSetVal)
        t->value = t->matches(t->lastData);
    else
        t->value = !t->value;
    t->go(okState);
}







void Switch::onSend(){
    machine.get().onDataSent(this);
}


Switch::Switch(QWidget *parent,Tokeniser *t) : QWidget(NULL) {
    bool always=false;
    value = 0;
    renderer = NULL;
    immediate =false;
    char outVar[64]; //!< name of the variable to write to
    char keyname[64];
    QString title("");
    keyname[0]=0;
    int setWidth=-1,setHeight=-1;
    
    isButton=false;
    onCol = Qt::green;
    offCol = Qt::red;
    
    ConfigRect pos = ConfigManager::parseRect();
    
    bool done = false;
    
    t->getnextcheck(T_OCURLY);
    
    DataBuffer<float> *buf=NULL;
    
    outVar[0]=0;
    hasSetVal=false;
    
    while(!done){
        switch(t->getnext()){
        case T_EXPR: // optional, provides a 'feedback' value
        case T_VAR:
            t->rewind();
            buf = ConfigManager::parseFloatSource();
            break;
        case T_SET:
            setVal = t->getnextfloat();
            hasSetVal = true;
            break;
        case T_OUT:
            t->getnextident(outVar);
            break;
        case T_TITLE:{
            char buf[256];
            t->getnextstring(buf);
            title = QString(buf);
            break;
        }
        case T_SIZE:
            setWidth = t->getnextint();
            t->getnextcheck(T_COMMA);
            setHeight = t->getnextint();
            break;
        case T_ALWAYS:
            always=true;
            break;
        case T_BUTTON:
            isButton = true;
            break;
        case T_IMMEDIATE:
            immediate=true;
            break;
        case T_KEY:
            t->getnextstring(keyname);
            getApp()->setKey(keyname,this);
            break;
        case T_COLOURS:
            onCol = ConfigManager::parseColour(Qt::green);
            t->getnextcheck(T_COMMA);
            offCol = ConfigManager::parseColour(Qt::red);
            break;
        case T_CCURLY:
            done=true;
            break;
        default:
            throw ParseException(t).set("Unexpected '%s'",t->getstring());
        }
    }
    if(!outVar[0])
        throw ParseException(t,"no output name given for switch");
    
    // create a new outvar
    
    out = new OutValue(outVar,0,always);
    out->listener=this;
    UDPClient::getInstance()->add(out);
    
    if(!title.size())
        title = QString(outVar);
    
    if(keyname[0]){
        QString foo(keyname);
        foo=foo.toUpper();
        foo.append(": ");
        foo.append(title);
        title = foo;
    }
    
    if(isButton){
        bd.init(this,title,ConfigManager::inverse,&pos);
        bd.waitingCol = Qt::gray;
        main = this; // so the renderer connects up correctly
        
        // overrides what was done in ButtonDraw::init()
        if(setWidth>0){
            setMinimumSize(setWidth,setHeight);
            setMaximumSize(setWidth,setHeight);
        }
    }else{
        if(setWidth>0)
            throw ParseException(t,"cannot set absolute size for a toggle-like switch");
        
        layout = new QVBoxLayout(this);
        layout->setSpacing(0);
        main = new SwitchInternal(this);
        label = new QLabel(title);
        label->setAlignment(Qt::AlignCenter);
        label->setMaximumSize(10000,30);
    
        if(pos.minsizex < 100)pos.minsizex=100;
        if(pos.minsizey < 100)pos.minsizey=100;
    
        setMinimumSize(pos.minsizex,pos.minsizey);
        layout->addWidget(main);
        layout->addWidget(label);
        setLayout(layout);
        QGridLayout *l = (QGridLayout*)parent->layout();
        l->addWidget(this,pos.y,pos.x,pos.h,pos.w);
    }
    QGridLayout *l = (QGridLayout*)parent->layout();
    l->addWidget(this,pos.y,pos.x,pos.h,pos.w);
    
    renderer = buf ? new DataRenderer(main,buf) : NULL;
    
    if(hasSetVal && !hasFeedback())
        throw ParseException(t,"a switch with `set' must have a feedback source");
    
    value = false;
    
    /// start the state machine
    machine.start(initState,this);
    
    
}

void SwitchInternal::mousePressEvent(QMouseEvent *event){
    sw->onKey();
    QWidget::mousePressEvent(event);
}

void Switch::mousePressEvent(QMouseEvent *event){
    if(isButton){
        onKey();
        QWidget::mousePressEvent(event);
    }
}


void SwitchInternal::paintEvent(QPaintEvent *event){
    sw->handlePaint(event,this);
    QWidget::paintEvent(event);
}


void Switch::checkForNewData(){
    if(renderer){
        DataBuffer<float> *b = renderer->getBuffer()->getFloatBuffer();
            
        // get most recent datum
        Datum<float> *d = b->read(0);
        if(d && d->isRecent()){
            if(d->t > out->timeSent){
                // a value has been received
                lastData=d->d;
                machine.get().onNewData(this,d->d);
            }
        } else {
            // data is nonexistent or state, head to the INIT
            // state.
            if(machine.get().getUDPState()!=INIT)
                go(initState);
        }
    }
}

void Switch::paintEvent(QPaintEvent *e){
    if(isButton){
        checkForNewData();
        setButtonDrawCol();
        bd.draw(e,machine.get().getUDPState());
    }
}

void Switch::handlePaint(UNUSED QPaintEvent *p,SwitchInternal *widget){
    checkForNewData();
    
    int w = widget->width();
    int h = widget->height();
    
    QPainter painter(widget);
    painter.setRenderHint(QPainter::Antialiasing);
    
    
    QColor col = value ? onCol : offCol;
    
    float switchWidth = h*0.5;
    if(switchWidth>w)switchWidth=w;
    QPen pen(col);
    pen.setWidth(5);
    
    painter.setPen(pen);
    
    QBrush brush(Qt::SolidPattern);
    switch(machine.get().getUDPState()){
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
    case OK:
        brush.setColor(col);
        break;
    case WAITING:
        break;
    }
    
    int yoffset=0;
    if(!value)
        yoffset=h/2;
    
    painter.fillRect(w/2-switchWidth/2,0,switchWidth,h,QBrush(Qt::black));
    painter.fillRect(w/2-switchWidth/2,yoffset,switchWidth,h/2,brush);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(w/2-switchWidth/2,0,switchWidth,h);
}

void Switch::setButtonDrawCol(){
    if(isButton)
        bd.OKCol = value ? onCol : offCol;
    
}

void Switch::onKey(){
    // state notification and perhaps change
    machine.get().onClick(this);
}

        
void Switch::sendData(){
    if(hasSetVal)
          out->set(setVal);
    else
          out->set(value?1:0);
    if(immediate)
        UDPClient::getInstance()->update();
}
