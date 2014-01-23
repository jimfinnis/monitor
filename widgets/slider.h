/**
 * \file
 * Brief description. Longer description.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __SLIDER_H
#define __SLIDER_H

#include <QWidget>
#include <QSlider>
#include <QTimer>

#include "datarenderer.h"
#include "tokeniser.h"
#include "udp.h"
#include "nudgeable.h"
#include "state.h"

/// this only exists to override the key press events - the event handler just calls
/// ignore.
class InternalSlider : public QSlider {
public:
    explicit InternalSlider(Qt::Orientation orientation,QWidget *parent) : QSlider(orientation,parent){
    }
    virtual void keyPressEvent(QKeyEvent *event);
};

class Slider : public QWidget, UDPClientSendListener, Nudgeable {
    Q_OBJECT
          
public:
    explicit Slider(QWidget *parent, Tokeniser *t);
    virtual ~Slider(){
        if(renderer)
            delete renderer;
    }
    
    virtual void onSend();
    virtual void paintEvent(QPaintEvent *event);
    
    virtual void go(State<Slider>& s){
        machine.go(s);
        setDrawProperties();
        update();
    }
    
    void set(float v){
        v -= minVal;
        v /= (maxVal-minVal);
        v *= (slider->maximum()-slider->minimum());
        v += slider->minimum();
        slider->setSliderPosition(v);
        slider->update();
    }
    
    void queueSend();
    
    bool initSet; //!< should we set an initial value
    float initial; //!< the initial value we should set
    
    bool hasFeedback(){
        return renderer ? true : false;
    }
    
    bool withinEpsilon(float e){
        if(e<0)e=-e;
        return e<=epsilon;
    }
    /// the value we change
    OutValue *out;
    float lastReadInBadack;
    bool isInteger; //!< true if we should send as an integer

public slots:
    void timerTick();
    void pressed();
    void released();
    void dataChanged(){
        update();
    }

private:
    StateMachine<Slider> machine;
    
    void setDrawProperties();

    bool inverse;
    
    char title[64];
    UDPState state;
    class QLabel *label;
    class InternalSlider *slider;
    
    
    /// the range we map to
    float minVal,maxVal;
    /// when we receive an ack packet, it must match the value
    /// sent to within this error value.
    float epsilon;
    /// optional link to feedback variable
    DataRenderer *renderer;
    /// timer used to update state until it's OK
    QTimer timer;
    /// if true, send changes immediately
    bool immediate;
    
    /// receive and handle a nudge
    virtual void nudge(NudgeType n);
};

              

#endif /* __SLIDER_H */
