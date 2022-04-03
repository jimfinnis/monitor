/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "udpserver.h"

static void die(const char *s){
    perror(s);
    exit(1);
}

UDPServer::UDPServer(int port){
    sockaddr_in addr;
    
    fd = socket(AF_INET,SOCK_DGRAM,0);
    if(fd<0)
        die("cannot open socket");

        
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    
    if(bind(fd,(sockaddr *)&addr,sizeof(addr))<0)
        die("cannot bind socket");
    
    // larger buffer, reuseaddr
    int v=8192;
    setsockopt(fd,SOL_SOCKET,SO_RCVBUF,(char *)&v,sizeof(v));
    v=1;
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&v,sizeof(v));
    
    // set to nonblocking
    v = fcntl(fd,F_GETFL);
    v |= O_NONBLOCK;
    fcntl(fd,F_SETFL,v);
    
}


// parse each individual line in a packet
void UDPServer::parseLine(const char *s){
    const char *varstart;
    const char *valstart;
    int n;
    enum { WAITVAR,INVAR,WAITEQ,WAITVAL,INVAL } mode;
    
    
    //    if(0==(rand()%5))return; // for debugging, skips random packets
    
    
    mode = WAITVAR;
    
    char varbuf[256];
    char valbuf[256];
    do {
        switch(mode){
        case WAITVAR:
            if(isalnum(*s)){
                varstart=s;
                n=1;
                mode=INVAR;
            }
            break;
        case INVAR:
            if(!isalnum(*s)){
                strncpy(varbuf,varstart,n);
                varbuf[n]=0;
                if(*s=='=')
                    mode=WAITVAL;
                else
                    mode=WAITEQ;
            }
            else n++;
            break;
        case WAITEQ:
            if(*s == '=')
                mode = WAITVAL;
            break;
        case WAITVAL:
            if(*s && !isspace(*s)){
                valstart=s;
                n=1;
                mode=INVAL;
            }
            break;
        case INVAL:
            if(!*s || *s==',' || isspace(*s)){
                strncpy(valbuf,valstart,n);
                valbuf[n]=0;
                mode=WAITVAR;
                
                  printf("Got [%s = %f]\n",varbuf,atof(valbuf));
                if(strcmp(varbuf,"time")){
                    float *p = findVar(varbuf);
                    if(p) *p=atof(valbuf);
                    else printf("CANNOT FIND VARIABLE %s\n",varbuf);
                }
            }
            else n++;
            break;
        }
    }while(*s++);
    
}

void UDPServer::update(){
    int size = recvfrom(fd,buf,8192,0,NULL,NULL);
    if(size<=0){
        // either an error or socket disconnected
        return;
    }
    printf("Size of packet: %d\n",size);
    buf[size]=0;
    printf("Packet: %s\n",buf);
    char *s = buf;
    
    int done=0;
    while(done<size){
        int len = strlen(s);
        parseLine(s);
        s+=len+1;
        done+=len+1;
    }
}

    
float *UDPServer::createVar(const char *name){
    float *v= new float();
    *v = 0;
    printf("CREATE %p\n",v);
    map[std::string(name)] = v;
    return v;
}

float *UDPServer::findVar(const char *name){
    std::string k(name);
    if(map.find(k)==map.end())
        return NULL;
    else{
        printf("FIND %p\n",map[k]);
        return map[k];
    }
}
