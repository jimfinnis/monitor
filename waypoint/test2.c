/**
 * @file 
 * Test code
 * 
 * @author $Author$
 * @date $Date$
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>
#include "waypoint.h"
#include "udp.h"

#define SENDPORT 13231
#define RECVPORT 33333

void sighandler(int t){
    udpClose();
    exit(1);
}

static void send(const char *s){
    printf("%4x SENDING: %s\n",wpStatus(),s);
    udpSend(SENDPORT,s);
}

int main(int argc,char *argv[]){
    signal(SIGQUIT,sighandler);
    signal(SIGINT,sighandler);
    
    udpInitRecv(RECVPORT);
    
    wpInit(send,NULL);
    
    wpResetDefinitions();
    
    wpAddField("linger",0);
    wpAddField("dist",0.009);
    wpAddField("target_speed",3);
    
    wpCreateWorking(100);
    
    double *d;
    
    d = wpGet(wpAppend());
    d[0] = 1;
    d[1] = 2;
    d[2] = 3;
    d = wpGet(wpAppend());
    d[0] = 4;
    d[1] = 5;
    d[2] = 6;
    d = wpGet(wpAppend());
    d[0] = 7;
    d[1] = 8;
    d[2] = 9;
    d = wpGet(wpAppend());
    d[0] = 10;
    d[1] = 11;
    d[2] = 12;
    d = wpGet(wpAppend());
    d[0] = 13;
    d[1] = 14;
    d[2] = 15;
    
    for(;;){
        const char *s = udpPoll(10);
        if(s && *s=='+'){
            printf("RECEIVED: [%s]\n",s);
            wpProcessString(s);
        }
        int stat = wpStatus();
//        if(stat)printf("Status: %d\n",stat);
        wpTick();
        
        static int foo=100;
        if(!--foo){
            send("a=0");foo=100;
        }
 
    }
            
}
