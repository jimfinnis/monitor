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
public slots:
    void timerTick();
private:
    QString title;
    int waitct;
    UDPState state;
    ButtonDrawer bd;
    
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
    
    /// check for state transitions
    void stateCheck();
    
    virtual void onKey();
};


#endif /* __MOMENTARY_H */
