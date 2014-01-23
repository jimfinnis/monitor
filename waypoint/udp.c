/**
 * @file 
 * Some very simple UDP code to do some tests with - has
 * the ability to discard packets.
 * 
 * @author $Author$
 * @date $Date$
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>


// how likely a packet is to get sent, out of 100
#define RELIABILITY 30


/// fatal error
static void die(const char *s,...){
    va_list args;
    va_start(args,s);
    char buf[400];
    vsnprintf(buf,400,s,args);
    printf("Fatal error: %s\n",buf);
    va_end(args);
    exit(1);
}

void udpSend(int port,const char *s){
    int sd;
    struct sockaddr_in groupSock;
    
    if((rand()%100)>RELIABILITY)
        return;
    
    /* Create a datagram socket on which to send. */
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0)
    {
        perror("Opening datagram socket error");
        die("send failed");
    }


    memset((char *) &groupSock, 0, sizeof(groupSock));
    
    
    groupSock.sin_family = AF_INET;
    groupSock.sin_addr.s_addr = inet_addr("127.0.0.1");
    groupSock.sin_port = htons(port);


    if (sendto(sd, s, strlen(s)+1, 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0)
    {
        perror("Sending datagram message error");
        die("send failed");
        return;
    }

    close(sd);
}


int insd=-1;

void udpInitRecv(int port){
    struct sockaddr_in me;
    insd = socket(AF_INET,SOCK_DGRAM,0);
    if(insd<0){
        perror("failed to open receive socket");
        die("initrecv failed");
    }
    
    memset((char *)&me,0,sizeof(me));
    me.sin_family = AF_INET;
    me.sin_addr.s_addr = inet_addr("127.0.0.1");
    me.sin_port = htons(port);
    if(bind(insd,(struct sockaddr*)&me,sizeof(me))<0){
        perror("bind");
        die("initrecv failed");
    }
}

void udpClose(){
    if(insd>=0)close(insd);
    insd=-1;
}

const char *udpPoll(long waitms){
    struct timespec ts;
    fd_set s;
    int rv;
    
    if(insd<0)die("no open socket");
    
    ts.tv_sec = 0;
    ts.tv_nsec = waitms * 1000000L;

    FD_ZERO(&s);
    FD_SET(insd,&s);
    
    rv = pselect(insd+1,&s,NULL,NULL,&ts,NULL);
    if(rv<0)
        perror("pselect");
    else if(rv>0){
        static char buf[512];
        int d = read(insd,buf,512);
        return buf;
    } else
        return NULL;
}
