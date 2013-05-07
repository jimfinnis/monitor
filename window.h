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

class Window : public QMainWindow {
    Q_OBJECT
public:          
    Window();
    virtual ~Window();
    virtual void keyPressEvent(QKeyEvent *event);
};


#endif /* __WINDOW_H */
