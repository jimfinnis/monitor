/**
 * \file
 * Data buffer management. These are buffers of float arrays of varying size,
 * timestamped. The buffers are cyclic.
 * The data collection plugins feed them, and then the monitoring
 * widgets read them.
 * 
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __DATAMGR_H
#define __DATAMGR_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <QList>
#include "exception.h"

#define DEFAULTDATAVALIDINTERVAL (60*60*24*365*2)

#define UNUSED __attribute__((unused))

class RawDataBuffer;
template <class T> class DataBuffer;



/// this class manages the incoming data, keeping a list of the
/// DataBuffer objects and feeding them with data as it comes in.
/// It's an entirely static class (and many of its globals are static
/// global in datamgr.cpp)

class DataManager {
private:
    /// Take an incoming data item and feed it to the appropriate
    /// DataBuffer. Will attempt to coerce it to the correct type. If the name is
    /// "time", the internal time field will be set and used for all subsequent values.
    /// WILL SILENTLY FAIL if a variable is not in any of the internal lists.
    static void write(const char *name,const char *value);
    
public:
    /// initialise the datamanager; typically creates the built-in buffers
    static void init();
    
    /// Create a new float buffer with the given name, and capable
    /// of containing the given number of items; add it to the internal
    /// map and return it - or return NULL if there was a problem.
    static DataBuffer<float> *createFloatBuffer(const char *name,int capacity,float mn,float mx);
    /// Look for a buffer of the given name, return NULL if not found.
    static DataBuffer<float> *findFloatBuffer(const char *name);

    /// Take an incoming message consisting of a number of null-terminated lines
    /// in the form a=b c=d e=f and parse each line, feeding the results into the write()
    /// method of each buffer mentioned. Once the packet has been parsed, recalculate
    /// any expressions dependent on those buffers.
    /// Note - will write a null to s[size] to ensure termination.
    static void parsePacket(char *s,int size);
    
    /// helper function to get the current time including ms.
    static double getTimeNow();
    
#if DIAMOND
    static void pollDiamond();
#endif
    
    /// redraw everything, by sending a message saying that
    /// all variables have changed. Will NOT result in a UDP resend of output variables;
    /// they're a separate mechanism.
    static void updateAll();
    
    /// update the timesincepacket data; needs to be called every now
    /// and then even when data isn't received.
    static void tick();
    
    /// recalculate any expressions which need it, and 
    /// clear the set holding those expressions.
    static void recalcExpressions();
    
    /// if no data has been received for this number of seconds,
    /// the data is invalid (isRecent() will return false.)
    static float dataValidInterval;
    
    /// buffer containing intervals between subsequent
    /// packets
    static DataBuffer<float> *lastPacketIntervalBuffer;
    /// buffer containing time since last packet
    static DataBuffer<float> *timeSinceLastPacketBuffer;

    /// this is the offset between the timestamp on the last packet received and
    /// the local time at which it was received - it's localtime-packettime.
    static double packetTimeOffset;
};



/// data within a DataBuffer, timestamped.
template <class T> struct Datum {
    double t; //!< timestamp
    T d;
    
    /// true if the data arrived relatively recently
    bool isRecent(){
        // if the remote and monitor are on the same timebase, I could use just
        // getTimeNow here.
        double now = DataManager::getTimeNow() - DataManager::packetTimeOffset;
        return (now - t < DataManager::dataValidInterval);
    }
        
};


/// objects extend this interface if they want to be informed when
/// a data buffer gets a new value. Register these with
/// DataBuffer::addListener(). Naturally you'll have to extend this
/// several times with each different type if you require different types.

class DataBufferListener {
public:
    /// called when a databuffer is changed
    virtual void onNewData(RawDataBuffer *b) = 0;
};

class Expression;



/// this is the typeless core of a data buffer, allowing them to be linked
/// across types. MOST of the time we try to use this if we don't want to
/// constrain ourselves to a particular type of buffer, which has meant that
/// a lot of the listener code has had to be in here leading to some
/// unpleasantness if we want to get the typed buffer.

class RawDataBuffer {
    /// list of expressions dependent on this variable
    QList<Expression *>exprs;
protected:
    /// list of listeners which are notified when a new item is added
    QList<DataBufferListener *> listeners;
    
public:
    // return codes for chop, have to be in here to avoid template
    static const int Inexact = 1;
    static const int Exact = 2;
    static const int TooEarly = -1;
    static const int TooLate = -2;
    static const int NoData = 0;
    
    /// an enum of the ACTUAL type of this data buffer. The type system
    /// here is rather odd; internally all our listener stuff is on
    /// untyped buffers to stop the templates getting out of hand but that
    /// means we need a way to get the typed buffer out. Storing the type
    /// as an enum in here is a rather unpleasant way of doing that.
    enum Type {
        NONE,
        FLOAT,
          };
    
    /// get the actual type of the buffer
    Type getType(){
        return type;
    }
    
    /// if this is a float buffer, return the float version otherwise
    /// throw an exception
    DataBuffer<float> *getFloatBuffer(){
        if(type == FLOAT)
            return (DataBuffer<float> *)this;
        else
            throw Exception().set("'%s' is not a floating point value",name);
    }
    
    const char *name; //!< name of buffer
    
    int n; //!< number of data written to the array
    int capacity; //!< capacity of the array in elements
    
    /// the field which links the buffers together into a circular structure for
    /// the creation of "fake" entries for lat/long pairs etc. where only one update
    /// is received (See link()). Normally points to the buffer itself.

    RawDataBuffer *linkPtr;
    
    /// add an expression dependent on this variable
    void addExpr(Expression *e){
        exprs.append(e);
    }
    
    /// constructor, taking name and buffer capacity (the buffer itself
    /// is delegated to subclasses)
    
    RawDataBuffer(Type tp,const char *nam,int cap){
        capacity = cap;
        name = strdup(nam);
        n = 0;
        type = tp;
        linkPtr=this;
    }
    
    virtual ~RawDataBuffer(){
        free((void *)name);
    }
    
    /// get the timestamp for a datum, or 0 if the datum is out of range. 
    virtual double getTimeOfDatum(int i)=0;
    
    /// return the number of items written in total
    int getCount(){
        return n;
    }

    /// is this a linked buffer?
    bool isLinked(){
        return linkPtr != this;
    }
    
    /// link this data buffer with another - linked buffers form a set, and if any one
    /// buffer in the set is updated in a single call to DataManager::parseLine() without
    /// the others getting updated, those will also receive a "faked" update for
    /// their previous value. This is to make sure that variables which are naturally paired,
    /// such as lat/long, are easy to process because their buffers will have the same number
    /// of entries with the same timestamps.
    void link(RawDataBuffer *b){
        // first, find the end of the list.
        RawDataBuffer *p = linkPtr;
        while(p->linkPtr!=this)
            p = p->linkPtr;
        // we're now at the tail. Or the head. Whatever.
        b->linkPtr = this;
        p->linkPtr=b;
    }
    
    /// add an object which will be informed when a new data item is added -
    /// will not add a listener which is already in the list.
    void addListener(DataBufferListener *l){
        if(!listeners.contains(l)){
            listeners.append(l);
        }
    }
    
    /// remove an object from the listener list
    void removeListener(DataBufferListener *l){
        listeners.removeOne(l);
    }
    
    /// notify my listeners that I may have changed (these may also
    /// be sent to unchanged buffers periodically). Will add any
    /// dependent expressions to a set of expressions to be
    /// recalculated.
    virtual void notify();        
        

    /// create a duplicate of the latest item with the timestamp given. Used when
    /// linked variables only have one of the variables changing. Actually
    /// properly defined in the subclassing template
    virtual void addDuplicateWrite(double timeToWrite)=0;

    /// check all the buffers I am linked to - if any have less than my number of writes, add
    /// duplicate writes of the last datum to them.
    virtual void checkLinksAndAddDuplicatesIfRequired()=0;
    
private:
    /// the actual type of the buffer
    Type type;
    
    
};

/// a data buffer of a given type, containing data in the form specified
/// by its associated type
template <class T> class DataBuffer : public RawDataBuffer {
    /// the data within the buffer, stored as an array of Datum<T>
    /// objects - timestamped data.
    Datum<T> *data;
    
    /// if this is set, minVal and maxVal are modified by the incoming data
    bool autoRange;
    
public:
    
    
    T maxVal; //!< max value for rendering
    T minVal; //!< min value for rendering
    
    
    /// Create a new buffer
    /// @param type the actual type so we can use the get methods in RawDataBuffer
    /// @param nam name (will be duplicated)
    /// @param cap capacity, in elements
    /// @param mn the minimum value which can be rendered
    /// @param mx the maximum value which can be rendered
    explicit DataBuffer(RawDataBuffer::Type tp,
               const char *nam, int cap, T mn, T mx) : 
    RawDataBuffer(tp,nam,cap){
        // data must be a multiple of stride
        data = new Datum<T>[cap];
        maxVal = mx;
        minVal = mn;
        autoRange=false;
    }
    
    /// set the range after creation
    void setMinMax(T mn,T mx){
        maxVal = mx;
        minVal = mn;
    }
    
    /// set the buffer to automatically set its range from the data
    void setAutoRange(){
        autoRange = true;
    }
    
    bool isAutoRange(){
        return autoRange;
    }
    
    virtual ~DataBuffer(){
        delete[]data;
    }
    
    
    
    /// write a new datum to the buffer. Takes a float, which is the
    /// timestamp; and the datum itself.
    void write(double time,T d){
        int idx = (n++) % capacity;
        Datum<T> *datum = data+idx;
        datum->t = time;
        datum->d = d;
        
        if(autoRange){
            if(d < minVal)
                minVal = d;
            if(d > maxVal)
                maxVal = d;
        }
        
        // inform listeners
        notify();
    }

    /// read an item from a given index, where the index is 0 for the most
    /// recent item and increases as we go back into the buffer. Returns 
    /// the datum with a timestamp, or NULL if we're out of range (either
    /// that many items haven't yet been written, or we're trying to read
    /// larger than the capacity of the buffer.)
    Datum<T> *read(int i){
        if(i>=n||i<0)return NULL; // out of range
        if(i >= capacity)return NULL; // no longer in the buffer
        
        i = (((n-1)+capacity)-i)%capacity;
        return data + i;
    }
    
    virtual double getTimeOfDatum(int i){
        Datum<T> *d = read(i);
        if(!d)
            return 0;
        else
            return d->t;
    }
    
    
    /// search for a given time, giving the two elements
    /// which bracket it. Possible results are:
    /// - return TooLate, meaning the given time is after all the data
    /// - return TooEarly, meaning the given time is before all the data
    /// - return NoData, meaning there is no data in the buffer
    /// - return Exact, a match is found exactly, when it will be in mn
    /// - return Inexact, the match is between mx and mn, where min is the MOST RECENT index.
    /// The error values (the first three) are negative, the others are positive.
    /// The mn,mx values will be indices of the bracketing items.
    /// Return values are found in RawDataBuffer to avoid template problems.
    /// search for a given time, giving the two elements
    /// which bracket it. Possible results are:
    /// - return TooLate, meaning the given time is after all the data
    /// - return TooEarly, meaning the given time is before all the data
    /// - return NoData, meaning there is no data in the buffer
    /// - return Exact, a match is found exactly, when it will be in min
    /// - return Inexact, the match is between max and min, where min is the MOST RECENT index.
    /// The return values are indices into the buffer.
    int chop(double t,int *mn,int *mx){
        int imax = n-1;
        if(imax>=capacity)imax=capacity-1;
        int imin = 0;
        
        Datum<T> *d = read(imin);
        if(!d)return NoData;
        if(d->t < t)return TooLate;
        d = read(imax);
        if(d->t > t)return TooEarly;
        
        while(imax-imin > 1){
            int imid = (imax+imin)/2;
            Datum<T> *mid = read(imid);
            if(mid->t < t)
                imax = imid;
            else
                imin = imid;
        }
        *mx = imax;
        *mn = imin;
        if(read(imin)->t==t)
            return Exact;
        else
            return Inexact;
    }
    
    /// interpolated read - returns the same return codes as
    /// chop, but fills the output with a sensible value.
    int readInterp(double t,T *output);
    
    /// check all the buffers I am linked to - if any have less than my number of writes, add
    /// duplicate writes of the last datum to them.
    virtual void checkLinksAndAddDuplicatesIfRequired(){
        double t = read(0)->t; // get time of latest datum
        for(RawDataBuffer *b = linkPtr;b!=this;b=b->linkPtr){
            if(b->n < n)
                b->addDuplicateWrite(t);
        }
    }

    virtual void addDuplicateWrite(double timeToWrite){
        Datum<T> *d = read(0);
        // if there is no datum to duplicate, we've got the weird case where one of the linked
        // variables is being updated, but on the other we're silent. For example, as if in the first
        // tick we had a longitude update, but no latitude! This can't happen.
        // Actually, it can if one of the variables isn't defined!
        if(!d){
            throw Exception().set("There is no data at all for variable '%s'",name);
        }

        printf("no datum for buffer %s, adding %d = %f\n",name,timeToWrite,(float)d->d);
        write(timeToWrite,d->d);
    }

};


#endif /* __DATAMGR_H */
