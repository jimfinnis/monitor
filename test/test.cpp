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
#include <unistd.h>
#include <math.h>
#include <time.h>

#include "udpclient.h"


int main(int argc,char *argv[]){
    time_t t;
    char buf[256];
    int port =13231;
    
    time_t st;
    time(&st);
    
    if(argc>1)
        port =atoi(argv[1]);

    
    printf("Port is %d\n",port);
    for(;;){
        sleep(1);
        time(&t);
        
        float f = t-st;
        
        float lat = sinf(f)+52;
        float lng = cos(f)-4;
        
        float a = sinf(f*0.1f);
        float b = cosf(f*2.3f);
        
        sprintf(buf,"time=%d lat=%f lon=%f a=%f b=%f c=%f",t,lat,lng,a,b,sinf(a));
        printf("%s\n",buf);
        udpSend("127.0.0.1",port,buf);
    }
        
}
