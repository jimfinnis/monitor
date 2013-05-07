/**
 * \file
 * Brief description. Longer description.
 *
 * 
 * \author $Author$
 * \date $Date$
 */


#include "window.h"
#include "app.h"
#include <QKeyEvent>
#include <QTimer>

Window::Window() : QMainWindow() {
}

Window::~Window(){
}


void Window::keyPressEvent(QKeyEvent *event){
    if(!getApp()->keyPress(event->key()))
        QMainWindow::keyPressEvent(event);
    
}

