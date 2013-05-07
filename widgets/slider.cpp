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
#include <QLabel>

#include "../config.h"
#include "../exception.h"
#include "../datamgr.h"
#include "../tokens.h"
#include "../doubletime.h"
#include "../udp.h"

static const char *defaultStyle=
" QSlider::groove:horizontal {border: 1px solid #999999; height: 8px; margin: 2px 0;}\
  QSlider::handle:horizontal {background: #ffffff;border: 1px solid #5c5c5c; width: 18px; height: 18px; margin: -10px 0;  border-radius: 3px;}\
  QSlider::groove:vertical {border: 1px solid #999999; width: 8px; margin: 0 2px;}\
  QSlider::handle:vertical {background: #ffffff;border: 1px solid #5c5c5c; height: 18px; width: 18px; margin: 0 -10px;  border-radius: 3px;}\
";
static const char *unsentStyle=
" QSlider::groove:horizontal {border: 1px solid #999999; height: 8px; margin: 2px 0;}\
  QSlider::handle:horizontal {background: #808080;border: 1px solid #5c5c5c; width: 18px; height: 18px; margin: -10px 0;  border-radius: 3px;}\
  QSlider::groove:vertical {border: 1px solid #999999; width: 8px; margin: 0 2px;}\
  QSlider::handle:vertical {background: #808080;border: 1px solid #5c5c5c; height: 18px; width: 18px; margin: 0 -10px;  border-radius: 3px;}\
";
static const char *unackStyle=
" QSlider::groove:horizontal {border: 1px solid #999999; height: 8px; margin: 2px 0;}\
  QSlider::handle {background: url(:images/diag.png);border: 1px solid #5c5c5c; width: 18px; height: 18px; margin: -10px 0;  border-radius: 3px;}\
  QSlider::groove:vertical {border: 1px solid #999999; width: 8px; margin: 0 2px;}\
  QSlider::handle:vertical {background: url(:images/diag.png);border: 1px solid #5c5c5c; width: 18px; height: 18px; margin: 0 -10px;  border-radius: 3px;}\
";
static const char *badAckStyle=
" QSlider::groove:horizontal {border: 1px solid #999999; height: 8px; margin: 2px 0;}\
  QSlider::handle {background: url(:images/cross.png);border: 1px solid #5c5c5c; width: 18px; margin: -10px 0;  border-radius: 3px;}\
  QSlider::groove:vertical {border: 1px solid #999999; width: 8px; margin: 0 2px;}\
  QSlider::handle:vertical {background: url(:images/cross/png);border: 1px solid #5c5c5c; width: 18px; height: 18px; margin: 0 -10px;  border-radius: 3px;}\
";
          
          

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
    
    bool initSet=false;
    float initial;
    
    DataBuffer<float> *buf=NULL;
    
    outVar[0]=0;
    epsilon = 0.001f;
    while(!done){
        switch(t->getnext()){
        case T_EXPR: // optional, provides a 'feedback' value
        case T_VAR:
            t->rewind();
            buf = ConfigManager::parseFloatSource();
            break;
        case T_EPSILON:
            epsilon = t->getnextfloat();
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
            throw Exception().set("Unexpected '%s'",t->getstring());
        }
    }
    
    if(!outVar[0])
        throw Exception("no output name given for slider");
    if(!rangeGot)
        throw Exception("no range given for slider");
    if(!initSet)initial=minVal;
    
    // create a new outvar
    
    out = new OutValue(outVar,0,always);
    out->listener=this; // so we know when our var was sent
    UDPClient::getInstance()->add(out);
    
    if(!title[0])
        strcpy(title,outVar);
    
    renderer = buf ? new DataRenderer(this,buf) : NULL;
    connect(&timer,SIGNAL(timeout()),this,SLOT(timerTick()));
    
    ConfigManager::registerNudgeable(outVar,this);
    
    QVBoxLayout *layout;
    
    layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    slider = new QSlider(vertical?Qt::Vertical:Qt::Horizontal,this);
    layout->setAlignment(Qt::AlignCenter);
    label = new QLabel(title);
    label->setAlignment(Qt::AlignCenter);
    label->setMaximumSize(10000,30);
    layout->addWidget(slider);
    layout->addWidget(label);
    setLayout(layout);
    
    slider->setStyleSheet(defaultStyle);
    connect(slider,SIGNAL(valueChanged(int)),this,SLOT(changed(int)));
    connect(slider,SIGNAL(sliderReleased()),this,SLOT(released()));
    
    // send initial value
    out->set(initial);
    
    // set slider position
    initial = initial-minVal;
    initial = initial/(maxVal-minVal);
    initial *= (slider->maximum()-slider->minimum());
    initial += slider->minimum();
    slider->setSliderPosition((int)initial);
    
    
    QGridLayout *l = (QGridLayout*)parent->layout();
    l->addWidget(this,pos.y,pos.x,pos.h,pos.w);
    setMinimumSize(100,100);
    setState(UNSENT);
    timer.start(200);
}


void Slider::onSend(){
    if(renderer)
        setState(UNACK);
    else
        setState(OK);
    update();
}

void Slider::paintEvent(QPaintEvent *event){
    QWidget::paintEvent(event);
}

void Slider::timerTick(){
    DataBuffer<float> *b;
    Datum<float> *d;
    switch(state){
    case UNSENT:
        break;
    case BADACK:// keep waiting - maybe a good ack will turn up
    case UNACK:
        b = renderer->getBuffer()->getFloatBuffer();
        // here, the value we check against is the value we sent.
        d = b->read(0);
        // see if the variable we're watching matches the slider value
        if(d && d->isRecent()){
            float err = d->d - out->val;
            if(err<0)err=-err;
            if(err < epsilon) // it's match
                setState(OK);
            else
                setState(BADACK);
        }
        break;
    case WAITING:
    case OK:
        timer.stop();
        break;
    }
}

void Slider::setState(UDPState st){
    state = st;
    switch(state){
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
}

void Slider::changed(int iv){
    float v = iv;
    // get to 0-1
    v /= (slider->maximum()-slider->minimum());
    v -= slider->minimum();
    // remap to output
    v *= maxVal-minVal;
    v += minVal;
    out->set(v);
    setState(UNSENT);
    timer.start(200);
}

void Slider::released(){
    if(immediate)
        UDPClient::getInstance()->update();
        
}

void Slider::nudge(NudgeType n){
    switch(n){
    case UP:
        slider->triggerAction(QAbstractSlider::SliderSingleStepAdd);
        break;
    case DOWN:
        slider->triggerAction(QAbstractSlider::SliderSingleStepSub);
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
    slider->update();
}
