/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

//#define TEST

extern void setval(const char *var,double val);
extern void setwpcount(int n);
extern void setwp(int n,double lat,double lon);


#ifdef TEST
#define debug printf
void setval(const char *var,double val){
    printf("Set value: %s=%f\n",var,val);
}
void setwpcount(int n){
    printf("Setting waypoint count to %d\n",n);
}
void setwp(int n,double lat,double lon){
    printf("Setting waypoint %d to lat %f,lon %f\n",n,lat,lon);
}

//#include "udp.h"
    
UDP udp;

inline void readline(char *buf){
    debug("Waiting for line.\n");
    const char *data = udp.poll(true);
    if(data){
        strcpy(buf,data);
        strcat(buf,"\n");
    } else
        buf[0]=0;
}
#else
#define debug 
#endif

static int recvtid = -1;

uint16_t Fletcher16( const uint8_t* data, int count )
{
    uint16_t sum1 = 0;
    uint16_t sum2 = 0;
    int index;
    
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

inline void write(const char *s){
    char buf[128];
    // first, we write the TID past where the checksum will go
    sprintf(buf+5,"+%03x+",recvtid);
    // then we write the message past that
    strcpy(buf+10,s);
    // then we calculate the checksum and put that at the beginning
    int sum = Fletcher16((uint8_t *)buf+5,strlen(buf+5));
    sprintf(buf,"+%04x",sum);
    buf[5]='+';
    
#ifdef TEST
    printf("SENDING %s\n",buf);
    udp.send(buf);
#else
    Serial.println(buf);
#endif
}


uint16_t hex2int(const char *a, unsigned int len)
{
    int i;
    uint16_t val = 0;
    
    for(i=0;i<len;i++)
        if(a[i] <= 57)
            val += (a[i]-48)*(1<<(4*(len-1-i)));
        else
            val += (a[i]-87)*(1<<(4*(len-1-i)));
    return val;
}

char *processSerialString(char *s,bool storeTID){
    if(*s=='+'){
        debug(">> %s\n",s);
        
        s++;
        int sum = hex2int(s,4);
        s+=4;
        int strSum=Fletcher16((uint8_t *)s,strlen(s));
        if(sum==strSum){
            s++;
            int ttt = hex2int(s,3);
            if(storeTID)
                recvtid = ttt;
            else if(recvtid!=ttt){
                debug("FAILED - TID mismatch, mine=%x, theirs=%x\n",(int)recvtid,(int)ttt);
                return NULL;
            }
            s+=4;
            return s;
        } else
            debug("FAILED - bad checksum\n");
            return NULL;
    } else {
        // this is a variable set
        char *keyvalpair = strtok(s," ");
        while(keyvalpair){
            char *brk = strchr(keyvalpair,'=');
            if(brk){
                char key[32];
                memcpy(key,keyvalpair,brk-keyvalpair);
                key[brk-keyvalpair]=0;
                setval(key,atof(brk+1));
            }
            keyvalpair = strtok(NULL," ");
        }
        return NULL;
    }
}

#define INIT 0
#define WAYP 1

static char mode = INIT;
char bitfield[32];
int waypointcount;
int waypointsremaining;

static void processWaypointLine(char *p){
    p = processSerialString(p,false);
    if(p && *p=='w'){
        p++;
        char *t = strtok(p," ");
        if(!t)return;
        int wpn = atoi(t);
        // waypoint number is now wpn. Have we got it?
        if(!(bitfield[wpn/8] & (1<<(wpn%8)))){
            // now read the fields
            // assume an ordering for the fields
            if(!(t = strtok(NULL," ")))
                return;
            double lat = atof(t);
            if(!(t = strtok(NULL," ")))
                return;
            double lon = atof(t);
            // Add extra fields here
            // NOW SET WAYPOINT WPN to LAT,LON
            setwp(wpn,lat,lon);
            // set the bit and decrement remaining points
            bitfield[wpn/8] |= 1<<(wpn%8);
            waypointsremaining--;
        }
    } else {
        debug("dammit\n");
        mode = INIT;
    }
}

static void processLine(char *s){
    debug("READ [mode %d]: %s\n",mode,s);
    switch(mode){
    default:
    case INIT:
        s = processSerialString(s,true);
        if(s && *s =='t'){
            // it's a waypoint message. At this stage,
            // ignore anything that isn't "+t..."
            s++;
            char *colon = strchr(s,':');
            if(colon){
                *colon = 0;
                waypointcount = atoi(s);
                setwpcount(waypointcount);
                waypointsremaining = waypointcount;
                mode = WAYP;
                memset(bitfield,0,32);
                write("at\n");
            }
        }
        break;
    case WAYP:
        processWaypointLine(s);
        if(waypointsremaining==0){
            write("aw1\n");
            mode = INIT;
        }
        break;
    }
}

#define MAXLINE 128
void handleSerialChar(char c){
    char buf[MAXLINE];
    static int cidx=0;
    
    if(c=='\n' || c==0){
        buf[cidx]=0;
        processLine(buf);
        cidx=0;
    } else if(cidx<MAXLINE)
        buf[cidx++]=c;
}





    
#ifdef TEST
int main(int argc,char *argv[]){
    
    int err = udp.start(8888,"127.0.0.1",8889);
    if(err){
        perror("UDP startup");
        exit(1);
    }
    udp.send("time=0");
    
    
    for(;;){
        char buf[256];
        readline(buf);
        
        for(char *p=buf;*p;p++)
            handleSerialChar(*p);
    }
}
#endif
