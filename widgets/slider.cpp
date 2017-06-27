/**
 * \file
 * Brief description. Longer description.
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#include "slider.h"

#include <QPainter>
#include <QGridLayout>
#include <QSlider>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <math.h>

#include "../config.h"
#include "../exception.h"
#include "../datamgr.h"
#include "../tokens.h"
#include "../doubletime.h"
#include "../udp.h"
#include "sliderstyles.h"

static class SliderInitState : public State<Slider> {
public:
    virtual UDPState getUDPState(){return INIT;}
    virtual void onEnter(State<Slider> &prevState, Slider *t);
    virtual void onNewData(Slider *t,float v);
} initState;

static class SliderOKState : public State<Slider> {
public:
    virtual UDPState getUDPState(){return OK;}
    virtual void onNewData(Slider *t,float v);
    virtual void onClick(Slider *t);
} okState;

static class SliderUnsentState : public State<Slider> {
public:
    virtual UDPState getUDPState(){return UNSENT;}
    virtual void onDataSent(Slider *t);
    virtual void onClick(Slider *t);
} unsentState;

static class SliderDraggingState : public State<Slider> {
    virtual UDPState getUDPState(){return UNSENT;}
    virtual void onSliderRelease(Slider *t);
} draggingState;

static class SliderUnackState : public State<Slider> {
public:
    virtual UDPState getUDPState(){return UNACK;}
    virtual void onNewData(Slider *t,float v);
    virtual void onClick(Slider *t);
} unackState ;

static class SliderBadackState : public State<Slider> {
public:
    virtual UDPState getUDPState(){return BADACK;}
    virtual void onNewData(Slider *t,float v);
    virtual void onClick(Slider *t);
} badackState;


void SliderInitState::onEnter(State<Slider> &prevState, Slider *t){
    if(t->initSet){
        t->set(t->initial);
        t->out->set(t->initial);
    }
    if(!t->hasFeedback())
        t->go(okState);
}
void SliderInitState::onNewData(Slider *t,float v){
    t->set(v);
    t->go(okState);
}


void SliderOKState::onClick(Slider *t){
    t->go(draggingState);
}

void SliderOKState::onNewData(Slider *t,float v){
    t->set(v); // not at all sure about this!
}


void SliderDraggingState::onSliderRelease(Slider *t){
    t->go(unsentState);
    t->queueSend();
}

void SliderUnsentState::onDataSent(Slider *t){
    if(t->hasFeedback())
        t->go(unackState);
    else
        t->go(okState);
}

void SliderUnsentState::onClick(Slider *t){
    // OK, we're going to send *again*
    t->go(draggingState);
}

void SliderUnackState::onNewData(Slider *t,float v){
    float err = v - t->out->val;
    if(t->withinEpsilon(err))
        t->go(okState);
    else
        t->go(badackState);
}

void SliderUnackState::onClick(Slider *t){
    // OK, we're going to send *again*
    t->go(draggingState);
}

void SliderBadackState::onNewData(Slider *t,float v){
    // check to see if the problem somehow resolves itself
    float err = v - t->out->val;
    t->lastReadInBadack=v;
//    t->set(v); /* DON'T DO THIS or your slider will move as soon as you set it! */
    if(t->withinEpsilon(err))
        t->go(okState);
}

void SliderBadackState::onClick(Slider *t){
    // we've clicked a badack - it's fine, just go into dragging again
    t->go(draggingState);
}

          
          

Slider::Slider(QWidget *parent,Tokeniser *t) :
QWidget(NULL)
{
    bool rangeGot = false;
    bool always = false;
    bool vertical = false;
    immediate = false;
    renderer = NULL;
    char outVar[64]; //!< name of the variable to write to
    title[0]=0;
    inverse = ConfigManager::inverse;
    
    ConfigRect pos = ConfigManager::parseRect();
    bool done = false;
    
    t->getnextcheck(T_OCURLY);
    
    initSet=false;
    
    DataBuffer<float> *buf=NULL;
    
    outVar[0]=0;
    epsilon = 0.001f;
    isInteger=false;
    
#if DIAMOND
    const char *topic = NULL;
#endif
    
    while(!done){
        switch(t->getnext()){
        case T_EXPR: // optional, provides a 'feedback' value
        case T_VAR:
            t->rewind();
            buf = ConfigManager::parseFloatSource();
            break;
#if DIAMOND
        case T_DIAMOND:{
            char tname[256];
            t->getnextstring(tname);
            topic = strdup(tname);
            break;
        }
#endif
        case T_EPSILON:
            epsilon = t->getnextfloat();
            break;
        case T_INTEGER:
            isInteger=true;
            break;
        case T_OUT:
            t->getnextident(outVar);
            break;
        case T_INITIAL:
            initial = t->getnextfloat();
            initSet=true;
            break;
        case T_HORIZONTAL: // dummy since it's the default
            break;
        case T_VERTICAL:
            vertical = true;
            break;
        case T_RANGE:
            minVal = t->getnextfloat();
            t->getnextcheck(T_TO);
            maxVal = t->getnextfloat();
            rangeGot=true;
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
            throw Exception(t->getline()).set("Unexpected '%s'",t->getstring());
        }
    }
    
    if(!outVar[0])
        throw Exception("no output name given for slider",t->getline());
    if(!rangeGot)
        throw Exception("no range given for slider",t->getline());
    if(!initSet)initial=minVal;
    
    // create a new outvar
    
    out = new OutValue(outVar,0,always);
#if DIAMOND
    if(topic){
        out->isDiamond=true;
        out->topic=topic;
    }
#endif
        
    out->listener=this; // so we know when our var was sent
    UDPClient::getInstance()->add(out);
    
    if(!title[0])
        strcpy(title,outVar);
    
    renderer = buf ? new DataRenderer(this,buf) : NULL;
    connect(&timer,SIGNAL(timeout()),this,SLOT(timerTick()));
    
    try{
        ConfigManager::registerNudgeable(outVar,this);
    }catch(Exception& e){
        throw Exception(e,t->getline());
    }
    
    QVBoxLayout *layout;
    
    layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    slider = new InternalSlider(vertical?Qt::Vertical:Qt::Horizontal,this);
    slider->setMaximum(100);
    slider->setMinimum(0);
    layout->setAlignment(Qt::AlignCenter);
    label = new QLabel(title);
    label->setAlignment(Qt::AlignCenter);
    label->setMaximumSize(10000,30);
    layout->addWidget(slider);
    layout->addWidget(label);
    setLayout(layout);
    
    slider->setStyleSheet(defaultStyle);
    connect(slider,SIGNAL(sliderPressed()),this,SLOT(pressed()));
    connect(slider,SIGNAL(sliderReleased()),this,SLOT(released()));
    
    QGridLayout *l = (QGridLayout*)parent->layout();
    l->addWidget(this,pos.y,pos.x,pos.h,pos.w);
    setMinimumSize(100,100);
    timer.start(200);
    
    machine.start(initState,this);
    setDrawProperties();
}


void Slider::onSend(){
    machine.get().onDataSent(this);
    update();
}

void Slider::paintEvent(QPaintEvent *event){
    QWidget::paintEvent(event);
}

void Slider::timerTick(){
    
    if(renderer){
        DataBuffer<float> *b = renderer->getBuffer()->getFloatBuffer();
        Datum<float> *d = b->read(0);
        if(d && d->isRecent()){
            if(d->t > out->timeSent){
//                printf("New data : %f. Outvar set to %f\n",d->d,out->val);
                machine.get().onNewData(this,d->d);
            }
        }
    }
}

void InternalSlider::keyPressEvent(QKeyEvent *event){
    event->ignore();
}

void Slider::pressed(){
    machine.get().onClick(this);
}

void Slider::released(){
    machine.get().onSliderRelease(this);
}

void Slider::queueSend(){
    float v = slider->sliderPosition();
    
//    printf("sending slider pos %f ",v);
    
    v -= slider->minimum();
    v /= (slider->maximum()-slider->minimum());
    v *= maxVal-minVal;
    v += minVal;
//    printf("which maps to value %f\n",v);
    out->set(isInteger?floorf(v):v);
    update();
    
    if(immediate)
        UDPClient::getInstance()->update();
}    

void Slider::nudge(NudgeType n){
//    printf("snark: slider attempted nudge\n");
    pressed();
    
    const int step = 10;
    
    int v = slider->sliderPosition();
//    printf("Nudge: value was %d, ",v);
    switch(n){
    case UP:
        //        slider->triggerAction(QAbstractSlider::SliderSingleStepAdd);
        v+=step;
        if(v>slider->maximum())v=slider->maximum();
        slider->setSliderPosition(v);
//        printf("value is now %d\n",slider->sliderPosition());
        break;
    case DOWN:
//        slider->triggerAction(QAbstractSlider::SliderSingleStepSub);
        v-=step;
        if(v<slider->minimum())v=slider->minimum();
        slider->setSliderPosition(v);
//        printf("value is now %d\n",slider->sliderPosition());
        break;
    case MIN:
        slider->setSliderPosition(slider->minimum());
        break;
    case MAX:
        slider->setSliderPosition(slider->maximum());
        break;
    case CENTRE:
        slider->setSliderPosition((slider->maximum()+slider->minimum())/2);
        break;
    }
    released();
    slider->update();
}

void Slider::setDrawProperties(){
    switch(machine.get().getUDPState()){
    case INIT:
        slider->setStyleSheet(initStyle);
        break;
    case UNSENT:
        slider->setStyleSheet(unsentStyle);
        break;
    case UNACK:
        slider->setStyleSheet(unackStyle);
        break;
    case BADACK:
        slider->setStyleSheet(badAckStyle);
        break;
    case WAITING:
    case OK:
        slider->setStyleSheet(defaultStyle);
        break;
    }
    
    setEnabled(machine.get().getUDPState()!=INIT);
    
    printf("slider state %d\n",machine.get().getUDPState());
}
