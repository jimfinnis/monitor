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
#include "waypoint.h"
#include "udp.h"

#define SENDPORT 33333
#define RECVPORT 13231

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
    
    // add some waypoints
    double *d;
    d = wpGet(wpAppend());
    d[0] = 0;
    d[1] = 1;
    d[2] = 2;
    d = wpGet(wpAppend());
    d[0] = 0;
    d[1] = 1;
    d[2] = 2;
    d = wpGet(wpAppend());
    d[0] = 0;
    d[1] = 1;
    d[2] = 2;
    d = wpGet(wpAppend());
    d[0] = 0;
    d[1] = 1;
    d[2] = 2;
    d = wpGet(wpAppend());
    d[0] = 0;
    d[1] = 1;
    d[2] = 2;
    
    
    // and transmit
    //    wpSendWaypoints();
    wpRequestWaypoints(1);
    
    for(;;){
        const char *s = udpPoll(10);
        if(s && *s=='+'){
            printf("RECEIVED: [%s]\n",s);
            wpProcessString(s);
        }
        int stat = wpStatus();
//        if(stat)printf("Status: %d\n",stat);
        wpTick();
    }
            
}
