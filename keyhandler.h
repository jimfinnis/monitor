/**
 * \file
 * Brief description. Longer description.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __KEYHANDLER_H
#define __KEYHANDLER_H

/// interface providing something for widgets to implement when they
/// want to handle a key click
class KeyHandler {
public:
    virtual void onKey()=0;
};

#endif /* __KEYHANDLER_H */
