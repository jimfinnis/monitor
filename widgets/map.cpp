/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */
#include "map.h"

#include "../config.h"
#include "../exception.h"
#include "../datamgr.h"
#include "../widgetmgr.h"
#include "../tokens.h"

#include <QVariant>
#include <marble/GeoPainter.h>

using namespace Marble;

MapWidget::MapWidget(const char *frameName,Tokeniser *t) :
MarbleWidget((QWidget *)NULL) {
    
    ConfigRect pos;
    pos.x = -1;
    bool done=false;
    
    setProjection(Marble::Mercator);
    setMapThemeId("earth/openstreetmap/openstreetmap.dgml");
    
    setProperty("longitude", QVariant(-4));
    setProperty("latitude", QVariant(52));
    setShowOverviewMap(false);
    setShowScaleBar(false);
    setShowCompass(false);
    setShowGrid(false);
    
    t->getnextcheck(T_OCURLY);
    while(!done){
        
        switch(t->getnext()){
        case T_POS:
            pos = ConfigManager::parseRect();
            break;
        case T_POINT:
            {
                MapItemPointRenderer *r = 
                      new MapItemPointRenderer(this);
                r->parseConfig(t);
                addRenderer(r);
                break;
            }
        case T_VECTOR:
            {
                MapItemVectorRenderer *r = 
                      new MapItemVectorRenderer(this);
                r->parseConfig(t);
                addRenderer(r);
                break;
            }
        case T_CCURLY:
            done = true;
            break;
        default:
            throw Exception().set("Unexpected '%s'",t->getstring());
            
        }
    }
    if(pos.x < 0)
        throw Exception("no position given for map");
    
    
    WidgetManager::addWidget(frameName,this,pos.x,pos.y,pos.w,pos.h);
}

void MapItemRenderer::parseLocation(Tokeniser *t){
    latBuf = ConfigManager::parseFloatSource();
    t->getnextcheck(T_COMMA);
    longBuf = ConfigManager::parseFloatSource();
}


void MapWidget::customPaint(GeoPainter *painter){
    // we iterate through the renderers and call each one
    
    for(int i=0;i<renderers.size();i++){
        renderers[i]->render(painter);
    }
}


/// produce a value from 0-1 from a buffer interpolated read,
/// with a default provided if there is no buffer or the read is
/// not good. Also clips to 0,1.

inline float interpBufToRangeWithDefault(DataBuffer<float> *b,double t,float def){
    float v;
    if(b && b->readInterp(t,&v)!=RawDataBuffer::NoData){
        v -= b->minVal;
        v /= b->maxVal - b->minVal;
        if(v<0)v=0;
        if(v>1)v=1;
    } else
        v = def;
    return v;
}

QColor MapColour::get(double t){
    float hue = interpBufToRangeWithDefault(hueBuf,t,0);
    float sat = interpBufToRangeWithDefault(satBuf,t,(hueBuf||valBuf)?1:0);
    float val = interpBufToRangeWithDefault(valBuf,t,1);
    
    QColor c;
    c.setHsvF(hue,sat,val);
    
    // mix the colour we just made with the preset colour
    // using multiplicative blend
    
    qreal r1,g1,b1;
    qreal r2,g2,b2;
    
    color.getRgbF(&r1,&g1,&b1);
    c.getRgbF(&r2,&g2,&b2);
    
    c.setRgbF(r1*r2,g1*g2,b1*b2);
    
    return c;
}

float MapFloat::get(double t){
    float s=interpBufToRangeWithDefault(buf,t,0);
    s = s*factor+base;
    return s;
}

MapItemPointRenderer::MapItemPointRenderer(MapWidget *w) : MapItemRenderer(w){
    // set defaults
    trailSize = 0;
}


void MapItemPointRenderer::parseConfig(Tokeniser *t){
    t->getnextcheck(T_OCURLY);
    bool done=false;
    while(!done){
        switch(t->getnext()){
        case T_CCURLY:
            done = true;
            break;
        case T_LOCATION:
            parseLocation(t);
            break;
        case T_ON:
            // will add the widget to the buffer's listeners, so that
            // a new point is plotted when this value changes
            onVar = new DataRenderer(widget,ConfigManager::parseFloatSource());
            break;
            
        case T_COL:
        case T_COLOUR:
        case T_COLOR:
            col.color = ConfigManager::parseColour(Qt::white);
            break;
        case T_HUE:
            col.hueBuf = ConfigManager::parseFloatSource();
            break;
        case T_SATURATION:
            col.satBuf = ConfigManager::parseFloatSource();
            break;
        case T_VALUE:
            col.valBuf = ConfigManager::parseFloatSource();
            break;
            
        case T_TRAIL:
            trailSize = t->getnextint();
            break;
            
        case T_SIZE:
            size.base = t->getnextfloat();
            break;
        case T_SIZERANGE: // "4 10 var foo" means size goes from 4 to 10 according to foo.
            size.base = t->getnextfloat();
            size.factor = t->getnextfloat()-size.base;
            size.buf = ConfigManager::parseFloatSource();
            break;
            
        default:
            throw Exception().set("Unexpected '%s'",t->getstring());
            
        }
    }
    if(!latBuf || !longBuf)
        throw Exception("map item requires a location");
    if(!onVar)
        throw Exception("map item requires an 'on' clause");
}

void MapItemPointRenderer::setDrawProperties(GeoPainter *p,double t){
    
    QColor c = col.get(t);
    
    p->setPen(Qt::black);
    p->setBrush(c);
}


void MapItemPointRenderer::render(GeoPainter *painter){
    // this is the buffer from which we get the points
    RawDataBuffer *onBuf = onVar->getBuffer();
    QPen pen;
    for(int i=0;i<trailSize+1;i++){
        // get the time of the item
        double t = onBuf->getTimeOfDatum(i);
        
        // we now need to get the various data,
        // interpolating as we go.
        
        float lat,lng;
        if(latBuf->readInterp(t,&lat)>0){
            if(longBuf->readInterp(t,&lng)>0){
                // create geo object
                GeoDataCoordinates pos(lng,lat,0,GeoDataCoordinates::Degree);
                // work out the size separately from the pen
                float s = size.get(t);
                // now the rest of the pen properties
                setDrawProperties(painter,t);
                // and draw with unprojected width and height
                painter->drawEllipse(pos,s,s);
            }
        }
    }
}

MapItemVectorRenderer::MapItemVectorRenderer(MapWidget *w) : MapItemRenderer(w){
    // set defaults
    trailSize = 0;
    width.base = 1;
}

void MapItemVectorRenderer::parseConfig(Tokeniser *t){
    t->getnextcheck(T_OCURLY);
    bool done=false;
    while(!done){
        switch(t->getnext()){
        case T_CCURLY:
            done = true;
            break;
        case T_LOCATION:
            parseLocation(t);
            break;
        case T_ON:
            // will add the widget to the buffer's listeners, so that
            // a new vector is plotted when this value changes
            onVar = new DataRenderer(widget,ConfigManager::parseFloatSource());
            break;
            
        case T_DEGREES:
            /// the angle should be in degrees
            angleBuf = ConfigManager::parseFloatSource();
            angleMult = 3.1415927f/180.0f;
            break;
        case T_RADIANS:
            /// the angle should be in radians
            angleBuf = ConfigManager::parseFloatSource();
            angleMult = 1;
            break;
            
        case T_COL:
        case T_COLOUR:
        case T_COLOR:
            col.color = ConfigManager::parseColour(Qt::white);
            break;
        case T_HUE:
            col.hueBuf = ConfigManager::parseFloatSource();
            break;
        case T_SATURATION:
            col.satBuf = ConfigManager::parseFloatSource();
            break;
        case T_VALUE:
            col.valBuf = ConfigManager::parseFloatSource();
            break;
            
        case T_TRAIL:
            trailSize = t->getnextint();
            break;
            
        case T_WIDTH:
            width.base = t->getnextfloat();
            break;
        case T_WIDTHRANGE: // "4 10 var foo" means size goes from 4 to 10 according to foo.
            width.base = t->getnextfloat();
            width.factor = t->getnextfloat()-width.base;
            width.buf = ConfigManager::parseFloatSource();
            break;
            // the length is in kilometers
        case T_LENGTH:
            length.base = t->getnextfloat();
            break;
        case T_LENGTHRANGE:
            length.base = t->getnextfloat();
            length.factor = t->getnextfloat()-length.base;
            length.buf = ConfigManager::parseFloatSource();
            break;
        default:
            throw Exception().set("Unexpected '%s'",t->getstring());
            
        }
    }
    if(!latBuf || !longBuf)
        throw Exception("map item requires a location");
    if(!onVar)
        throw Exception("map item requires an 'on' clause");
}

void MapItemVectorRenderer::setDrawProperties(GeoPainter *p,double t){
    QColor c = col.get(t);
    QPen pen(c);
    pen.setWidth((int)width.get(t));
    p->setPen(pen);
}

void MapItemVectorRenderer::render(GeoPainter *painter){
    // this is the buffer from which we get the points
    RawDataBuffer *onBuf = onVar->getBuffer();
    QPen pen;
    
    for(int i=0;i<trailSize+1;i++){
        // get the time of the item
        double t = onBuf->getTimeOfDatum(i);
        
        // we now need to get the various data,
        // interpolating as we go.
        
        float lat,lng;
        if(latBuf->readInterp(t,&lat)>0){
            if(longBuf->readInterp(t,&lng)>0){
                // create geo object
                GeoDataCoordinates pos(lng,lat,0,GeoDataCoordinates::Degree);
                // work out the length separately from the pen
                float s = length.get(t);
                // now the rest of the pen properties
                setDrawProperties(painter,t);
                
                // Get the screen coordinates of the start point.
                qreal x1,y1;
                if(widget->screenCoordinates(lng,lat,x1,y1)){
                    // work out the angle (this will only work at tight zooms)
                    float angle;
                    if(angleBuf->readInterp(t,&angle)){
                        // work out the screen coords of the end point
                        angle *= angleMult;
                        qreal x2 = x1 + sinf(angle)*s;
                        qreal y2 = y1 - cosf(angle)*s; // -1 because Y axis goes down
                        // and that's the line we draw
                        painter->drawLine(x1,y1,x2,y2);
                    }
                }
            }
        }
    }
}
