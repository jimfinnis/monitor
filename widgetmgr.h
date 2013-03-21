/**
 * \file
 * The widget manager handles a set of named frames (set up in the Qt code) each of which
 * contains data widgets (set up by the widget manager, from the configuration file.)
 */

/// A collection of frames set up in Qt, each of which is a collection of widgets which
/// we create dynamically - and which have data routed to them.

class WidgetManager {
public:
    /// add a frame (or some other kind of container) as a widget container, with a name. Also creates a grid
    /// layout for the frame.
    static void addFrame(const char *name,class QWidget *frame);
    
    /// add a widget to a frame, adding it to the layout in the given row and column.
    static void addWidget(const char *frameName, class QWidget *wid,int x, int y,int w=1,int h=1);
    
    /// tell all the widgets to redraw
    static void updateAll();
};


#ifndef __WIDGETMGR_H
#define __WIDGETMGR_H



#endif /* __WIDGETMGR_H */
