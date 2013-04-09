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

#include "datarenderer.h"
#include "tokeniser.h"
#include "udp.h"

/// the internal widget which actually does the toggle switch

class SwitchInternal : public QWidget {
    Q_OBJECT

public:
    explicit SwitchInternal(class Switch *g) : QWidget((QWidget *)g){
        sw = g;
    }
    

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
private:
    class Switch *sw;

};

/// a toggle switch

class Switch : public QWidget, public UDPClientSendListener {
    Q_OBJECT
          
public:
    virtual ~Switch(){
        if(renderer)
            delete renderer;
    }
    
    virtual void onSend(){
        update();
    }

    /// the painter, actually called by an internal widget (which needs to be passed in,
    /// as it's that which is redrawn)
    virtual void handlePaint(QPaintEvent *p,SwitchInternal *w);
    
    
    /// parse the configuration details for a Switch and construct it
    Switch(QWidget *parent,Tokeniser *t);
    
    /// toggle the switch, update the time changed and send the data.
    void toggle();

private:
    QVBoxLayout *layout; //!< the layout
    QWidget *main; //!< the main widget
    QLabel *label; //!< the label containing varname or title
    
    bool value; //!< the value of the toggle
    OutValue *out; //!< the output value
    bool immediate; //!< if true, sent data immediately, else wait for next interval tick.
    
    /// our link to the variable - optional, for when the switch has
    /// feedback.
    DataRenderer *renderer;
};

#endif /* __SWITCH_H */
