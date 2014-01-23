/**
 * \file
 * state system for switches and buttons
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __STATE_H
#define __STATE_H


/// this defines a simple state machine for switches and buttons
template <class T> class State {
public:
    virtual void onEnter(State<T> &prevState, T *t){}
    virtual void onExit(State<T> &nextState, T *t){}
    virtual void onNewData(T *t,float v){}
    virtual void onClick(T *t){} // clicking does nothing by default
    virtual void onSliderRelease(T *t){} // releasing does nothing by default (and only applies to sliders)
    virtual void onDataSent(T *t){}
    virtual void onTimerTick(T *t){} // used in wait states
    
    /// get the "UDP state" which is used for rendering
    virtual UDPState getUDPState()=0;
    
};


template <class T> class StateMachine {
private:
    State<T> *cur;
    T *target;
public:
    /// we don't do this with a ctor because we don't want
    /// the onEntry for the start state to run in static initialisation.
    void start(State<T> &init, T *t){
        target = t;
        cur = &init;
        cur->onEnter(*cur,t); // special case
    }
    
    virtual void go(State<T> &s){
        cur->onExit(s,target);
        s.onEnter(*cur,target);
        cur=&s;
    }
    
    State<T> &get(){
        return *cur;
    }
};


#endif /* __STATE_H */
