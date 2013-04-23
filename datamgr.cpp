/**
 * \file
 * Implementation details of the data manager system,
 * particularly DataManager itself.
 * 
 * \author $Author$
 * \date $Date$
 */

#include <QHash>
#include <QSet>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "datamgr.h"
#include "expr.h"

float DataManager::dataValidInterval=DEFAULTDATAVALIDINTERVAL;

DataBuffer<float> *DataManager::lastPacketIntervalBuffer,
                    *DataManager::timeSinceLastPacketBuffer;

float DataManager::packetTimeOffset=0;

double DataManager::getTimeNow(){
    // get current time including microseconds
    timeval tv;
    gettimeofday(&tv,NULL);
    
    // convert to a double
    double usec = tv.tv_usec;
    double t = tv.tv_sec;
    t += usec * 1e-6;
    return t;
}

/// a map of strings onto data buffer objects, maintained by the DataManager.
static QHash<QString,DataBuffer<float> *> floatMap;

/// this is a static list of buffers, used to check for linked variables. 
static QList<RawDataBuffer *> linkedVariablesUsedInLastUpdate;

/// all the variables there are
static QList<RawDataBuffer *> allVariables;

/// a set of expressions which require recalculation because their
/// dependent buffers have changed
static QSet<Expression *> expressionsRequiringRecalc;


static void clearLinkedBuffersUsed(){
    linkedVariablesUsedInLastUpdate.clear();
}
static void checkLinkedBuffersUsed(){
    // one for each type
    for(int i=0;i<linkedVariablesUsedInLastUpdate.count();i++){
        linkedVariablesUsedInLastUpdate[i]->checkLinksAndAddDuplicatesIfRequired();
    }
}

DataBuffer<float> *DataManager::createFloatBuffer(const char *name, int capacity,float mn, float mx){
    DataBuffer<float> *b = new DataBuffer<float>(
                                                 RawDataBuffer::FLOAT,
                                                 name,capacity,mn,mx);
    floatMap.insert(name,b);
    allVariables.append(b);
    return b;
}

DataBuffer<float> *DataManager::findFloatBuffer(const char *name){
    // note that Qt will provide a default-constructed value of 0
    // for pointer values, so this will return NULL if the item is 
    // not found.
    return floatMap.value(name);
}

void RawDataBuffer::notify(){
    for(int i=0;i<listeners.size();i++){
        listeners[i]->onNewData(this);
    }
    for(int i=0;i<exprs.size();i++)
        expressionsRequiringRecalc.insert(exprs[i]);
}

void DataManager::recalcExpressions(){
    foreach(Expression *p, expressionsRequiringRecalc){
        p->recalc();
    }
    expressionsRequiringRecalc.clear();
}

static double packetTime=0;

void writeToBuffer(const char *name,const char *value){
    if(!strcasecmp("time",name)){ //handle time as a special case
        packetTime = atof(value);
    } else if(DataBuffer<float> *b = DataManager::findFloatBuffer((name))){
        // it's a float buffer, add it as a float
        b->write(packetTime,atof(value));
        // if this is a linked buffer, we need to check that all the linked buffers were
        // also updated and if not create duplicate entries.
        if(b->isLinked())
            linkedVariablesUsedInLastUpdate.append(b);
    }
}

enum ParseMode { WAITVAR,INVAR,WAITEQ,WAITVAL,INVAL };

/// parse each individual line in a packet
static void parseLine(const char *s){
    const char *varstart;
    const char *valstart;
    int n;
    ParseMode mode = WAITVAR;
    
    char varbuf[256];
    char valbuf[256];
    
    do {
        switch(mode){
        case WAITVAR:
            if(isalnum(*s)){
                varstart=s;
                n=1;
                mode=INVAR;
            }
            break;
        case INVAR:
            if(!isalnum(*s)){
                strncpy(varbuf,varstart,n);
                varbuf[n]=0;
                if(*s=='=')
                    mode=WAITVAL;
                else
                    mode=WAITEQ;
            }
            else n++;
            break;
        case WAITEQ:
            if(*s == '=')
                mode = WAITVAL;
            break;
        case WAITVAL:
            if(*s && !isspace(*s)){
                valstart=s;
                n=1;
                mode=INVAL;
            }
            break;
        case INVAL:
            if(!*s || *s==',' || isspace(*s)){
                strncpy(valbuf,valstart,n);
                valbuf[n]=0;
                mode=WAITVAR;
                
//                printf("Got [%s = %s]\n",varbuf,valbuf);
                writeToBuffer(varbuf,valbuf);
            }
            else n++;
            break;
        }
    }while(*s++);
    
}

void DataManager::parsePacket(char *s,int size){
    int done=0;
    
    s[size]=0;
    // the packet is made up of null-terminated lines. Note that the clearLinked.. and checkLinked..
    // calls could be moved out of this loop *if* we can guarantee that the times are all the same
    // for every line.
    while(done<size){
        int len = strlen(s);
        clearLinkedBuffersUsed(); // clear the list of linked buffers used in this packet
        parseLine(s);
        // perform required duplicate writes
        checkLinkedBuffersUsed();
        s+=len+1;
        done+=len+1;
    }

    // calculate packet time offset, the difference between the time on the packet and when
    // it was received locally

    packetTimeOffset = getTimeNow()-packetTime;

    // write the difference interval between packet times to a special buffer
    
    static double lastPacketTime = -1;
    if(lastPacketTime>=0){
        lastPacketIntervalBuffer->write(packetTime,packetTime-lastPacketTime);
    }
    lastPacketTime = packetTime;
    
    // recalculate any expressions whose variables may have changed
    recalcExpressions();
    
}


// specialisation of float readInterp

template<> int DataBuffer<float>::readInterp(double t,float *output){
    
    int imin,imax;
    int res = chop(t,&imin,&imax);
    switch(res){
    case RawDataBuffer::TooEarly:
        {
            // I need to get the earliest item
            int k = n-1;
            if(k>capacity-1)k=capacity-1;
            *output = read(k)->d;
        }
        break;
    case RawDataBuffer::TooLate:
        // I need to get the latest item
        *output = read(0)->d;
        break;
    case RawDataBuffer::NoData:
        *output=0;
        break;
    case RawDataBuffer::Exact:
        *output = read(imin)->d;
        break;
    case RawDataBuffer::Inexact:
        {
            // the interpolation case. We swap around here
            // because MAX index number is the OLDEST and so
            // has MIN time.
            Datum<float> *mx = read(imin);
            Datum<float> *mn = read(imax);
            double mnt = mn->t;
            double mxt = mx->t;
            float factor = (t - mnt);
            factor /= (float) (mxt-mnt);
            
            *output = mn->d + (mx->d - mn->d)*factor;
        }
        break;
    }
    return res;
    
}

void DataManager::updateAll(){
    for(int i=0;i<allVariables.size();i++){
        allVariables[i]->notify();
    }
    // also update the time since last packet (packetTime is the time of the
    // last received packet)
    timeSinceLastPacketBuffer->write(getTimeNow(),
                                     getTimeNow()-packetTime);
}

void DataManager::init(){
    lastPacketIntervalBuffer = createFloatBuffer("lastpacketinterval",1000,0,1);
    lastPacketIntervalBuffer->setAutoRange();
    timeSinceLastPacketBuffer = createFloatBuffer("timesincepacket",1000,0,1);
    timeSinceLastPacketBuffer->setAutoRange();
}
