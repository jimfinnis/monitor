/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#ifndef __UDPSERVER_H
#define __UDPSERVER_H

class UDPServerListener {
public:
    virtual void onMessage(const char *s);
};

class UDPServer {
private:
    int fd;
    UDPServerListener *listener;
public:
    UDPServer();
    int start(int port);
    void stop();
    
    void setListener(UDPServerListener *l){
        listener = l;
    }
    
    void poll();
};

#endif /* __UDPSERVER_H */