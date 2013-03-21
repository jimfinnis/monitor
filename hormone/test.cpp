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


float temp[6]={2,2,2,2,2,2};
// constants

const float warming[6] = {0.12,0.04,0.14,0.2,0.16,0.149};
const float tdecay = 0.9992;

float limp[6]={0,0,0,0,0,0};

#define MINTHRESH  12
#define MAXTHRESH  18

void runsim(){
    
    float avglimp = 0;
    for(int i=0;i<6;i++){
        // calculate limp factors
        if(temp[i]<MINTHRESH)limp[i]=0;
        else if(temp[i]>MAXTHRESH)limp[i]=1;
        else{
            limp[i] = (temp[i]-MINTHRESH)/(MAXTHRESH-MINTHRESH);
            avglimp += limp[i];
        }
    }
    avglimp /= 6.0f;
    
    for(int i=0;i<6;i++){
        
        // calculate load. Complicated. The load is a function of distance
        // between how much I'm limping and how much all the other wheels
        // are limping.
        
        float dist = avglimp-limp[i];
        if(dist<0)dist=0;
        float load = 1-dist;
        
        
        temp[i] += warming[i]*0.01; //temperature coming in
        temp[i] += load*0.02; // extra heat
        temp[i] *= tdecay; // temperature going out
        
        
        
        udpwrite("temp%d=%f limp%d=%f load%d=%f",i,temp[i],i,limp[i],i,load);
    }
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
    
//    heat=s.createVar("heat");
    
    printf("outport is %d, inport is %d\n",outport,inport);
    
    while(temp[0]<12)runsim();
    
    for(;;){
        usleep(100000);
        s.update();
        runsim();
    }
        
}
