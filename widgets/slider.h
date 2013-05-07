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
#include <QTimer>

#include "datarenderer.h"
#include "tokeniser.h"
#include "udp.h"
#include "nudgeable.h"



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
public slots:
    void timerTick();
    void changed(int v);
    void released();
    void dataChanged(){
        update();
    }
private:
    bool inverse;
    char title[64];
    int waitct;
    UDPState state;
    class QLabel *label;
    class QSlider *slider;
    
    /// the value we change
    OutValue *out;
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
    /// check for state transitions
    void stateCheck();
    /// change state, and make the on-screen representation match the state
    void setState(UDPState st);
    
    /// receive and handle a nudge
    virtual void nudge(NudgeType n);
};

              

#endif /* __SLIDER_H */
