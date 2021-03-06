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
#include <string.h>
#include <time.h>

#include "udpclient.h"
#include "udpserver.h"

int servPort = 33333;

class RateListener : public UDPServerListener {
public:
    float rate;
    RateListener(){
        rate = 0;
    }
    virtual void onKeyValuePair(const char *s,float v){
        if(!strcmp("rate",s)){
            rate = v;
            printf("Rate now %f\n",rate);
        }
    }
    
    void update(){
        rate *= 0.98f;
    }
};

UDPServer server;
RateListener rateListener;

int main(int argc,char *argv[]){
    time_t t;
    char buf[256];
    int port =13231;
    
    
    int i = 0;
    
    time_t st;
    time(&st);
    
    // apply a random offset to time for testing
    st+=1001948L;
    
    if(argc>1)
        port =atoi(argv[1]);

    
    printf("Port is %d, UDP server port is %d\n",port,servPort);
    
    server.setListener(&rateListener);
    int e = server.start(servPort);
    if(e)
        printf("error: %d\n",e);
    
    for(;;){
        sleep(1);
        time(&t);
        rateListener.update();
        server.poll();
        
        float f = t-st;
        
        float lat = sinf(f)+52;
        float lng = cos(f)-4;
        
        float lat2 = sinf(f*0.5f)+52;
        float lng2 = cos(f*0.1f)-4;
        
        float a = sinf(f*0.1f);
        float b = cosf(f*2.3f*(rateListener.rate+1));
        
        sprintf(buf,"time=%d lat=%f lon=%f a=%f b=%f c=%f i=%d",t,lat,lng,a,b,sinf(a),i++);
        printf("%s\n",buf);
        udpSend("127.0.0.1",port,buf);
        
        if(!(i%10)){
            sprintf(buf,"time=%d lat2=%f lon2=%f rate=%f",t,lat2,lng2,rateListener.rate);
            printf("%s\n",buf);
            i++;
        }
        udpSend("127.0.0.1",port,buf);
    
    }
        
}
