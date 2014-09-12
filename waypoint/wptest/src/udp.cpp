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
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "udp.h"


UDP::UDP(){
    fd = -1;
}

bool UDP::send(const char *msg){
    sockaddr_in servaddr;
    int fd = socket(AF_INET,SOCK_DGRAM,0);
    if(fd<0){
        perror("cannot open socket");
        return false;
    }
    
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(outputhost);
    servaddr.sin_port = htons(outputport);
    if (sendto(fd, msg, strlen(msg)+1, 0, // +1 to include terminator
               (sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        perror("cannot send message");
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

int UDP::start(int inport,const char *outhost,int outport){
    sockaddr_in addr;
    fd = socket(AF_INET,SOCK_DGRAM,0);
    if(fd<0)
        return errno;
    
    outputport = outport;
    outputhost = outhost;
    
    bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(inport);
    if(bind(fd,(sockaddr *)&addr,sizeof(addr))){
        close(fd);
        fd=-1;
        return errno;
    }
    return 0;
}

void UDP::stop(){
    if(fd>=0)
        close(fd);
}


const char *UDP::poll(bool block){
    static char buf[1024];
    sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    int n = recvfrom(fd,buf,1024,
                     block?0:MSG_DONTWAIT,
                     (struct sockaddr *)&cliaddr,
                     &len); 
    if(n>0){
        return buf;
    } else return NULL;
}

