/**
 * \file
 * Brief description. Longer description.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __UDPCLIENT_H
#define __UDPCLIENT_H

/// a useful function to write a UDP packet prefixed by a timestamp
/// and formatted with printf semantics.

class UDPClient {
    int serverPort;
    const char *serverAddr;
    bool broadcast;
    
    bool send(const char *msg);
    
public:
    UDPClient(const char *addr, int port, bool isBroadcast);
    void write(const char *s,...);
};


#endif /* __UDPCLIENT_H */
