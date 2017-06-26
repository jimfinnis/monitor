/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */
#include "map.h"
#if MARBLE

#include "../config.h"
#include "../exception.h"
#include "../datamgr.h"
#include "../tokens.h"
#include "../app.h"
#include "../waypointdialog.h"

#include "waypoint/waypoint.h"

#define PI 3.1415927


#include <QMessageBox>
#include <QVariant>
#include <QGridLayout>
#include <QKeyEvent>
#include <marble/GeoPainter.h>
#include <marble/GeoDataPoint.h>
#include <marble/GeoDataLatLonBox.h>

using namespace Marble;



MapWidget::MapWidget(QWidget *parent,Tokeniser *t) :
MarbleWidget((QWidget *)NULL) {
    
    centeringRenderer = NULL;
    ConfigRect pos;
    pos.x = -1;
    bool done=false;
    hasWaypointRenderer = false;
    bool dragEnabled = false;
    
    curWaypoint = -1;
    dragWaypoint = -1;
    wpDisplayMode = 0;
    
    setProjection(Marble::Mercator);
    setMapThemeId("earth/openstreetmap/openstreetmap.dgml");
    
    setProperty("longitude", QVariant(-4));
    setProperty("latitude", QVariant(52));
    setShowOverviewMap(false);
    setShowScaleBar(false);
    setShowCompass(false);
    setShowGrid(false);
    
    float centreLat = -1000;
    float centreLong = 0;
    float cameraHeight = 0;
    hasOut=false;
    char outLatName[128];
    char outLonName[128];
    bool always = false;
    immediate = false;
    char imageFileName[256];
    imageFileName[0]=0;
    double imageLat[2],imageLon[2];
    double imageX[2],imageY[2];
    double alpha;
    MapImage *mi;
    
    pos = ConfigManager::parseRect();
    t->getnextcheck(T_OCURLY);
    while(!done){
        switch(t->getnext()){
        case T_CENTRE:
            centreLat = t->getnextfloat();
            t->getnextcheck(T_COMMA);
            centreLong = t->getnextfloat();
            centerOn(centreLong,centreLat);
            break;
        case T_IMMEDIATE:
            immediate=true;
            break;
        case T_HEIGHT:
            setDistance(t->getnextfloat());
            break;
        case T_OUT:
            t->getnextident(outLatName);
            t->getnextcheck(T_COMMA);
            t->getnextident(outLonName);
            hasOut=true;
            break;
        case T_IMAGE:
            t->getnextcheck(T_OCURLY);
            if(!t->getnextstring(imageFileName))
                throw UnexpException(t,"image file name");
            if(t->getnext() == T_ALPHA){
                alpha = t->getnextfloat();
            } else {
                alpha = 1.0;
                t->rewind();
            }
                
            // two pos/screen pairs.
            t->getnextcheck(T_POS);
            imageLat[0] = t->getnextfloat();
            t->getnextcheck(T_COMMA);
            imageLon[0] = t->getnextfloat();
            t->getnextcheck(T_IMAGE);
            imageX[0] = t->getnextfloat();
            t->getnextcheck(T_COMMA);
            imageY[0] = t->getnextfloat();
            
            t->getnextcheck(T_POS);
            imageLat[1] = t->getnextfloat();
            t->getnextcheck(T_COMMA);
            imageLon[1] = t->getnextfloat();
            t->getnextcheck(T_IMAGE);
            imageX[1] = t->getnextfloat();
            t->getnextcheck(T_COMMA);
            imageY[1] = t->getnextfloat();
            t->getnextcheck(T_CCURLY);
            mi = new MapImage(imageFileName,this,alpha,
                              imageLat[0],imageLon[0],imageX[0],imageY[0],
                              imageLat[1],imageLon[1],imageX[1],imageY[1]
                              );
            images.append(mi);
    
            break;
        case T_POINT:
            {
                MapItemPointRenderer *r = 
                      new MapItemPointRenderer(this);
                r->parseConfig(t);
                addRenderer(r);
                break;
            }
        case T_LINE:
            {
                MapItemLineRenderer *r =
                      new MapItemLineRenderer(this);
                r->parseConfig(t);
                addRenderer(r);
                break;
            }
        case T_WAYPOINT:
            {
                MapItemWaypointRenderer *r = 
                      new MapItemWaypointRenderer(this);
                r->parseConfig(t);
                addRenderer(r);
                hasWaypointRenderer = true;
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
            throw ParseException(t).set("Unexpected '%s'",t->getstring());
            
        }
    }
    
    if(hasOut){
        outLat = new OutValue(outLatName,0,always);
        outLat->listener=this;
        UDPClient::getInstance()->add(outLat);
        outLon = new OutValue(outLonName,0,always);
        outLon->listener=this;
        UDPClient::getInstance()->add(outLon);
    }
    
    // connect us up to the app's mapreset() signal
    connect(getApp(),SIGNAL(mapreset()),this,SLOT(resetBox()));
    
    // using an event filter is a hack, really, but overriding keyPressEvent
    // doesn't work because the map handles the keys first. I'm also going to use this
    // to handle the mouse click event.
    
    installEventFilter(this);
    
    // create a new context menu (since we can't add to the existing one)
    
    contextMenu = new QMenu("blort",this);
    
    openMenuAction = contextMenu->addAction("Open Marble menu");
    resetAction = contextMenu->addAction("Reset map");
    clearTrailAction = contextMenu->addAction("Clear trails (TBD)");
    //    placeWaypointAction = contextMenu->addAction("Place emergency waypoint");
    delWaypoint = contextMenu->addAction("Delete current waypoint");
    appendWaypoint = contextMenu->addAction("Append waypoint at end");
    insertWaypoint = contextMenu->addAction("Insert waypoint before");
    clearWaypoints = contextMenu->addAction("Clear waypoints");
    changeWPDisplayAction = contextMenu->addAction("Change WP display mode");
    toggleDragAction = contextMenu->addAction("Enable waypoint dragging");
    loadAction = contextMenu->addAction("Load waypoints");
    saveAction = contextMenu->addAction("Save waypoints");
    sendAction = contextMenu->addAction("Send waypoints");
    fetchAction = contextMenu->addAction("Fetch waypoints");
    useAction = contextMenu->addAction("EMERGENCY goto point");
    
    
    
    // and add the widget
    QGridLayout *l = (QGridLayout*)parent->layout();
    setMinimumSize(pos.minsizex,pos.minsizey);
    l->addWidget(this,pos.y,pos.x,pos.h,pos.w);
}

MapImage::MapImage(const char *imageName,
                   MapWidget *w,double a,
                   double lat1,double lon1,double x1,double y1,
                   double lat2,double lon2,double x2,double y2){
    pixmap = new QPixmap(imageName);
    widget = w;
    alpha = a;
    
    mapPosLat1 = lat1;    mapPosLon1 = lon1;
    scrPosX1 = x1;        scrPosY1 = y1;
    mapPosLat2 = lat2;    mapPosLon2 = lon2;
    scrPosX2 = x2;        scrPosY2 = y2;
    
}

void MapImage::draw(GeoPainter *p){
    // first, work out the screen coordinates of the corners of the image in the map
    double x,y;
    
    widget->screenCoordinates(mapPosLon1,mapPosLat1,x,y);
    QPointF p1 = QPointF(x,y);
    widget->screenCoordinates(mapPosLon2,mapPosLat2,x,y);
    QPointF p2 = QPointF(x,y);
    
    // ok, we now have the screen coordinates for where we'd like to draw the map
    // in p1 and p2. Now generate a polygon from that. Note that quadToQuad needs
    // open polys but the constructor from a QRect returns a closed. Hence the hackage.
    
    QRectF r = QRectF(p1,p2);
    QPolygonF poly1;
    poly1 << r.topLeft() << r.topRight() << r.bottomRight() << r.bottomLeft();
    
    
    // now generate a polygon for the two positions within the image.
    r = QRectF(QPointF(scrPosX1,scrPosY1),
                      QPointF(scrPosX2,scrPosY2));
    
    QPolygonF poly2;
    poly2 << r.topLeft() << r.topRight() << r.bottomRight() << r.bottomLeft();
    
    // now generate a transform to go from one to the other!
    
    QTransform t;
    
    if(QTransform::quadToQuad(poly2,poly1,t)){
        p->save();
        p->setOpacity(alpha);
        p->setTransform(t);
        p->drawPixmap(0,0,*pixmap);
        p->restore();
    }
                                       
    
    
}


void MapItemRenderer::parseLocation(Tokeniser *t){
    latBuf = ConfigManager::parseFloatSource();
    t->getnextcheck(T_COMMA);
    longBuf = ConfigManager::parseFloatSource();
}


void MapWidget::customPaint(GeoPainter *painter){
    
    // we draw each image
    
    for(int i=0;i<images.size();i++){
        MapImage *mi = images[i];
        mi->draw(painter);
    }
    
    
    // we iterate through the renderers and call each one
    
    box.clear();
    for(int i=0;i<renderers.size();i++){
        renderers[i]->render(painter);
    }
}

void MapWidget::clearTrails(){
    for(int i=0;i<renderers.size();i++){
        renderers[i]->clearTrail();
    }
}

void MapWidget::resetBox(){
    GeoDataLatLonBox b = GeoDataLatLonBox();
    b.setBoundaries(box.north,box.south,box.east,box.west,
                    GeoDataCoordinates::Degree);
    centerOn(b);
}

void MapWidget::checkCenteringRenderer(MapItemRenderer *r,float lat,float lng){
    if(r==centeringRenderer){
        centeringRenderer = NULL;
        centerOn(lng,lat);
    }
}

bool MapWidget::checkStartDrag(double x,double y,bool allowdrag){
    qreal lon,lat;
    // run through the waypoints, converting them into screen coords
    double *head = wpGet(0);
    int n = wpGetCount();
    int stride = wpGetStride();
    
    // we iterate in reverse, since the draw order is forwards - we want
    // to pick the topmost ones.
    for(int i=n-1;i>=0;i--){
        qreal wx,wy;
        double *d = head+i*stride;
        if(screenCoordinates(d[1],d[0],wx,wy)){
            // work out screen distance (squared)
            wx-=x;            wy-=y;
            wx*=wx;           wy*=wy;
            if(wx+wy < 400){ // 20 pixels
                curWaypoint = i;
                if(allowdrag && dragEnabled)
                    dragWaypoint = i;
                update();
                return true;
            }
        }
    }
    return false;
}

void MapWidget::stopDrag(){
    dragWaypoint = -1;
}

bool MapWidget::checkDrag(double x,double y){
    if(dragWaypoint>=0){
        qreal lon,lat;
        if(geoCoordinates(x,y,lon,lat)){
            double *d = wpGet(dragWaypoint);
            d[0]=lat;
            d[1]=lon;
            update();
            return true;
        }
    }
    return false;
}


bool confirm(QString t){
    QMessageBox::StandardButton rv = 
          QMessageBox::question(NULL,"Are you sure?",
                                t,
                                QMessageBox::Yes|
                                QMessageBox::No);
    return rv == QMessageBox::Yes;
}

void MapWidget::openCurrentWaypoint(){
    if(curWaypoint>=0){
        WaypointDialog *dlg = new WaypointDialog(curWaypoint,this);
        dlg->exec();
        delete dlg;
    }
}

void MapWidget::delWaypointDo(){
    if(curWaypoint>=0 && confirm("delete waypoint?")){
        wpDelete(curWaypoint);
        if(curWaypoint >= wpGetCount())
            curWaypoint = -1;
    }
}
void MapWidget::appendWaypointDo(double lat,double lon){
    if(!wpGetCount()){
        wpCreateWorking(100);
        curWaypoint = dragWaypoint = -1;
    }
    curWaypoint = wpAppendPos(lat,lon);
}

void MapWidget::insertWaypointDo(double lat,double lon){
    if(!wpGetCount()){
        appendWaypointDo(lat,lon);
    } else if(curWaypoint>=0){
        double *d = wpInsert(curWaypoint);
        if(d){
            d[0] = lat;
            d[1] = lon;
        }
    }
}

void MapWidget::clearWaypointsDo(){
    if(confirm("Clear all waypoints?")){
        wpCreateWorking(100);
        curWaypoint = dragWaypoint = -1;
    } 
}

bool MapWidget::eventFilter(QObject *obj,QEvent *event){
    qreal lon,lat;
    
    switch(event->type()){
    case QEvent::KeyPress:{
        QKeyEvent *e = static_cast<QKeyEvent *>(event);
        if(getApp()->keyPress(e->key()))
            return true;
        break;
    }
    case QEvent::MouseButtonPress:{
        QMouseEvent *e = static_cast<QMouseEvent *>(event);
        
        qreal lat,lon;
        bool onscreen = geoCoordinates(e->x(),e->y(),
                                       lon,lat,
                                       GeoDataCoordinates::Degree);
        
        if(e->button() == Qt::RightButton){
            // the right button
            if(hasWaypointRenderer)
                checkStartDrag(e->x(),e->y(),false);
            QAction *a = contextMenu->exec(e->globalPos());
            if(a==openMenuAction)
                return false; // say this wasn't handled so the normal menu opens
            else if(a==resetAction)
                resetBox();
            else if(a==fetchAction)
                wpRequestWaypoints(1);
#if COMPLEXWAYPOINTS
            else if(a==sendAction && confirm("Are you sure? (Particularly about\nthe current waypoint!")){
                if(curWaypoint<0){
                    if(confirm("No current waypoint - use the first?"))
                        curWaypoint=0;
                }
                if(curWaypoint<0)
                    QMessageBox::critical(this,"Error","No current waypoint provided.");
                else {
                    if(wpGetCount() && curWaypoint>=0)
                        wpSendWaypoints(curWaypoint+1);
                    else QMessageBox::critical(this,"Error","No waypoints to send!");
                }
            }
            else if(a==useAction){
                if(confirm("THIS WILL DELETE ALL WAYPOINTS!")){
                    wpCreateWorking(100);
                    curWaypoint = wpAppendPos(lat,lon);
                    wpSendWaypoints(curWaypoint+1);
                }
            }
#else
            else if(a==useAction || a==sendAction){
                if(confirm("THIS WILL DELETE ALL REMOTE WAYPOINTS!")){
                    wpSendWaypoints(true);
                }
            }
            
#endif
            else if(a==loadAction)
                WaypointDialog::load(this);
            else if(a==saveAction)
                WaypointDialog::save(this);
            else if(a==toggleDragAction){
                dragEnabled = !dragEnabled;
                if(dragEnabled)
                    toggleDragAction->setText("Disable waypoint dragging");
                else
                    toggleDragAction->setText("Enable waypoint dragging");
            } else if(a==clearWaypoints && hasWaypointRenderer)
                clearWaypointsDo();
            else if(a==delWaypoint && hasWaypointRenderer)
                delWaypointDo();
            else if(a==insertWaypoint && hasWaypointRenderer)
                insertWaypointDo(lat,lon);
            else if(a==appendWaypoint && hasWaypointRenderer)
                appendWaypointDo(lat,lon);
            /* Colin's emergency waypoint putter ??
               else if(a==placeWaypointAction && hasWaypointRenderer){
                if(onscreen){
                    if(hasOut){
                        printf("Waypoint: %f %f\n",lat,lon);
                        outLat->set(lat);
                        outLon->set(lon);
                        if(immediate)
                            UDPClient::getInstance()->update();
                    }
                }
            } */
               else if(a==changeWPDisplayAction && hasWaypointRenderer) {
                wpDisplayMode++;
                wpDisplayMode %= 3;
                update();
            }
            return true;
        } else if(e->button() == Qt::LeftButton && hasWaypointRenderer){
            // the left button
            return checkStartDrag(e->x(),e->y());
        }
        break;
    }
    case QEvent::MouseButtonDblClick:
        if(hasWaypointRenderer){
            openCurrentWaypoint();
            return true;
        }
        break;
    case QEvent::MouseMove:{
        QMouseEvent *e = static_cast<QMouseEvent *>(event);
        if(hasWaypointRenderer)
            return checkDrag(e->x(),e->y());
    }
    case QEvent::MouseButtonRelease:{
        QMouseEvent *e = static_cast<QMouseEvent *>(event);
        if(e->button() == Qt::LeftButton){
            if(hasWaypointRenderer)
                stopDrag();
        }
    }
        break;
    default:
        break;
    }
    return false;
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
    trailEvery = 1;
}


void MapItemPointRenderer::parseConfig(Tokeniser *t){
    t->getnextcheck(T_OCURLY);
    bool done=false;
    label=NULL;
    
    while(!done){
        switch(t->getnext()){
        case T_CCURLY:
            done = true;
            break;
        case T_CENTRE:
            widget->centeringRenderer = this;
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
        case T_TRAILEVERY:
            trailEvery = t->getnextint();
            break;
        case T_LABEL:
            if(!t->getnextstring(labelFormat))
                throw ParseException(t,"expected a format string after 'label'");
            label = ConfigManager::parseFloatSource();
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
            throw ParseException(t).set("Unexpected '%s'",t->getstring());
            
        }
    }
    if(!latBuf || !longBuf)
        throw ParseException(t,"map item requires a location");
    if(!onVar)
        throw ParseException(t,"map item requires an 'on' clause");
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
    for(int i=0;i<trailSize+1;i+=trailEvery){
        // get the time of the item
        double t = onBuf->getTimeOfDatum(i);
        if(t<0.0001)break; // 0=out of range
        
        // we now need to get the various data,
        // interpolating as we go.
        
        float lat,lng;
        if(latBuf->readInterp(t,&lat)!=RawDataBuffer::NoData){
            if(longBuf->readInterp(t,&lng)!=RawDataBuffer::NoData){
                widget->addToBox(lng,lat);
                widget->checkCenteringRenderer(this,lat,lng);
                GeoDataCoordinates pos(lng,lat,0,GeoDataCoordinates::Degree);
                // work out the size separately from the pen
                float s = size.get(t);
                // now the rest of the pen properties
                setDrawProperties(painter,t);
                // and draw with unprojected width and height
                painter->drawEllipse(pos,s,s);
                // now draw any required label
                if(label){
                    float v;
                    if(label->readInterp(t,&v)!=RawDataBuffer::NoData){
                        char buf[128];
                        sprintf(buf,labelFormat,v);
                        painter->drawText(pos,buf);
                    }
                }
            }
        }
    }
}

MapItemVectorRenderer::MapItemVectorRenderer(MapWidget *w) : MapItemRenderer(w){
    // set defaults
    trailSize = 0;
    trailEvery = 1;
    width.base = 1;
    clip=false;
    arrowLength=10;
    //    clip=true; // until I figure out how to make it work
}

void MapItemVectorRenderer::parseConfig(Tokeniser *t){
    t->getnextcheck(T_OCURLY);
    bool done=false;
    while(!done){
        switch(t->getnext()){
        case T_CCURLY:
            done = true;
            break;
        case T_CENTRE:
            widget->centeringRenderer = this;
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
        case T_CLIP:
            clip=true;
            break;
        case T_LENGTHRANGE:
            length.base = t->getnextfloat();
            length.factor = t->getnextfloat()-length.base;
            length.buf = ConfigManager::parseFloatSource();
            break;
        case T_TRAIL:
            trailSize = t->getnextint();
            break;
        case T_TRAILEVERY:
            trailEvery = t->getnextint();
            break;
        case T_ARROW:
            t->getnextcheck(T_LENGTH);
            arrowLength = t->getnextfloat();
            t->getnextcheck(T_ANGLE);
            arrowAngle = t->getnextfloat()*(PI/180.0f);
            break;
            
        default:
            throw Exception(t->getline()).set("Unexpected '%s'",t->getstring());
            
        }
    }
    if(!latBuf || !longBuf)
        throw ParseException(t,"map item requires a location");
    if(!onVar)
        throw ParseException(t,"map item requires an 'on' clause");
}

void MapItemVectorRenderer::setDrawProperties(GeoPainter *p,double t){
    QColor c = col.get(t);
    QPen pen(c);
    pen.setWidth((int)width.get(t));
    p->setPen(pen);
}

void drawArrow(QPainter *painter,qreal x1,qreal y1,qreal x2,qreal y2,
               float arrowheadangle=-1,
               float arrowheadlength=-1,
               int penWidth=0
               ){
    
    arrowheadangle *= 3.1415927/180.0;
    
    // and that's the line we draw
    painter->drawLine(x1,y1,x2,y2);
    
    // draw arrowheads?
    if(arrowheadlength>0){
        float angle = atan2f(x2-x1,y1-y2); // note y negated
        float s = sqrtf((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
        
        // work out the length of the perpendicular
        qreal perplen = arrowheadlength*sinf(arrowheadangle);
        // work out the perpendicular vector by rotating the
        // (x2,y2) vector by 90, and multiply by the perplen
        qreal px = -cosf(angle)*perplen;
        qreal py = -sinf(angle)*perplen;
        // calculate the negative offset from the tip
        qreal offset = arrowheadlength*cosf(arrowheadangle);
        // calculate the point at which the arrowhead ends, joined, intersect
        // the line
        qreal arrx = x1 + sinf(angle)*(s-offset);
        qreal arry = y1 - cosf(angle)*(s-offset);
        
              
        // and now calculate those arrowhead ends
        QPointF points[3] = {
            {arrx+px,arry+py},
            {arrx-px,arry-py},
            {x2,y2}
        };
        
        QPen p = painter->pen();
        QBrush br(p.color());
        p.setWidth(penWidth);
        painter->setPen(p);
        painter->setBrush(br);
        painter->drawPolygon(points,3);
    }
}

void MapItemVectorRenderer::render(GeoPainter *painter){
    // this is the buffer from which we get the points
    RawDataBuffer *onBuf = onVar->getBuffer();
    QPen pen;
    
    for(int i=0;i<trailSize+1;i+=trailEvery){
        // get the time of the item
        double t = onBuf->getTimeOfDatum(i);
        
        // we now need to get the various data,
        // interpolating as we go.
        
        float lat,lng;
        if(latBuf->readInterp(t,&lat)!=RawDataBuffer::NoData){
            if(longBuf->readInterp(t,&lng)!=RawDataBuffer::NoData){
                widget->addToBox(lng,lat);
                // work out the length separately from the pen
                float s = length.get(t);
                // now the rest of the pen properties
                setDrawProperties(painter,t);
                
                // Get the screen coordinates of the start point.
                qreal x1,y1;
                if(widget->screenCoordinates(lng,lat,x1,y1) || !clip){
                    widget->checkCenteringRenderer(this,lat,lng);
                    // work out the angle (this will only work at tight zooms)
                    float angle;
                    if(angleBuf->readInterp(t,&angle)!=RawDataBuffer::NoData){
                        // work out the screen coords of the end point
                        angle *= angleMult;
                        qreal x2 = x1 + sinf(angle)*s;
                        qreal y2 = y1 - cosf(angle)*s; // -1 because Y axis goes down
                        
                        drawArrow(painter,x1,y1,x2,y2,arrowAngle,arrowLength);
                    }
                }
            }
        }
    }
}


MapItemLineRenderer::MapItemLineRenderer(MapWidget *w) : MapItemRenderer(w){
    // set defaults
    width.base = 1;
    clip=false;
}

void MapItemLineRenderer::parseConfig(Tokeniser *t){
    t->getnextcheck(T_OCURLY);
    bool done=false;
    while(!done){
        switch(t->getnext()){
        case T_CCURLY:
            done = true;
            break;
        case T_CENTRE:
            widget->centeringRenderer = this;
            break;
        case T_START:
            parseLocation(t);
            break;
        case T_CLIP:
            clip=true;
            break;
        case T_END_WD:
            endLatBuf = ConfigManager::parseFloatSource();
            t->getnextcheck(T_COMMA);
            endLongBuf = ConfigManager::parseFloatSource();
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
        case T_WIDTH:
            width.base = t->getnextfloat();
            break;
        case T_WIDTHRANGE: // "4 10 var foo" means size goes from 4 to 10 according to foo.
            width.base = t->getnextfloat();
            width.factor = t->getnextfloat()-width.base;
            width.buf = ConfigManager::parseFloatSource();
            break;
        case T_ARROW:
            t->getnextcheck(T_LENGTH);
            arrowLength = t->getnextfloat();
            t->getnextcheck(T_ANGLE);
            arrowAngle = t->getnextfloat()*(PI/180.0f);
            break;
        default:
            throw ParseException(t).set("Unexpected '%s'",t->getstring());
        }
    }
    if(!latBuf || !longBuf)
        throw ParseException(t,"map item requires a location");
    if(!onVar)
        throw ParseException(t,"map item requires an 'on' clause");
    if(!endLatBuf || !endLongBuf)
        throw ParseException(t,"map line item requires an end location");
}

void MapItemLineRenderer::setDrawProperties(GeoPainter *p,double t){
    QColor c = col.get(t);
    QPen pen(c);
    pen.setWidth((int)width.get(t));
    QBrush brush(c);
    p->setPen(pen);
    p->setBrush(brush);
}

void MapItemLineRenderer::render(GeoPainter *painter){
    RawDataBuffer *onBuf = onVar->getBuffer();
    
    double t = onBuf->getTimeOfDatum(0); // may return zero if no data. No matter.
    
    // get start coordinates, if any
    
    float startLat,startLng;
    if(latBuf->readInterp(t,&startLat)!=RawDataBuffer::NoData){
        if(longBuf->readInterp(t,&startLng)!=RawDataBuffer::NoData){
            
            // get end coordinates at that time
            
            float endLat,endLng;
            if(endLatBuf->readInterp(t,&endLat)!=RawDataBuffer::NoData){
                if(endLongBuf->readInterp(t,&endLng)!=RawDataBuffer::NoData){
                    widget->checkCenteringRenderer(this,endLat,endLng);
                    widget->addToBox(startLng,startLat);
                    widget->addToBox(endLng,endLat);
                    
                    // convert coordinats
                    qreal x1,y1,x2,y2;
                    
                    bool startOnScreen = widget->screenCoordinates(startLng,startLat,x1,y1);
                    bool endOnScreen =   widget->screenCoordinates(endLng,endLat,x2,y2);
                    
                    if(!clip || (startOnScreen && endOnScreen)){
                        setDrawProperties(painter,t);
                        drawArrow(painter,x1,y1,x2,y2,arrowAngle,arrowLength);
                    }
                }
            }
        }
    }
}


MapItemWaypointRenderer::MapItemWaypointRenderer(MapWidget *w) : MapItemRenderer(w) {
    clip = false;
}    

void MapItemWaypointRenderer::parseConfig(Tokeniser *t){
    t->getnextcheck(T_OCURLY);
    bool done=false;
    while(!done){
        switch(t->getnext()){
        case T_CLIP:
            clip=true;
            break;
        case T_CCURLY:
            done = true;
            break;
        }
    }
}

static void renderWaypointEllipse(MapWidget *w,GeoPainter *p,double *d,bool clip){
    qreal x,y;
    if(w->screenCoordinates(d[1],d[0],x,y) || !clip){
        GeoDataCoordinates pos(d[1],d[0],0,GeoDataCoordinates::Degree);
        p->drawEllipse(pos,10,10);
    }
}

static void renderWaypointText(MapWidget *w,GeoPainter *p,double *d,
                               int i,
                               bool clip,bool working,
                               QFontMetrics *fm){
    qreal x,y;
    if(w->wpDisplayMode && w->screenCoordinates(d[1],d[0],x,y) || !clip){
        double lat=d[0];
        double lon=d[1];
        
        QString text;
                
        switch(w->wpDisplayMode){
        case 1:
            text.sprintf("%d: %f %f",i,lat,lon);
            break;
        case 2:
            text.sprintf("%d: %f %f",i,lat,lon);
            for(int f=2;f<wpGetNumFields();f++){
                QString t;
                t.sprintf("\n%s: %f",wpGetFieldName(f),
                          d[f]);
                text += t;
            }
            break;
        }
        
        QRect r = fm->boundingRect(x+10,y-10,500,500,
                                  Qt::TextWordWrap,text);
        r.adjust(-10,0,5,0);
        p->drawRect(r);
        r.setWidth(500);
        r.setHeight(500);
        
        p->drawText(r,Qt::TextWordWrap,text);
        
        
    }
    
    
}

void MapItemWaypointRenderer::renderWaypoints(GeoPainter *p,bool working,QColor col){
    double *head = working?wpGet(0):wpGetTransit(0);
    int n = working?wpGetCount():wpGetCountTransit();
    int stride = wpGetStride();
    
    QPen pen(col);
    pen.setWidth(3);
    p->setPen(pen);
    
    qreal plon,plat;
    
    QFont font("Sans",8,QFont::Normal);
    p->setFont(font);
    
    QFontMetrics fm(font);
    
    
    for(int i=0;i<n;i++){
        qreal x1,y1;
        
        double *d = head+i*stride;
        double lat = d[0];
        double lon = d[1];
        GeoDataCoordinates pos(lon,lat,0,GeoDataCoordinates::Degree);
        widget->addToBox(lon,lat);
        
        if(widget->screenCoordinates(lon,lat,x1,y1) || !clip){
            if(i!=widget->curWaypoint)
                renderWaypointEllipse(widget,p,d,clip);
            if(i){
                qreal x2,y2;
                if(widget->screenCoordinates(lon,lat,x1,y1) || !clip){
                    if(widget->screenCoordinates(plon,plat,x2,y2) || !clip){
                        drawArrow(p,x2,y2,x1,y1,20.0,15.0,3);
                    }
                }
            }
            plon = lon;
            plat = lat;
            
        }
    }
    
    // render current waypoint last
    if(head && working && widget->curWaypoint>=0){
        p->save();
        p->setBrush(Qt::red);
        p->setPen(Qt::red);
        renderWaypointEllipse(widget,p,head+widget->curWaypoint*stride,clip);
        p->restore();
    }

    // now the text
    
    pen.setColor(Qt::black);
    pen.setWidth(1);
    p->setPen(pen);
    p->setBrush(Qt::white);
    
    
    for(int i=0;i<n;i++){
        double *d = head+i*stride;
        if(i!=widget->curWaypoint){
            renderWaypointText(widget,p,d,
                               i,clip,working,&fm);
        }
    }
    if(head && working && widget->curWaypoint>=0)
        renderWaypointText(widget,p,head+widget->curWaypoint*stride,
                           widget->curWaypoint,clip,working,&fm);
}

void MapItemWaypointRenderer::render(GeoPainter *painter){
    renderWaypoints(painter,false,Qt::black); // transit
    renderWaypoints(painter,true,Qt::yellow); // working
}



#endif

