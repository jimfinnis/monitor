/**
 * \file
 * My map bounding box, because early versions of marble don't
 * have a good one.
 * \author $Author$
 * \date $Date$
 */

#include <math.h>
class MapPoint {
public:
    MapPoint(){
        lat=0;
        lng=0;
    }
    MapPoint(float a,float b){
        lng=a;
        lat=b;
    }
    float lat,lng;
};

/// it's annoying that this is required, but the current code won't
/// build on many systems which don't have an uptodate marble.

class MapBoundingBox {
    bool empty;
public:

    float east;
    float west;
    float north;
    float south;
    
    MapBoundingBox(){
        empty=true;
    }
    
    MapBoundingBox(float lng,float lat){
        north = lat;
        south = lat;
        east = lng;
        west = lng;
        empty=false;
    }
    
    bool isEmpty() const{
        return empty;
    }
    
    void clear(){
        empty = true;
    }
    
    bool crossesDateLine() const {
        return (east<west || (east==180 && west == -180));
    }
    
    MapPoint centre() const {
        if(isEmpty())
            return MapPoint();
        
        float clon,clat;
        if(crossesDateLine()){
            clon = east+360-(east+360-west)/2;
            if(clon>360)clon-=360;
            if(clon<0)clon+=360;
        } else {
            clon = east-(east-west)/2;
        }
        clat = north-(north-south)/2;
        
        return MapPoint(clon,clat);
    }



    
    MapBoundingBox united(MapBoundingBox &other){
        if(isEmpty())
            return other;
        if(other.isEmpty())
            return *this;
        
        MapBoundingBox r;
        r.empty=false;
        MapPoint c1 = centre();
        MapPoint c2 = other.centre();
        
        r.north = std::max(north,other.north);
        r.south = std::min(south,other.south);
        
        float w1 = west;
        float w2 = other.west;
        float e1 = east;
        float e2 = other.east;
        
        bool const idl1 = east<west;
        bool const idl2 = other.east<other.west;
        
        if(idl1){
            w1 += 360;
            e1 += 360;
        }
        if(idl2){
            w2 += 360;
            e2 += 360;
        }
        
        if(fabsf(c2.lng-c1.lng)>180 || (idl1^idl2)){
            r.east=std::min(e1,e2);
            r.west=std::max(w1,w2);
        } else {
            r.east=std::max(e1,e2);
            r.west=std::min(w1,w2);
        }
        return r;
    }
    
    void dump(){
        printf("Long: %f to %f,   Lat: %f to %f\n",
               east,west,north,south);
    }
    void add(float lng,float lat){
        MapBoundingBox t = MapBoundingBox(lng,lat);
        *this = united(t);
    }
};

#ifndef __MAPBBOX_H
#define __MAPBBOX_H



#endif /* __MAPBBOX_H */
