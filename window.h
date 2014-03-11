/**
 * \file
 * Brief description. Longer description.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __WINDOW_H
#define __WINDOW_H

#include <QMainWindow>

/// this is now a slightly pointless class, since all
/// the key stuff was moved out
class Window : public QMainWindow {
    Q_OBJECT
public:          
    Window();
    virtual ~Window();
};


#endif /* __WINDOW_H */
