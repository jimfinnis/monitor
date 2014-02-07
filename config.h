/**
 * \file
 * Handles the configuration file, creating widgets and renderers
 * appropriately.
 */


#ifndef __CONFIG_H
#define __CONFIG_H

#include <QColor>
#include <QString>
#include "datamgr.h"
#include "nudgeable.h"
#include "defaults.h"

/// produced by ConfigManager::parseRect() to get widget positions etc.
struct ConfigRect {
    int x,y,w,h;
    int minsizex,minsizey;
    ConfigRect(){
        x=-1;// undefined
        minsizex=60; // default minimum size
        minsizey=60; // default minimum size
    }
    bool isset(){
        return x>=0;
    }
};

/// this is yet another static class, for handling the configuration file
/// and creating buffers, widgets and renderers. It uses a recursive
/// descent parser, not written in flex/yacc because integrating that with
/// any C++ program, let alone one using qmake, can be hairy.

class ConfigManager {
public:
    /// parse the configuration file given, and produce the
    /// widgets, buffers, renderers etc. described therein.
    static void parseFile(QString fname);
    
    /// parse a location rectangle - either just x,y with implied width and
    /// height of 1,1, or a full x,y,w,h.
    static ConfigRect parseRect();
    
    /// parse a nudge type
    static NudgeType parseNudgeType();
    
    /// parse a float variable or expression: it's one of:
    /// -  var foo
    /// -  expr "foo*4+bar" range auto
    /// -  expr "foo*4+bar" range 0 to 100
    /// Returns the buffer associated with the variable or expression.
    static DataBuffer<float> *parseFloatSource();
    
    /// parse a colour specification - either a colour name,
    /// a string with a "#rgb#" spec, or "default" which returns
    /// the colour passed in.
    
    static QColor parseColour(QColor deflt);
    
    /// the RECEIVEport as given in the config file, or -1 for the default
    
    static int port;
    
    /// the port that switches etc. send to (33333 by default)
    static int udpSendPort;
    /// the host that switches etc. send to (localhost by default)
    static char udpSendAddr[];
    
    /// interval between sends of "always send" items
    static float sendInterval;
    
    /// interval between successive mandatory graphical updates (ms)
    static int graphicalUpdateInterval;
    
    /// if this is true, widgets and styles are black on white
    static bool inverse;
    
    /// set the stylesheet for a widget given whether we are inverse
    /// or not.
    static void setStyle(QWidget *w);
    
    /// find a widget by name, so we can reference it for nudging etc.
    static Nudgeable *getNudgeable(const char *name);
    
    static void registerNudgeable(const char *name,Nudgeable *n);
};


#endif /* __CONFIG_H */
