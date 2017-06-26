/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#ifndef __MAP_H
#define __MAP_H

#if MARBLE
#include <QWidget>
#include <QString>
#include <QHash>
#include <QPixmap>
#include <QList>
#include <QMenu>
#include <marble/MarbleWidget.h>
#include <marble/GeoDataLatLonBox.h>

#include "datarenderer.h"
#include "tokeniser.h"
#include "mapbbox.h"
#include "udp.h"

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
    
    /// clear any trail
    virtual void clearTrail(){}
};

/// used for drawing images over the map

class MapImage {
    QPixmap *pixmap;
    double mapPosLat1,mapPosLon1,scrPosX1,scrPosY1;
    double mapPosLat2,mapPosLon2,scrPosX2,scrPosY2;
    MapWidget *widget;
    double alpha;
public:
    MapImage(const char *imageName,MapWidget *w,double a,
         double lat1,double lon1,double x1,double y1,
         double lat2,double lon2,double x2,double y2);
    
    void draw(Marble::GeoPainter *p);
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
    int trailEvery; //!< interval for trail storage
    MapColour col;
    MapFloat size;
    DataBuffer<float> *label; //!< a float buffer from which we read an INTEGER label!
    char labelFormat[128]; //!< label format string, valid if label is nonzero
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

/// a renderer for lines between pairs of points - the point starts at the
/// position of the renderer (inherited from MapItemRenderer) and ends at endLong/LatBuf.

class MapItemLineRenderer : public MapItemRenderer {
protected:
    DataBuffer<float> *endLongBuf;
    DataBuffer<float> *endLatBuf;
    MapColour col;
    MapFloat width;
    bool clip; //!< true if we only draw the item if it is fully in the window
    float arrowAngle; // arrowhead angle
    float arrowLength; // arrowhead length (as part of vector)
    /// set up the pen and brush accordingly; reading closest value
    /// before time t from the buffers. Separated out for
    /// convenience, if ever I decide to do another kind of 2D plot.
    virtual void setDrawProperties(Marble::GeoPainter *p,double t);
public:
    MapItemLineRenderer(MapWidget *w);
    virtual void render(Marble::GeoPainter *painter);
    void parseConfig(Tokeniser *t);
    
};

/// a renderer for vectors
class MapItemVectorRenderer : public MapItemRenderer {
    int trailSize;
    int trailEvery; //!< interval for trail storage
    MapColour col;
    MapFloat width;

    MapFloat length;
    DataBuffer<float> *angleBuf;
    float angleMult;
    bool clip; //!< true if we only draw the item if it is fully in the window
    
    float arrowAngle; // arrowhead angle
    float arrowLength; // arrowhead length (as part of vector)
    
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


/// renders the waypoint lists.
class MapItemWaypointRenderer : public MapItemRenderer {
    // very little in the way of configuration
    void renderWaypoints(class Marble::GeoPainter *p,bool working,QColor col);
    bool clip;
public:
    MapItemWaypointRenderer(MapWidget *w);
    virtual void render(Marble::GeoPainter *painter);
    void parseConfig(Tokeniser *t);
};



/// the map widget, which renders MapPositionRenderer, each of
/// which has one or more MapItemRenderer. Note that we have to
/// handle keypresses using an event filter because otherwise the
/// map steals the events.
class MapWidget : public Marble::MarbleWidget, UDPClientSendListener{
    Q_OBJECT
          
    /// output variables for emergency waypoints, etc.
    OutValue *outLat,*outLon;
    bool hasOut;
    bool immediate;
    QList<MapImage *>images;
    
    QList<MapItemRenderer *> renderers;
    MapBoundingBox box; //!< used for reset on next draw
    QMenu *contextMenu; //!< a right-click menu replacing the standard one
    
    // menu actions, checked on return from exec() of the menu
    QAction *openMenuAction;
    QAction *clearTrailAction;
    QAction *placeWaypointAction;
    QAction *delWaypoint;
    QAction *appendWaypoint;
    QAction *insertWaypoint;
    QAction *clearWaypoints;
    QAction *resetAction;
    QAction *loadAction;
    QAction *saveAction;
    QAction *sendAction;
    QAction *fetchAction;
    QAction *toggleDragAction;
    QAction *changeWPDisplayAction;
    QAction *useAction;
    
    void delWaypointDo();
    void appendWaypointDo(double lat, double lon);
    void insertWaypointDo(double lat, double lon);
    void clearWaypointsDo();
    void openCurrentWaypoint();
    
    void clearTrails();
    
    void loadImageAndCreateTransform(const char *imageName,
                                     double lat1,double lon1,double x1,double y1,
                                     double lat2,double lon2,double x2,double y2);
    
    int dragWaypoint; //!< waypoint being dragged or -1
    bool dragEnabled; //!< are we allowed to drag?
    
    
    // drag-and-drop waypoint editing
    
    /// select and start dragging any waypoint under the mouse,
    /// return true if we got something, otherwise false.
    bool checkStartDrag(double x,double y,bool allowdrag=true);
    
    /// on mouse release, stop dragging
    void stopDrag();
    
    /// mouse is down moving - deal with dragging, returning
    /// true if we are dragging, false if not.
    bool checkDrag(double x,double y);
public:
    bool hasWaypointRenderer;
    // if non-null, the first packet to arrive on this renderer will
    // center the map
    MapItemRenderer *centeringRenderer;
    int curWaypoint; //!< waypoint being edited, target WP or -1
    int wpDisplayMode; //!< how we're viewing waypoints
    
    
    explicit MapWidget(QWidget *parent,Tokeniser *t);
    virtual void customPaint(Marble::GeoPainter *painter);
    void addRenderer(MapItemRenderer *r){
        renderers.append(r);
    }
    void removeRenderer(MapItemRenderer *r){
        renderers.removeAll(r);
    }
    
    void addToBox(float lng,float lat){
        box.add(lng,lat);
    }
    
    /// if r is the centering renderer, center on the given coords and then
    /// clear the centering renderer
    void checkCenteringRenderer(MapItemRenderer *r,float lat,float lng);
    
    /// required to cope with keys
    virtual bool eventFilter(QObject *obj,QEvent *event);

    /// for UDPClientSendListener
    virtual void onSend(){}


public slots:
    
    /// reset the bounding box to encompass all visible items
    /// (as set by addToBox during previous draw)
    void resetBox();

    void dataChanged(){
        update();
    }
};
#endif /* marble */
#endif /* __MAP_H */
