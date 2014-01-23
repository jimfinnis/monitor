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
    virtual void processUDP(char *buf,int size)=0;
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


/// this is implemented by objects which are interested in when a certain value actually gets sent

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
    bool hasAddress;
    int port;
    
    static UDPClient *instance;
    UDPClient(){
        hasAddress =false;
    }
    
public:
    static UDPClient *getInstance(){
        if(!instance)
            instance = new UDPClient();
        return instance;
    }
    
    /// this will only have an effect if no address
    /// has been set yet.
    void setAddress(QHostAddress& h){
        if(!hasAddress){
            addr = h;
            hasAddress=true;
        }
    }
    
    void setPort(int p){
        port = p;
    }
    
    /// by default the client will send to the same address
    /// the first telemetry packet comes from; setting this
    /// before that happens will override this.
    void setAddress(const char *s){
        addr = QHostAddress(s);
        hasAddress=true;
    }
    
    void add(OutValue *o){
        values.append(o);
    }
    
    /// this will send a UDP packet to the host containing
    /// those values which (a) need updating (b) are set to
    /// always update.
    void update();
    
    /// send an arbitrary string, typically a waypoint packet
    void send(const char *s);
};

/// enum used by UDP sending widgets - not all of them use all of these
enum UDPState {
    INIT, //!< widget unchanged and never pressed
    UNSENT, //!< widget changed but data not sent yet
          UNACK, //!< data sent but no acknowledgement received
          BADACK, //!< data sent, ack received, but it's not right
          WAITING, //!< a wait state of some kind
          OK //!< all is OK.
};

extern "C" {
/// this is a useful function for C-linkage UDP send
void udpSendFunc(const char *s);
}





#endif /* __UDP_H */
