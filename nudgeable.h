/**
 * \file
 * Brief description. Longer description.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __NUDGEABLE_H
#define __NUDGEABLE_H

enum NudgeType {
    UP,MIN,MAX,DOWN,CENTRE,
};

/// implemented by widgets which can be 'nudged' by momentary buttons,
/// like sliders and dials.

class Nudgeable {
public:
    virtual void nudge(NudgeType n)=0;
};

#endif /* __NUDGEABLE_H */
