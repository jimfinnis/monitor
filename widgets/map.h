/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#ifndef __MAP_H
#define __MAP_H

#include <QWidget>
#include <QString>
#include <QHash>
#include <QList>
#include <marble/MarbleWidget.h>

#include "datarenderer.h"
#include "tokeniser.h"

class MapWidget;

/// interface for classes which render something on the map.
class MapItemRenderer {
public:
    DataBuffer<float> *latBuf; //!< the latitude buffer
    DataBuffer<float> *longBuf; //!< the longitude buffer
    
    MapItemRenderer(MapWidget *w){
        latBuf=NULL;
        longBuf=NULL;
        onVar=NULL;
        widget=w;
    }
    
    MapWidget *widget; //!< the widget we're tied to
    
    /// this is a bit complicated - the widget will be redrawn every
    /// time this value changes, and also the time values from this variable
    /// will be used to produce points. For example, if the data buffer for
    /// this is "lat" or "long", we'll get points every time the GPS reports
    /// a new position; but if it's some other variable we'll get points
    /// for every new reading of that, with the positions interpolated.
    DataRenderer *onVar;
    
    /// parse the location (assumes we've just parsed T_LOCATION)
    void parseLocation(Tokeniser *t);
    
    /// this does the actual rendering
    virtual void render(Marble::GeoPainter *painter)=0;
};

/// used for getting colour data

struct MapColour {
    MapColour(){
        color = Qt::white;
        hueBuf = NULL;
        satBuf = NULL;
        valBuf = NULL;
    }
    
    QColor color; //!< base colour, white by default
    DataBuffer<float> *hueBuf; //!< hue is 0 if null
    DataBuffer<float> *satBuf; //!< sat is 0 if null and hue and val are both null, otherwise the default is 1.
    DataBuffer<float> *valBuf; //!< val is  1 if null
    
    QColor get(double t); //!< get the colour at time t
};

/// used for getting some kind of mapped float range data

struct MapFloat {
    MapFloat(){
        base = 10;
        factor = 0;
        buf = NULL;
    }
                            
    float base; //!< base value...
    float factor; //!< ...to which is added factor*variable
    
    DataBuffer<float> *buf; //!< the variable we're looking at, which could be NULL in which case base is returned
    
    float get(double t); //!< get the value at time t
};
    

/// this renderer is the default for rendering points - it's still
/// quite flexible.
/// How colour is mapped:
/// - if valBuf exists a 0-1 value is extracted from that buffer, else 1 is used
/// - if hueBuf exists, a hue is extracted from it, else 0 is used
/// - if satBuf exists, a saturation is extracted else 1 is used
/// - the HSV colour is generated, and multiplied with "color", which is white by default
class MapItemPointRenderer : public MapItemRenderer {
    
    int trailSize; //!< how many historic items to draw (zero by default)
    MapColour col;
    MapFloat size;
    
protected:
    /// set up the pen and brush accordingly; reading closest value
    /// before time t from the buffers. Separated out for
    /// convenience, if ever I decide to do another kind of 2D plot.
    virtual void setDrawProperties(Marble::GeoPainter *p,double t);
    
public:
    MapItemPointRenderer(MapWidget *w);
    virtual void render(Marble::GeoPainter *painter);
    void parseConfig(Tokeniser *t);
};

/// a renderer for vectors
class MapItemVectorRenderer : public MapItemRenderer {
    int trailSize;
    MapColour col;
    MapFloat width;
    MapFloat length;
    DataBuffer<float> *angleBuf;
    float angleMult;
    
protected:
    /// set up the pen and brush accordingly; reading closest value
    /// before time t from the buffers. Separated out for
    /// convenience, if ever I decide to do another kind of 2D plot.
    virtual void setDrawProperties(Marble::GeoPainter *p,double t);
    
public:
    MapItemVectorRenderer(MapWidget *w);
    virtual void render(Marble::GeoPainter *painter);
    void parseConfig(Tokeniser *t);
};



/// the map widget, which renders MapPositionRenderer, each of
/// which has one or more MapItemRenderer.
class MapWidget : public Marble::MarbleWidget{
    Q_OBJECT
          
    QList<MapItemRenderer *> renderers;          
public:
    explicit MapWidget(QWidget *parent,Tokeniser *t);
    virtual void customPaint(Marble::GeoPainter *painter);
    void addRenderer(MapItemRenderer *r){
        renderers.append(r);
    }
    void removeRenderer(MapItemRenderer *r){
        renderers.removeAll(r);
    }
public slots:
    void dataChanged(){
        update();
    }
    
    
};

#endif /* __MAP_H */
