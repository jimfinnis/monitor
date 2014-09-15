/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#ifndef __UDP_H
#define __UDP_H

class UDP {
private:
    int fd;
    const char *outputhost;
    int outputport;
public:
    UDP();
    int start(int inport,const char *outhost,int outport);
    void stop();
    const char *poll(bool block=false);
    
    bool send(const char *msg);
};


#endif /* __UDP_H */
