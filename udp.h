/**
 * \file
 * UDP server.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __UDP_H
#define __UDP_H

#include <QUdpSocket>

class UDPListener {
public:
    virtual void process(const char *buf,int size)=0;
};

class UDPServer : public QObject{
    Q_OBJECT
          
    QUdpSocket *sock;
    UDPListener *listener;
    
    char buf[2048]; // 2K should be enough for a line
    
public slots:
    void readPendingDatagrams();
public:
    
    void setListener(UDPListener *l){
        listener=l;
    }
    
    UDPServer(int port);
    ~UDPServer();
};



/*
 * 
 * Clientside stuff
 *
 */


class UDPClientSendListener {
public:
    virtual void onSend()=0;
};

/// an OutValue is a float which gets sent to another UDP server
/// (on the robot) from time to time.

struct OutValue {
    const char *name;
    double timeSent; //!< time last sent to server
    double timeChanged; //!< time last changed by client (i.e. me)
    float val; //!< value
    bool always; //!< if true, will be sent in every update packet.
    /// this is called whenever this value is sent
    UDPClientSendListener *listener;
    
    OutValue(const char *n,float init=0,bool alw=true);
    
    /// use this to set the value
    void set(float v);
        
};


/// this is a singleton class which will send UDP messages for switch settings,
/// etc, every now and then.

class UDPClient {
    QList<OutValue *> values;
    QUdpSocket sock;
    QHostAddress addr;
    int port;
    
    static UDPClient *instance;
    UDPClient(){}
    
public:
    static UDPClient *getInstance(){
        if(!instance)
            instance = new UDPClient();
        return instance;
    }
    
    void setAddress(const char *add, int p){
        port = p;
        addr = QHostAddress(add);
    }
        
    
    void add(OutValue *o){
        values.append(o);
    }
    
    /// this will send a UDP packet to the host containing
    /// those values which (a) need updating (b) are set to
    /// always update.
    void update();
    
};



#endif /* __UDP_H */
