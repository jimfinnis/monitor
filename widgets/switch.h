/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#ifndef __SWITCH_H
#define __SWITCH_H

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <math.h>

#include "datarenderer.h"
#include "tokeniser.h"
#include "udp.h"
#include "buttondraw.h"
#include "keyhandler.h"
#include "state.h"


/// the internal widget which actually does the toggle switch

class SwitchInternal : public QWidget {
    Q_OBJECT

public:
    explicit SwitchInternal(class Switch *g) : QWidget((QWidget *)g){
        sw = g;
    }
public slots:
    void dataChanged(){
        update();
    }
    
    

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
private:
    class Switch *sw;

};

/// a toggle switch

class Switch : public QWidget,  public UDPClientSendListener, KeyHandler {
    Q_OBJECT
          
public:
    virtual ~Switch(){
        if(renderer)
            delete renderer;
    }
    
    /// true if we have a feedback variable
    bool hasFeedback(){
        return renderer ? true : false;
    }
    /// add our value to be sent by the UDP server
    virtual void sendData();
    
    /// for UDPClientSendListener
    virtual void onSend();
    
    /// the painter, actually called by an internal widget (which needs to be passed in,
    /// as it's that which is redrawn). Only applies to toggle switches,
    /// button-style switches use the paintEvent().
    
    virtual void handlePaint(QPaintEvent *p,SwitchInternal *w);
    
    /// only used for button-style
    virtual void paintEvent(QPaintEvent *e);
    
    /// parse the configuration details for a Switch and construct it
    Switch(QWidget *parent,Tokeniser *t);
    
    /// toggle the switch, update the time changed and send the data.
    virtual void onKey();
    /// only applies for button-style, toggle-style uses the one
    /// in SwitchInternal
    virtual void mousePressEvent(QMouseEvent *event);
    
    bool value; //!< the value of the toggle
    
    /// if true, clicking the button doesn't toggle - it sends setVal, and the state of the button
    /// reflects whether the feedback variable = setVal. There must be a feedback.
    bool hasSetVal;
    float setVal;
    
    QColor onCol,offCol;

    /// shorthand for machine.go(), which updates the widget too
    virtual void go(State<Switch>& s){
        machine.go(s);
        update();
    }
    
    /// used for setVal switches to see if the server value matches
    bool matches(float v){
        v = fabsf(v-setVal);
        return v<0.01f;
    }
    
    float lastData; //!< last datum received
    
public slots:
    /// we need this if the switch is button-style.
    void dataChanged(){
        update();
    }
private:
    bool isButton;
    /// used if we are a button-style
    ButtonDrawer bd;
    
    /// state machine, controlled by go()
    StateMachine<Switch> machine;
    
    /// check to see if there's new data - newer than the last time data was
    /// sent by this switch. If so, call the current state's  onNewData(). Only
    /// does this if there is a feedback variable.
    void checkForNewData();
    
    // these are used if we are switch-style
    
    QVBoxLayout *layout; //!< the layout
    QWidget *main; //!< the main widget
    QLabel *label; //!< the label containing varname or title
    
    // used for both.
    
    OutValue *out; //!< the output value
    void setButtonDrawCol();
    
    /// our link to the variable - optional, for when the switch has
    /// feedback.
    DataRenderer *renderer;
    bool immediate; //!< if true, sent data immediately, else wait for next interval tick.
    
};





#endif /* __SWITCH_H */
