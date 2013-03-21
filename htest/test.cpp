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

int port=33333;
void udpwrite(const char *s,...);

float a = 2;

float warming = 0.19;
float tdecay = 0.99;
float heat =0;

void runsim(double t){
    udpwrite("a=%f",a);
    
    
    a += warming; //temperature coming in
    a += heat*0.01; // extra heat
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
    udpSend("127.0.0.1",port,buf);
    va_end(args);
}


int main(int argc,char *argv[]){
    time_t t;
    char buf[256];
    
    time_t st;
    time(&st);
    
    heat = atof(argv[1]);
    
    printf("Port is %d\n",port);
    for(;;){
        usleep(30000);
        runsim(t);
    }
        
}
