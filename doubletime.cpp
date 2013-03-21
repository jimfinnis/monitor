/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */
#include <time.h>
#include "doubletime.h"

double gettime(){
    timespec ts;
    clock_gettime(CLOCK_REALTIME,&ts);
    
    double t = ts.tv_nsec;
    t *= 1e-9;
    t += ts.tv_sec;
    return t;
}
