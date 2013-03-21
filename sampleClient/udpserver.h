/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#ifndef __UDPSERVER_H
#define __UDPSERVER_H

#include <map>
#include <string>

/*
 * 
 * Simple UDP server, used when we want to listen for data changes
 *
 */

class UDPServer {
    
    /// map of string to float
    std::map<std::string,float *> map;
    /// the socket
    int fd;
    
    /// parse a line
    void parseLine(const char *s);
    
    /// receive buffer
    char buf[8192];
    
public:
    UDPServer(int port);
    
    /// create a variable and return a pointer to it
    float *createVar(const char *name);
    
    /// return a pointer to a pre-existing variable
    float *findVar(const char *name);
    
    /// check if there are messages
    void update();
};

#endif /* __UDPSERVER_H */
