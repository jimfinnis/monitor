/**
 * \file
 * C-language waypoint protocol. Each waypoint is a set of doubles, two of which
 * are lat and lon.
 * 
 */


#ifndef __WAYPOINT_H
#define __WAYPOINT_H

// just making sure we can use these functions from C++ code

#if defined(__cplusplus)
extern "C" {
#endif

/* Waypoint definition management */

/// reset the waypoint extra data definitions. After this, waypoints will
/// only be defined as having lat and lon. Any existing waypoint data will
/// also be cleared (it's going to be gibberish)
void wpResetDefinitions();

/// add a new extra field for each waypoint. The index will be returned. A default
/// is provided.
int wpAddField(const char *name,double deflt);

/// get the index for a waypoint field (-1 if not found)
int wpGetFieldIdx(const char *name);

const char *wpGetFieldName(int i);
int wpGetNumFields();

/// get the number of fields in the waypoint - effectively, the stride.

/* Waypoint data management */

/// return the number of waypoints in the working set 
int wpGetCount();

/// get a pointer to the first double in the given waypoint in the working set
double *wpGet(int n);

/// return the number of waypoints in the transit set
int wpGetCountTransit();

/// get a pointer to the first double in the given waypoint in the transit set
double *wpGetTransit(int n);

/// alternative access method - get the stride of the lists in
/// doubles, so we can step through from 0 onwards once we have
/// the address of the first item, by incrementing by the number.
int wpGetStride();

/// get a default value for a field
double wpGetDefault(int n);
/// set a default value for a field
void wpSetDefault(int n,double v);

/*
 * Save and load
 */

/// load waypoints from a file; returned value is errno (zero if ok)
int wpLoad(const char *fn);
/// true if the waypoint defs didn't match in the last load:
/// 1 set if an unknown field was specified
/// 2 set if an expected field was not present
int wpLoadStatus();

/// save waypoints to a file; returned value is errno (zero if ok)
int wpSave(const char *fn);

/// set the timeout. If, after the protocol has started, the interval
/// since the last message exceeds this time, we set an error state
/// and goto INIT. The ack function is also called. Default is 10s.
void wpSetTimeout(int t);


/*
 * Waypoint editing (of the working set)
 */


/// append a new waypoint to the working set
int wpAppend();

/// append a new waypoint with lat/long
int wpAppendPos(double lat,double lon);

/// delete a waypoint from the working set
void wpDelete(int n);

/// insert a waypoint before the given waypoint in the working set -
/// the item number will be the same as that passed in, so I'll
/// return the data pointer.
double *wpInsert(int n);

/// initialise the working set
void wpCreateWorking(int capacity);


/*
 * Comms functions
 */

/// start sending waypoints from me to the other system- 
/// if autoswitch is set, the remote will automatically copy
/// transit into working on completion. The starting waypoint
/// is autoswitch-1.
void wpSendWaypoints(int autoswitch);

/// simple send function for bob-like boats.
void wpSendSimple();

/// tell the other system to send waypoints to me
/// if autoswitch is set, automatically copy
/// transit into working on completion. The starting waypoint
/// is autoswitch-1.
void wpRequestWaypoints(int autoswitch);

/// tell the other system to copy the transit set
/// into the working set, and to use a given waypoint
/// as the current waypoint.
void wpRequestSwitch(int cw);

/// process an incoming string (including the initial '+')
void wpProcessString(const char *s);

/// the type of the 'send string' function
typedef void (*SENDFUNC)(const char *str);

/// the type of the 'acknowledge' function with a non-zero err msg,
/// and also the autoswitch flag (1+newstartpoint if a the transfer
/// should automatically copy the transit to the working set, zero otherwise)
/// This is also called in the case of a timeout (when the error=3).
typedef void (*ACKFUNC)(int err,int wasXferAutoswitch);

/// initialise, passing in a pointer to the 'send string'
/// and 'acknowledge' functions (the latter can be null)
void wpInit(SENDFUNC f,ACKFUNC af);

/// this must be called now and then 
void wpTick();

/// status data:
/// bottom bit = !transitComplete (so zero means it is)
/// 1-7 = state
/// 8+  = error
/// So an error-free complete transfer is zero.
///
/// Errors:
/// 1 = too many protocol retries
/// 2 = wrong number of fields
/// 10+ = an error occurred on the other system which has been 
///       transmitted back. Subtract 10 for the code.

int wpStatus();

/// reset the state, error and transitcomplete.
void wpReset();

/// get the current waypoint in the working set
void wpSetCurrent(int cur);
/// set the current waypoint in the working set
int wpGetCurrent();

/// delete any old transit buffer and create a new
/// one copied from the working set. The current waypoint index
/// in the transit buffer will be zeroed.
void wpCopyWorkingToTransit();
/// delete old working buffer and make a new one from
/// the transit buffer, also set the current waypoint in the working set.
void wpCopyTransitToWorking(int cur);

typedef void (*VOIDFUNC)();

void wpSetPreCopyWorkingToTransit(VOIDFUNC f);
void wpSetPostCopyTransitToWorking(VOIDFUNC f);




#if defined(__cplusplus)
}
#endif



#endif /* __WAYPOINT_H */
