/**
 * \file
 * New version of momentary switch.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __MOMENTARY_H
#define __MOMENTARY_H

#include <QWidget>
#include <QTimer>


#include "keyhandler.h"
#include "datarenderer.h"
#include "tokeniser.h"
#include "udp.h"
#include "nudgeable.h"
#include "buttondraw.h"
#include "state.h"



class Momentary : public QWidget,UDPClientSendListener,KeyHandler {
    Q_OBJECT    

public:
    
    explicit Momentary(QWidget *parent,Tokeniser *t);
    virtual ~Momentary(){
        if(renderer)delete renderer;
    }

    virtual void onSend();
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    void press();

    /// shorthand for machine.go(), which updates the widget too
    virtual void go(State<Momentary>& s){
        machine.go(s);
        update();
    }
    virtual bool hasFeedback(){
        return renderer ? true : false;
    }
    
    virtual void startTimer(int time){
        waitct=0;
        timer.start(time);
    }
    virtual void stopTimer(){
        timer.stop();
    }
    virtual float getSendValue(){
        return outVal;
    }
    
    virtual void performAction();
    int waitct;

public slots:
    void dataChanged(){
        update();
    }
    void timerTick();
    
    
    
private:
    StateMachine<Momentary> machine;
    
    QString title;
    ButtonDrawer bd;
    bool isSpecial; //!< indicates this is not a normal button or a nudge, but a special action
    char special[256]; //!< and this is the special action string if isSpecial is true
    
    /// the value we change when the switch is clicked; may be NULL
    /// if we're a nudger.
    OutValue *out;
    /// if we're a nudger, this is what we should nudge
    Nudgeable *nudge;
    /// if we're a nudger, this is what we should do
    NudgeType nudgeType;
    
    /// the value to output
    float outVal;
    /// optional link to feedback variable
    DataRenderer *renderer;
    /// timer used to update state until it's OK
    QTimer timer;
    /// if true, send changes immediately
    bool immediate;
    
    /// special action processing - add to this for more specials
    void doSpecial();
    
    virtual void onKey();
    void checkForNewData();
};


#endif /* __MOMENTARY_H */
