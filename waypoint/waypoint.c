/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>

#include "waypoint.h"

// max number of fields in a wp
#define MAXFIELDS 32
// how many times we retry to send waypoints if the protocol
// gets confused
#define RETRIES_PERMITTED 32
// how many times we go round the waypoint send loop before
// abandoning and restarting the protocol
#define MAX_LOOP_RETRIES 128

static int wperror=0;
static int autoswitchFlag=0;
static time_t timeOfLastMsg;
static time_t timeout=10;

void wpSetTimeout(int t){
    timeout = t;
}

static void _setAS(int line,int as){
    autoswitchFlag = as;
    printf("%d: setting autoswitch=%d\n",line,as);fflush(stdout);
}

#define setAS(x) _setAS(__LINE__,(x))

// fatal error - this MUST NOT OCCUR AS PART OF THE PROTOCOL. Set the 
// wperror variable instead.
static void wppanic(const char *s){
    printf("Waypoint system error: %s\n",s);
    exit(1);
}

#define dprintf printf

static int timerTicks=0;
static VOIDFUNC preCopyWorkingToTransit=NULL;
static VOIDFUNC postCopyTransitToWorking=NULL;

void wpSetPreCopyWorkingToTransit(VOIDFUNC f){
    preCopyWorkingToTransit=f;
}
void wpSetPostCopyTransitToWorking(VOIDFUNC f){
    postCopyTransitToWorking=f;
}


/*
 * Waypoint field definition table
 */


static int numFields=0;
static const char *fieldNames[32];
static double fieldDefaults[32];

void wpResetDefinitions(){
    int i;
    for(i=0;i<numFields;i++)
        free((void *)fieldNames[i]);
    fieldNames[0] = strdup("lat");
    fieldNames[1] = strdup("lon");
    
    fieldDefaults[0] = 0;
    fieldDefaults[1] = 0;
    numFields=2;
}

double wpGetDefault(int n){
    return fieldDefaults[n];
}
void wpSetDefault(int n,double v){
    fieldDefaults[n]=v;
}


int wpAddField(const char *name,double deflt){
    if(numFields==MAXFIELDS)
        wppanic("too many fields");
    fieldDefaults[numFields]=deflt;
    fieldNames[numFields]=strdup(name);
    return numFields++;
}

int wpGetFieldIdx(const char *name){
    int i;
    for(i=0;i<numFields;i++){
        if(!strcmp(name,fieldNames[i]))return i;
    }
    return -1;
}

/*
 * A waypoint set
 */

typedef struct  {
    double *data; // numFields doubles for every wp
    char *used; // one char for every wp
    int current;
    int count; // number of waypoints - not the number of doubles!
    int capacity;
} waypointset;


/// create a set large enough for n entries
static waypointset *createSet(int n){
    waypointset *wp = (waypointset *)malloc(sizeof(waypointset));
    wp->capacity = n;
    wp->count = 0;
    wp->used=(char *)malloc(sizeof(char)*n);
    wp->data = (double *)malloc(sizeof(double)*numFields*n);
    wp->current = 0;
    memset(wp->used,0,n);
    return wp;
}

static void grow(waypointset *wp){
    int n = wp->count*2;
    wp->used = (char *)realloc(wp->used,sizeof(char)*n);
    wp->data = (double *)realloc(wp->data,sizeof(double)*numFields*n);
    memset(wp->used+wp->capacity,0,n-wp->capacity);
    wp->capacity = n;
}


/// delete a waypoint set
static void deleteSet(waypointset *wp){
    free(wp->data);
    free(wp->used);
    free(wp);
}


/*
 * Initial sets are null
 */

static waypointset *working=NULL;
static waypointset *transit=NULL;
/// the state of any transit. Since no transit is being done initially,
/// the state is "complete."
static int transitComplete = 1;

/// set up a new waypoint
static void initEntry(int n){
    int i;
    working->used[n]=1;
    double *d = wpGet(n);
    for(i=0;i<numFields;i++){
        d[i] = fieldDefaults[i];
    }
}


void wpCreateWorking(int cap){
    if(working){
        deleteSet(working);
    }
    working = createSet(cap);
}


int wpGetCount(){
    if(working)
        return working->count;
    else
        return 0;
}

double *wpGet(int n){
    if(working)
        return working->data+numFields*n;
    else
        return NULL;
}

int wpGetCountTransit(){
    if(transit)
        return transit->count;
    else
        return 0;
    
}

double *wpGetTransit(int n){
    if(transit)
        return transit->data+numFields*n;
    else
        return NULL;
}

int wpGetStride(){
    return numFields;
}

const char *wpGetFieldName(int i){
    if(i>=numFields)
        return NULL;
    else return fieldNames[i];
}

// same as wpGetStride!

int wpGetNumFields(){
    return numFields;
}



int wpAppend(){
    if(!working)
        return -1;
    if(working->count >= working->capacity)
        grow(working);
    
    printf("working: %x/%x, count=%d cap=%d\n",
           working->data,working->used,working->count,working->capacity);
    fflush(stdout);
    
    initEntry(working->count);
    return working->count++;
}

int wpAppendPos(double lat,double lon){
    int i = wpAppend();
    if(i>=0){
        double *d = wpGet(i);
        d[0]=lat;
        d[1]=lon;
    }
    return i;
}

void wpDelete(int n){
    int movesize;
    
    if(!working)
        return;
    if(n>=working->count)
        return;
    
    movesize = working->count-(n+1);
    memmove(working->data+numFields*n,
            working->data+numFields*(n+1),
            movesize*numFields*sizeof(double));
    memmove(working->used+n,
            working->used+(n+1),
            movesize*sizeof(char));
    working->count--;
    
}

double *wpInsert(int n){
    int movesize;
    
    if(!working)
        return NULL;
    if(n>=working->count)
        return NULL;
    if(working->count >= working->capacity)
        grow(working);
    
    movesize = working->count-n;
    
    memmove(working->data+numFields*(n+1),
            working->data+numFields*n,
            movesize*numFields*sizeof(double));
    
    memmove(working->used+(n+1),
            working->used+n,
            movesize*sizeof(char));
    
    printf("working: %x/%x, count=%d cap=%d\n",
           working->data,working->used,working->count,working->capacity);
    fflush(stdout);
    working->count++;
    initEntry(n);
    return working->data+numFields*n;
}

void wpSetCurrent(int cur){
    if(working)
        working->current = cur;
}
/// set the current waypoint in the working set
int wpGetCurrent(){
    return working?working->current:-1;
}


// delete old working buffer and make a new one from
// the transit buffer, also set the current waypoint
void wpCopyTransitToWorking(int cur){
    printf("COPYING TRANSIT TO WORKING - current is %d\n",cur);
    if(working)deleteSet(working);
    working=createSet(transit->count);
    memcpy(working->data,transit->data,transit->count*numFields*sizeof(double));
    memcpy(working->used,transit->used,transit->count*sizeof(char));
    working->capacity = transit->count; // we're only copying the actually used slots
    working->count = transit->count;
    working->current = cur;
    if(postCopyTransitToWorking)
        (*postCopyTransitToWorking)();
}

void wpCopyWorkingToTransit(){
    if(preCopyWorkingToTransit)
        (*preCopyWorkingToTransit)();
    if(transit)deleteSet(transit);
    transit=createSet(working->count);
    memcpy(transit->data,working->data,working->count*numFields*sizeof(double));
    memcpy(transit->used,working->used,working->count*sizeof(char));
    transit->capacity = working->count; // we're only copying the actually used slots
    transit->count = working->count;
    transit->current = 0; // used to count current waypoint in sending
}


/*
 * The 'tokeniser' - I'm not a fan of strtok, for many reasons,
 * not least of which is that it requires write access to the
 * string it parses. But I'm not going to throw my big tokeniser
 * in here. That's like using a sledgehammer to crack a nut.
 * 
 * This simply reads from a string until it finds a specific
 * character, copying what it reads into the buffer (except
 * for the delimiter). It will return a pointer to the character
 * after the delimiter, or null if there is no string remaining
 * after the token read.
 * There is no whitespace skip.
 */

static const char *getnext(char *out,const char *s,char delim){
    while(*s && *s!=delim)
        *out++ = *s++;
    *out = 0;
    if(!*s)return NULL;
    return ++s;
}



/*
 * 
 * The actual protocol itself
 */

/// each state has functions: one called on entry, 
/// one called in response to a message being received,
/// one called when a timer expires

typedef struct {
    void (*onEntry)();
    void (*onMsg)(const char *msg,uint16_t recvtid);
    void (*onTimer)(int tick);
} wpstate;

wpstate states[]; // fwd declaration

// list of internal states

#define INIT		0
#define SENDINIT	1
#define SENDING		2
#define RECVINIT	3
#define RECEIVING	4
#define RECVDONE	5
#define SWITCH		6
#define FETCH		7
#define REQSWITCH	8

/// the current state
static int state = INIT;

/// the transaction identifier
static uint16_t tid=0;

void defaultAckFunc(int err,int was){}

// the callback functions
static SENDFUNC sendFunc;
static ACKFUNC ackFunc=defaultAckFunc;

// the retry sending counter
int protocolRetryCount = RETRIES_PERMITTED;

uint16_t Fletcher16( const uint8_t* data, int count )
{
    uint16_t sum1 = 0;
    uint16_t sum2 = 0;
    int index;
    
    char buf[256];
    strncpy(buf,data,count);buf[count]=0;
    
    for( index = 0; index < count; ++index )
    {
	if(data[index]=='\n')
	{
	    break;
	}
        sum1 = (sum1 + data[index]) % 255;
        sum2 = (sum2 + sum1) % 255;
    }
    
    return (sum2 << 8) | sum1;
}

/// send a packet, prefixing +, the checksum, and the TID

static void send(const char *s,...){
    va_list args;
    va_start(args,s);
    char buf[512];
    // first, we write the TID past where the checksum will go
    printf("TId = %d\n",tid);
    sprintf(buf+5,"+%03x+",tid);
    // then we write the message past that
    vsnprintf(buf+10,400,s,args);
    // then we calculate the checksum and put that at the beginning
    int sum = Fletcher16(buf+5,strlen(buf+5));
    sprintf(buf,"+%04x",sum,tid);
    buf[5]='+';
    (*sendFunc)(buf);
    va_end(args);
}

/// transfer to a given state, calling the necessary transition
/// function(s)

static void gotoState(int s){
    time(&timeOfLastMsg);
    printf("Entering state %d\n",s);
    state = s;
    (*states[state].onEntry)();
}

static char sendMsgType;
static void _wpSendWaypoints(int autoswitch){
    
    wperror=0;
    if(!working)
        wppanic("no working set to send");
    
    wpCopyWorkingToTransit();
    
    if(sendMsgType=='t') // complete restart
        tid = rand()%1000; // generate a randomish number for the TID
    transitComplete=0; // mark as not having yet been completely sent
    
    // send the start of the transaction
    printf("Sending transaction start with autoswitch=%d\n",autoswitch);
    send("%c%d:%d:%d",sendMsgType,working->count,numFields,autoswitch);
    autoswitchFlag = autoswitch;
    
    gotoState(SENDINIT);
}

void wpSendWaypoints(int autoswitch){
    sendMsgType = 't';
    _wpSendWaypoints(autoswitch);
}


static void attemptRestartSend(){
    if(!--protocolRetryCount){
        dprintf("----- RETRY SEND ABORT ----- RETURN TO INIT\n");
        wperror=1;
        gotoState(INIT);
        (*ackFunc)(wperror,autoswitchFlag);
    } else {
        printf("retry with autoswitch=%d\n",autoswitchFlag);
        _wpSendWaypoints(autoswitchFlag);
    }
}


// some messages are handled in multiple states - they should
// be handled in separate functions here, and then called from
// those states.

static void processTMsg(const char *msg){
    char buf[32];
    int n,nf;
    
    wperror=0; // clear error state
    
    msg=getnext(buf,msg,':'); // number of items
    n = atoi(buf);
    
    msg=getnext(buf,msg,':');   // number of fields in sender config
    nf = atoi(buf);
    
    msg=getnext(buf,msg,0); // autoswitch flag
    setAS(atoi(buf));
    
    dprintf("new transaction, TID=%d, waypoints=%d, nf=%d, as=%d\n",tid,n,nf,autoswitchFlag);
    if(nf!=numFields){
        dprintf("Number of fields in sender is %d, but I have %d\n",nf,numFields);
        wperror = 2; // we won't receive waypoints if this is active
    } 
    
    // delete any old transit buffer
    if(transit)deleteSet(transit);
    transit=createSet(n);
    transit->count=n; // indicate that we have N empty slots
    // mark the transit buffer as invalid (it's not got anything
    // in it)
    transitComplete=0;
    // and go to RECVINIT
    gotoState(RECVINIT);
}

static int startingWaypoint = 0;
static void processUMsg(const char *msg){
    char buf[32];
    getnext(buf,msg,0);
    startingWaypoint = atoi(buf);
    setAS(startingWaypoint+1);
    gotoState(SWITCH);
}


static void nullEntry(){}
static void nullMsg(const char *msg,uint16_t recvtid){}
static void nullTimer(int tick){}

void initMsg(const char *msg,uint16_t recvtid){
    char buf[32];
    const char *orig = msg;
    int errcode;
    
    switch(*msg++){
    case 't':
        // this is always accepted and always resets the TID
        tid = recvtid;
        processTMsg(msg);
        break;
    case 'u':
        // this is always accepted and always resets the TID
        tid = recvtid;
        processUMsg(msg);
        break;
    case 'a':
        // tell the sender to Shut The Hell Up, we've already
        // had an acknowledgement.
        if(tid==recvtid){ // ignore wrong TID
            switch(*msg++){
            case 'w':
                msg=getnext(buf,msg,0);
                setAS(atoi(buf));
                send("stfu");
                break;
            case 'u':
                msg=getnext(buf,msg,0);
                errcode=atoi(buf);
                if(errcode){
                    dprintf("Error in switch acknowledge: %d\n",errcode);
                }
                send("stfu");
                break;
                
            case 't':
                send("stfu");
                break;
            default:
                dprintf("invalid %s in init\n",orig);
                break;
            }
        }
        break;
    case 'r':
        // this is always accepted and always resets the TID
        tid = recvtid;
        if(!working){ // forestall the fatal error in wpSendWaypoints()
            dprintf("no working set to send\n");
            send("ae:4");
        } else {
            setAS(atoi(msg));
            sendMsgType='T';
            _wpSendWaypoints(autoswitchFlag); 
        }
        break;
    case 's':break; // don't process stfu when we already have.
    default:
        dprintf("invalid %s in init\n",orig);
        break;
    }
}

void sendInitEntry(){
    timerTicks = 0; // reset the timer
}

void sendInitMsg(const char *msg,uint16_t recvtid){
    char buf[32];
    const char *orig=msg;
    int err;
    
    if(tid!=recvtid)
        return;
    
    switch(*msg++){
    case 'a':
        switch(*msg++){
        case 't':
            msg=getnext(buf,msg,0);
            err=atoi(buf);
            if(err){
                dprintf("error %d received in +at\n",err);
                wperror=err+10;
                gotoState(INIT);
                (*ackFunc)(wperror,autoswitchFlag);
            } else {
                gotoState(SENDING);
            }
        }
        break;
    default:
        dprintf("invalid in sendinit: %s\n",orig);
        attemptRestartSend();
        break;
    }
}

void sendInitTimer(int tick){
    if(tick == 150) // didn't get an ack after some time
        attemptRestartSend();
}

static int sendingLoopRetryCount = 0;

static void sendNextWaypoint(){
    char buf[512];
    int i;
    
    // start building the string
    sprintf(buf,"w%d ",transit->current);
    // get the waypoint data
    double *d = wpGetTransit(transit->current);
    
    // append the fields to the string
    for(i=0;i<numFields;i++){
        sprintf(buf+strlen(buf),"%f ",d[i]);
    }
    // remove the final space
    buf[strlen(buf)-1]=0;
    // and send
    send(buf);
    
    // increment the counter
    transit->current++;
    // loop if we're back at the start, but only let that
    // happen a few times
    if(transit->current == transit->count){
        transit->current =0;
        if(++sendingLoopRetryCount == MAX_LOOP_RETRIES){
            dprintf("gone round the loop too many times, retrying whole protocol");
            attemptRestartSend();
        }
    }
}

void sendingEntry(){
    sendingLoopRetryCount = 0;
    timerTicks = 0;
    sendNextWaypoint();
}

void sendingMsg(const char *msg,uint16_t recvtid){
    if(tid!=recvtid)
        return;
    
    if(*msg == 'a'){
        switch(msg[1]){
        case 'w':
            setAS(atoi(msg+2));
            // all messages received
            transitComplete = 1;
            gotoState(INIT);
            
            (*ackFunc)(wperror,autoswitchFlag);
            return; // NORMAL EXIT POINT
        case 'm':
            return; // ANOTHER NORMAL EXIT POINT
        }
    } else {
        gotoState(INIT);//abnormal
    }
    // only happens if we don't get a valid msg
    attemptRestartSend();
}

void sendingTimer(int t){
    sendNextWaypoint();
}


// handle +w messages, with the +w stripped off.

static void processWMsg(const char *msg){
    char buf[32];
    int n,i;
    double *d;
    
    // record the waypoint
    msg=getnext(buf,msg,' ');
    n = atoi(buf);
    dprintf("packet for waypoint %d\n",n);
    d = wpGetTransit(n);
    for(i=0;i<numFields;i++){
        msg=getnext(buf,msg,' ');
        //            dprintf("  field %d has data string %s\n",i,buf);
        d[i] = atof(buf);
    }
    transit->used[n]=1; // mark as used
    // now check to see if they're all used
    for(i=0;i<transit->count;i++){
        if(!transit->used[i])break;
    }
    if(i==transit->count){
        dprintf("All waypoints done!\n");
        // they are all filled in now
        gotoState(RECVDONE);
    } else {
        if(!((i+1)%5))
            send("am"); // occasional ack
    }
          
}


void recvInitEntry(){
    send("at%d",wperror);
    timerTicks=0;
}

void recvInitMsg(const char *msg,uint16_t recvtid){
    switch(*msg++){
    case 'w':
        if(tid == recvtid && !wperror){ // ignore waypoints if we're in an error state
            processWMsg(msg);
            // processWMsg() may have put us into the RECVDONE state
            // if there was only 1 waypoint. That's OK, but if that
            // wasn't done, we need to go into RECEIVING so we stop
            // sending the ack messages.
            if(state!=RECVDONE)gotoState(RECEIVING);
        }
        break;
    case 't':
        tid = recvtid;
        processTMsg(msg);
        break;
    case 's':
        if(tid==recvtid){
            gotoState(INIT);
            (*ackFunc)(wperror,autoswitchFlag);
        }
        break;
    default:
        if(tid==recvtid)
            gotoState(INIT);//abnormal
    }
}

void recvInitTimer(int t){
    // if the timer expires, it probably means that the other end
    // didn't get our acknowledgement. Send it again. Send these
    // quite close together so that the other side doesn't switch
    // to a new TID too soon.
    if(t==7)
        recvInitEntry();
}


void recvMsg(const char *msg,uint16_t recvtid){
    switch(*msg++){
    case 'w':
        if(tid==recvtid)
            processWMsg(msg);
        break;
    case 't':
        tid = recvtid;
        processTMsg(msg);
        break;
    default:
        if(tid==recvtid)
            gotoState(INIT);//abnormal
        break;
    }
}

void recvDoneEntry() {
    timerTicks=0;
    transitComplete = 1;
    
    send("aw%d",autoswitchFlag);
    // and here we do the autoswitch
    if(autoswitchFlag){
        wpCopyTransitToWorking(autoswitchFlag-1);
    }
}

void recvDoneMsg(const char *msg,uint16_t recvtid){
    char buf[32];
    switch(*msg++){
    case 'w':
        // yes, yes, we've already got them all!
        if(tid==recvtid)
            send("aw%d",autoswitchFlag);
        break;
    case 's':
        // shut up!
        if(tid==recvtid){
            gotoState(INIT);
            (*ackFunc)(wperror,autoswitchFlag);
        }
        break;
    case 'u':
        tid=recvtid;
        processUMsg(msg);
        break;
    case 't':
        tid=recvtid;
        processTMsg(msg);
        break;
    default:
        if(tid==recvtid)
            gotoState(INIT);//abnormal
        break;
    }
}

void recvDoneTimer(int t){
    // keep sending ack until we're shut up
    if(t==13){
        send("aw%d",autoswitchFlag);
        timerTicks=0;
    }
}

/// the value sent in the +au messages
static int switchErrorCode=5;

void switchEntry(){
    timerTicks=0;
    if(!transit)
        switchErrorCode=1; // error : no transit set
    else if(!transitComplete)
        switchErrorCode=2; // error : transit set incomplete
    else {
        // startingWaypoint was set when we first received the +u
        // message
        wpCopyTransitToWorking(startingWaypoint);
        switchErrorCode=0;
    }
    send("au%d",switchErrorCode);
}

void switchMsg(const char *msg,uint16_t recvtid){
    switch(*msg++){
    case 't':
        tid = recvtid;
        processTMsg(msg);
        break;
    case 's':
        // shut up!
        if(tid==recvtid){
            gotoState(INIT);
            (*ackFunc)(wperror,autoswitchFlag);
        }
        break;
    case 'u':
        // do it again
        tid = recvtid;
        processUMsg(msg);
        break;
    default:
        if(tid==recvtid)
            gotoState(INIT);//abnormal
        break;
    }
}

void switchTimer(int t){
    // keep sending ack until we're shut up
    if(t==12){
        send("au%d",switchErrorCode);
        timerTicks=0;
    }
}

void fetchEntry(){
    send("r%d",autoswitchFlag);
}

void fetchMsg(const char *msg,uint16_t recvtid){
    char buf[32];
    switch(*msg++){
    case 't':
        tid=recvtid;
        processTMsg(msg);
        break;
    case 'T':
        if(tid==recvtid) // as above but we check the TID.
            processTMsg(msg);
        break;
    case 'a':
        if(tid == recvtid && *msg++=='e'){
            msg = getnext(buf,msg,0);
            wperror = atoi(buf);
            gotoState(INIT);
            (*ackFunc)(wperror,autoswitchFlag);
        }
        break;
    default:
        if(tid == recvtid)
            gotoState(INIT);//abnormal
    }
}

void fetchTimer(int t){
    if(t==13){
        fetchEntry();
        timerTicks=0;
    }
}


void reqSwitchEntry(){
    send("u%d",transit->current);
}

void reqSwitchMsg(const char *msg,uint16_t recvtid){
    char buf[32];
    int err;
    switch(*msg++){
    case 't':
        tid = recvtid;
        processTMsg(msg);
        break;
    case 'a':
        if(tid == recvtid){
            msg=getnext(buf,msg,0);
            err = atoi(buf);
            if(err){
                wperror=atoi(buf)+10;
                dprintf("Error in switch acknowledge: %d\n",wperror);
            }
            setAS(working->current+1); // because we did actually switch
            gotoState(INIT);
            (*ackFunc)(wperror,autoswitchFlag);
        }
        break;
    default:
        if(tid == recvtid)
            gotoState(INIT);//abnormal
    }
}

void reqSwitchTimer(int t){
    // keep sending until we're shut up
    if(t==14){
        reqSwitchEntry();
        timerTicks=0;
    }
}


/// this is the state function table

wpstate states[] = {
    {nullEntry,initMsg,nullTimer},		// init
    {sendInitEntry,sendInitMsg,sendInitTimer},	// sendinit
    {sendingEntry,sendingMsg,sendingTimer},	// sending
    {recvInitEntry,recvInitMsg,recvInitTimer},	// recvinit
    {nullEntry,recvMsg,nullTimer},		// receiving
    {recvDoneEntry,recvDoneMsg,recvDoneTimer},	// recvdone
    {switchEntry,switchMsg,switchTimer},	// switch
    {fetchEntry,fetchMsg,fetchTimer},		// fetch
    {reqSwitchEntry,reqSwitchMsg,reqSwitchTimer},// reqSwitch
};


void wpInit(SENDFUNC f,ACKFUNC af){
    time_t t;
    srand(time(&t));
    wpResetDefinitions();
    sendFunc = f;
    if(af)
        ackFunc = af;
    wpReset();
}

void wpReset(){
    wperror=0;
    state=0;
    transitComplete=1;
}


void wpRequestWaypoints(int autoswitch){
    wperror=0;
    setAS(autoswitch);
    tid = rand()%1000;
    gotoState(FETCH);
}


void wpRequestSwitch(int cw){
    wperror=0;
    tid = rand()%1000;
    transit->current=cw;
    gotoState(REQSWITCH);
}

uint16_t hex2int(const char *a, unsigned int len)
{
    int i;
    uint16_t val = 0;
    
    char buf[256];
    strncpy(buf,a,len);buf[len]=0;
    
    for(i=0;i<len;i++)
        if(a[i] <= 57)
            val += (a[i]-48)*(1<<(4*(len-1-i)));
        else
            val += (a[i]-87)*(1<<(4*(len-1-i)));
    return val;
}

void wpProcessString(const char *s){
    const char *o = s;
    ++s; // skip '+'
    // we're now on the checksum
    
    int sum = hex2int(s,4);
    s+=4; // skip the received checksum and '+'
    // calculate the checksum for the string
    int strSum = Fletcher16(s,strlen(s));
    if(sum==strSum) {
        s++;  // skip +
        // checksum is good. Now get the TID
        uint16_t recvTID = hex2int(s,3);
        if(recvTID!=tid)
            printf("TID %x != received %x\n",tid,recvTID);
        s+=4; // skip TID and '+'
        (*states[state].onMsg)(s,recvTID);
        time(&timeOfLastMsg);
    } else {
        printf("Bad checksum: %s\n",o);
        printf("%4x!=%4x\n",sum,strSum);
    }
    
}

void wpTick(){
    // handle timeout
    if(state != INIT){
        time_t t;
        time(&t); 
        t -= timeOfLastMsg;
    
        if(t>timeout){
            wperror=3;
            gotoState(INIT);
            (*ackFunc)(wperror,autoswitchFlag);
        }
    }
    
    
    
    (*states[state].onTimer)(timerTicks++); // skip the '+'
}



int wpStatus(){
    int ret = transitComplete?0:1;
    ret |= state<<1;
    ret |= wperror<<8;
    return ret;
}

int loadproblem=0;

static int scanLine(char *s,double *wp){
    int entries = 0;
    int i;
    while(isspace(*s))s++;
    
    printf("LINE: %s\n",s);
    
    char fieldsGot[256]; 
    memset(fieldsGot,0,numFields);
    
    while(*s){
        char *t=s;
        while(!isspace(*t))t++;
        *t++=0;
        char *q = strchr(s,'=');
        if(q){
            *q++=0;
            int field = wpGetFieldIdx(s);
            if(field<0){
                printf("UNKNOWN FIELD %s - ignoring\n",s);
                fflush(stdout);
                loadproblem|=1;
            } else {
                double v = atof(q);
                wp[field]=v;
                fieldsGot[field]=1;
                entries++;
            }
        }
        s=t;
        while(isspace(*s))s++;
    }
    int notprovct=0;
    for(i=0;i<numFields;i++)
        if(!fieldsGot[i]){
            printf("NOT PROVIDED %s\n",fieldNames[i]);
            notprovct++;
        }
    if(entries && notprovct) // blank lines don't matter
        loadproblem|=2;
    return entries;
}

int wpLoadStatus() {
    return loadproblem;
}

int wpLoad(const char *fn){
    FILE *a;
    loadproblem=0;
    if(!(a=fopen(fn,"r")))
        return 1;
    
    char line[512];
    
    wpCreateWorking(1000);
    while(fgets(line,512,a)!=NULL){
        int n = wpAppend();
        double *d = wpGet(n);
        // quickly delete the end item if there weren't at least lat
        // and lon in the loaded entry
        int ents =  scanLine(line,d);
        if(ents<2)
            working->count--; 
        printf("%d added, %d entries\n",n,ents);
    }
    
    int i;
    for(i=0;i<wpGetCount();i++){
        double *d = wpGet(i);
        printf("%d -- %f %f\n",i,d[0],d[1]);
    }
    
    fclose(a);
    return 0;
}

int wpSave(const char *fn){
    int i,f;
    FILE *a;
    double *d;
    
    if(!(a=fopen(fn,"w")))
        return 1;
    
    for(i=0;i<wpGetCount();i++){
        d = wpGet(i);
        for(f=0;f<numFields;f++){
            fprintf(a,"%s=%f%c",wpGetFieldName(f),d[f],
                    f==numFields-1 ? '\n' : ' ');
        }
    }
    fclose(a);
    return 0;
}
