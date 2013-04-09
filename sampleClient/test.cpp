/**
 * \file
 * Brief description. Longer description.
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "udpclient.h"
#include "udpserver.h"

int outport=33333;
int inport=33334;

void udpwrite(const char *s,...);

float a = 2;

float warming = 0.15;
float tdecay = 0.99;
float *heat;


void runsim(double t){
    udpwrite("a=%f heat=%f",a,*heat);
    a += warming; //temperature coming in
    a += *heat*0.04; // extra heat
    a *= tdecay; // temperature going out
    
    
}

    
double gettime(){
    timespec ts;
    clock_gettime(CLOCK_REALTIME,&ts);
    
    double t = ts.tv_nsec;
    t *= 1e-9;
    t += ts.tv_sec;
    return t;
}

void udpwrite(const char *s,...){
    va_list args;
    va_start(args,s);
    char buf[1024];
    sprintf(buf,"time=%f ",gettime());
    vsnprintf(buf+strlen(buf),1024-strlen(buf),s,args);
    printf("%s\n",buf);
    udpSend("127.0.0.1",outport,buf);
    va_end(args);
}


int main(int argc,char *argv[]){
    
    UDPServer s(inport);
    
    heat=s.createVar("heat");
    
    printf("outport is %d, inport is %d\n",outport,inport);
    for(;;){
        usleep(300000);
        s.update();
        runsim(gettime());
    }
        
}
