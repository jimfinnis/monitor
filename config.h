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

/// produced by ConfigManager::parseRect() to get widget positions etc.
struct ConfigRect {
    int x,y,w,h;
    ConfigRect(){
        x=-1;// undefined
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
};


#endif /* __CONFIG_H */
